
#define NOMINMAX

#include <cwctype>
#include <algorithm>
#include <iomanip>

#include "Mapper.hpp"

#include "PeUtils.hpp"
#include "../core/Fs.hpp"
#include "linker/Linker.hpp"
#include "ImportHandler.hpp"
#include "ExportHandler.hpp"
#include "../memory/Section.hpp"
#include "../include/buzzemu/Address.hpp"

namespace bzmu::pe {
	Permission get_section_permission(u32 characteristics) {
		u8 perm = 0;
		if (characteristics & IMAGE_SCN_MEM_READ) 
			perm |= PERM_READ;
		if (characteristics & IMAGE_SCN_MEM_WRITE)
			perm |= PERM_WRITE;
		if (characteristics & IMAGE_SCN_MEM_EXECUTE)
			perm |= PERM_EXEC;
		return perm;
	}

	std::vector<Section> get_pe_sections(PIMAGE_NT_HEADERS nt_hdr) {
		std::vector<Section> ret;
		PIMAGE_SECTION_HEADER section_hdr = IMAGE_FIRST_SECTION(nt_hdr);
		for (size_t i = 0; i < nt_hdr->FileHeader.NumberOfSections; ++i) {
			auto section = section_hdr[i];
			ret.push_back(
				Section(
					rva_to_foa(nt_hdr, section.VirtualAddress).value(), /*TMP ADDRESS SINCE CODE DONT NEED IT YET*/
					section.VirtualAddress,
					section.SizeOfRawData,
					section.SizeOfRawData,
					get_section_permission(section.Characteristics)
				));
		}
		return ret;
	}

	result<export_container, map_result> pe_mapper::map_into_mem(Emulator& emu, std::vector<u8>& binary) {
		PIMAGE_DOS_HEADER dos_hdr =
			reinterpret_cast<PIMAGE_DOS_HEADER>(binary.data());
		
		// Read section to map from the main pe file
		PIMAGE_NT_HEADERS image_nt_hdrs = reinterpret_cast<PIMAGE_NT_HEADERS>(
			reinterpret_cast<u8*>(dos_hdr) + dos_hdr->e_lfanew);
		if (image_nt_hdrs->Signature != IMAGE_NT_SIGNATURE)
			return result_error{ map_result::map_pe_not_valid };

		std::vector<Section> exe_section_hdrs =
			get_pe_sections(image_nt_hdrs);

		//we will then map the dlls first

		import_container _imported;
		export_container _exported;

		_imported.set_import_table(dos_hdr);
		std::vector<std::wstring> imported_dlls;
		std::vector<import_function> data = _imported.get_imported_functions();
		for (const auto& func : data) {
			if (std::find(imported_dlls.begin(), imported_dlls.end(), func.dll_name)
				== imported_dlls.end())
				imported_dlls.push_back(func.dll_name); /* prevent duplicated dll */
		}

		auto sys_dir_len = GetSystemDirectory(NULL, 0);
		if (!sys_dir_len)
			return result_error{ map_result::sysdir_not_found };

		std::vector<u8> dll_content;

		/* Map all imported dlls */
		for (const auto& dll_file : imported_dlls) {
			/* fetch the updated allocation pointer */
			VirtualAddr dll_base = emu.memory.cur_alloc; //dll mapping base TODO: make this an actual virtual mapping

			std::wstring sys_path(sys_dir_len - 1, L'\0'); 
			GetSystemDirectoryW(sys_path.data(), sys_dir_len); 

			sys_path += L"\\" + dll_file;

			/* Load import dll into the memory */
			dll_content.clear();
			dll_content = read_file(sys_path);
			if (dll_content.empty()) {
				std::wcout << L"dll: " << sys_path << L" - not found\n";
				continue; // Skip this dll, not found
			}
			auto dos_hdr	= get_dos_hdr(dll_content);
			auto dll_nt_hdr = get_nt_hdrs(dos_hdr);
			if (dll_nt_hdr->Signature != IMAGE_NT_SIGNATURE) [[unlikely]] {
				continue; // Skip invalid dlls
			}

			/* add dll's export function for the linker */
			auto dll_tbl_orig_size = _exported.get_export_table().size();
			_exported.set_export_container(dos_hdr);
			for(auto i = dll_tbl_orig_size; /* start from the new imported dll in current iteration */
			 i < _exported.get_export_table().size(); i++){
				/* add the dynamic address to the base address */
				_exported.get_export_table()[i].address += dll_base;
			}

			auto section_hdrs = get_pe_sections(dll_nt_hdr);

			std::wcout << L"0x" << std::hex << dll_base << " - ";
			for (const auto& section : section_hdrs) {
				VirtualAddr dll_virtaddr = section.virt_addr + dll_base;
				emu.memory.SetPermission(dll_virtaddr, section.mem_size, PERM_WRITE);

				//Write in the original file contents 
				emu.memory.WriteFrom(dll_virtaddr, get_range(dll_content, section.file_off, section.file_size));

				//Write in any paddings with zeros
				if (section.mem_size > section.file_size) [[likely]] {
					std::vector<u8> padding(section.mem_size - section.file_size);
					emu.memory.WriteFrom(dll_virtaddr + section.file_size, padding);
				}

				//Demote permissions to originals
				emu.memory.SetPermission(dll_virtaddr, section.mem_size, section.permission);

				//Update the allocator beyond any sections we load
				emu.memory.cur_alloc = std::max(
					emu.memory.cur_alloc,
					(dll_virtaddr + section.mem_size + 0xf) & ~0xf
				);
			}
			std::wcout << L"0x" << std::hex << emu.memory.cur_alloc << L" [" << dll_file << L"]\n";
		}

		/* Perform runtime function resolve */
		linker _imp_func_resolver;
		_imp_func_resolver.link_runtime(binary, dos_hdr, _exported);

		/* Map the exe into mem */
		std::cout << "0x" << std::hex << exe_section_hdrs[0].virt_addr << " - ";
		for (const auto& section : exe_section_hdrs) {
			//Set memory to writable
			emu.memory.SetPermission(section.virt_addr, section.mem_size, PERM_WRITE);

			//Write in th original file contents 
			emu.memory.WriteFrom(section.virt_addr, get_range(binary, section.file_off, section.file_size));

			//Write in any paddings with zeros
			if (section.mem_size > section.file_size) [[likely]] {
				std::vector<u8> padding(section.mem_size - section.file_size);
				emu.memory.WriteFrom(section.virt_addr + section.file_size, padding);
			}

			//Demote permissions to originals
			emu.memory.SetPermission(section.virt_addr, section.mem_size, section.permission);

			//Update the allocator beyond any sections we load
			emu.memory.cur_alloc = std::max(
				emu.memory.cur_alloc,
				(section.virt_addr + section.mem_size + 0xf) & ~0xf
			);
		}

		std::cout << "0x" << std::hex << emu.memory.cur_alloc << " [Program]\n";
		return _exported;
	}
}