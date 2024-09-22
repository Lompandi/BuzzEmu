
#include "ExportHandler.hpp"

#include "../core/Fs.hpp"
#include "../core/Memtypes.hpp"
#include "../include/buzzemu/Strings.hpp"
#include "../include/buzzemu/Address.hpp"

namespace bzmu::pe {
	void export_container::set_export_container(PIMAGE_DOS_HEADER dos_hdr) {
		auto nt_hdr = reinterpret_cast<PIMAGE_NT_HEADERS>(
			reinterpret_cast<u8*>(dos_hdr) + dos_hdr->e_lfanew
		);
		if (nt_hdr->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
			if (reinterpret_cast<PIMAGE_NT_HEADERS64>(nt_hdr)->OptionalHeader
				.DataDirectory[0].Size == 0) {
				return;
			}
			auto export_dir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
				reinterpret_cast<u8*>(dos_hdr) + rva_to_foa(nt_hdr,
					reinterpret_cast<PIMAGE_NT_HEADERS64>(nt_hdr)->OptionalHeader.DataDirectory[0].VirtualAddress).value());

			auto address_of_funcs = reinterpret_cast<u32*>(
				reinterpret_cast<u8*>(dos_hdr) +
				rva_to_foa(nt_hdr, export_dir->AddressOfFunctions).value()
			);
			auto address_of_name_ordinals = reinterpret_cast<u16*>(
				reinterpret_cast<u8*>(dos_hdr) +
				rva_to_foa(nt_hdr, export_dir->AddressOfNameOrdinals).value()
			);
			auto address_of_names = reinterpret_cast<u32*>(
				reinterpret_cast<u8*>(dos_hdr) +
				rva_to_foa(nt_hdr, export_dir->AddressOfNames).value()
			);

			std::wstring module_name;
			module_name = bytes_to_wstring(reinterpret_cast<char*>(
				reinterpret_cast<u8*>(dos_hdr) +
				rva_to_foa(nt_hdr, export_dir->Name).value()));

			for (u32 i = 0; i < export_dir->NumberOfFunctions; i++) {
				//create an element
				export_function export_func;

				u32 j;
				for (j = 0; j < export_dir->NumberOfNames; j++) {
					if (i == address_of_name_ordinals[j]) {
						std::wstring func_name =
							bytes_to_wstring(reinterpret_cast<char*>(
								reinterpret_cast<u8*>(dos_hdr) +
								rva_to_foa(nt_hdr, address_of_names[j]).value()));
						export_func.func_name = wstring_to_string(func_name);
						break;
					}
				}
				if (j == export_dir->NumberOfNames)
					export_func.func_name = "{ORD}"; //export via ordinal number

				if (address_of_funcs[i]) {
					export_func.ordinal_num = export_dir->Base + i;
					export_func.address = address_of_funcs[i];
				}

				_exported.push_back(export_func);
			}
		}
		sync_hash_map(); //sync elements to hash map
		return;
	}

	VirtualAddr export_container::get_function_address(std::string_view func_name) {
		auto it = _function_to_addr.find(func_name);
		if (it != _function_to_addr.end())
			return it->second;
		return 0;
	}
}