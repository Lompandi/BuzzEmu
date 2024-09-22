
#define NOMINMAX

#include <algorithm>

#include "Mapper.hpp"
#include "../core/Fs.hpp"
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

	map_result pe_mapper::map_into_mem(Emulator& emu, std::vector<u8>& binary) {
		PIMAGE_DOS_HEADER dos_hdr =
			reinterpret_cast<PIMAGE_DOS_HEADER>(binary.data());
		
		// Read section to map from the main pe file
		PIMAGE_NT_HEADERS image_nt_hdrs = reinterpret_cast<PIMAGE_NT_HEADERS>(
			reinterpret_cast<u8*>(dos_hdr) + dos_hdr->e_lfanew);
		if (image_nt_hdrs->Signature != IMAGE_NT_SIGNATURE)
			return map_result::map_pe_not_valid;

		std::vector<Section> exe_section_hdrs(image_nt_hdrs->FileHeader.NumberOfSections);
		PIMAGE_SECTION_HEADER section_hdr = IMAGE_FIRST_SECTION(image_nt_hdrs);
		for (size_t i = 0; i < image_nt_hdrs->FileHeader.NumberOfSections; ++i) {
			auto section = section_hdr[i];
			exe_section_hdrs.push_back(
				Section(
					0, /*TMP ADDRESS SINCE CODE DONT NEED IT YET*/
					section.VirtualAddress,
					section.SizeOfRawData,
					section.SizeOfRawData,
					get_section_permission(section.Characteristics)
				));
		}

		for (const auto& section : exe_section_hdrs) {
			//Set memory to writable
			emu.memory.SetPermission(section.virt_addr, section.mem_size, PERM_WRITE);

			//Write in th eoriginal file contents 
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

		import_container _imported;
		export_container _exported;

		_imported.set_import_table(dos_hdr);
		std::vector<import_function> data = _imported.get_imported_functions();
		for (const auto& func : data) {

		}
	}
}