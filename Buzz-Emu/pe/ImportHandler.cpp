
#include "../core/Fs.hpp"
#include "ImportHandler.hpp"
#include "../core/Memtypes.hpp"
#include "../include/buzzemu/Strings.hpp"
#include "../include/buzzemu/Address.hpp"

namespace bzmu::pe {
	void import_container::set_import_table(PIMAGE_DOS_HEADER dos_hdr) {

		auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(
			reinterpret_cast<u8*>(dos_hdr) + dos_hdr->e_lfanew
		);

		if (nt_header->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
			if (reinterpret_cast<PIMAGE_NT_HEADERS64>(nt_header)->OptionalHeader.
				DataDirectory[1].Size == 0)
				return;

			PIMAGE_IMPORT_DESCRIPTOR import_desc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
				reinterpret_cast<u8*>(dos_hdr) +
				rva_to_foa(nt_header, reinterpret_cast<PIMAGE_NT_HEADERS64>(nt_header)->OptionalHeader.DataDirectory[1].VirtualAddress).value());
			
			PIMAGE_THUNK_DATA64 thunk_data64;
			PIMAGE_IMPORT_BY_NAME import_by_name;


			while (import_desc->OriginalFirstThunk ||
				import_desc->TimeDateStamp || import_desc->ForwarderChain ||
				import_desc->Name || import_desc->FirstThunk) {

				std::wstring dll_name(128, L' ');
				std::wstring func_name(128, L' ');

				dll_name = 

				MultiByteToWideChar(CP_UTF8, 0,
					reinterpret_cast<LPSTR>(reinterpret_cast<u8*>(dos_hdr) +
						rva_to_foa(nt_header, import_desc->Name).value()), -1, dll_name.data(),
					dll_name.size());

				thunk_data64 = reinterpret_cast<PIMAGE_THUNK_DATA64>(
					reinterpret_cast<u8*>(dos_hdr) +
					rva_to_foa(nt_header, import_desc->FirstThunk).value());
				while (thunk_data64->u1.AddressOfData != 0) {
					import_by_name = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(
						reinterpret_cast<u8*>(dos_hdr) +
						rva_to_foa(nt_header, thunk_data64->u1.AddressOfData).value());

					import_function import_func = {
						.dll_name = dll_name,
						.func_name = "",
						.func_addr = 0 //TODO
					};
					if (thunk_data64->u1.AddressOfData & IMAGE_ORDINAL_FLAG64) {	
						import_func.func_name = "{ORD}: " +
							std::to_string(thunk_data64->u1.AddressOfData); /*Import via serial number*/
					}
					else {
						MultiByteToWideChar(CP_UTF8, 0, import_by_name->Name, -1,
							func_name.data(), func_name.size());
						import_func.func_name = wstring_to_string(func_name);
					}
					thunk_data64++;

					_imported.push_back(import_func);
				}
				import_desc++;
			}
		}
		sync_hash_table();
		return;
	}

	result<std::wstring, search_error> import_container::get_dll_by_function(std::string_view func_name) {
		auto it = _function_to_dll.find(func_name);
		if (it != _function_to_dll.end())
			return it->second;
		return result_error{ search_error::not_found };
	}

	std::vector<std::string> import_container::get_functions_by_dll(std::wstring_view dll_name) {
		std::vector<std::string> data;
		for (const auto& it : _imported) {
			if (it.dll_name == dll_name)
				data.push_back(it.func_name);
		}
		return data;
	}

	void import_container::sync_hash_table() {
		for (const auto& it : _imported) {
			_function_to_dll[it.func_name] = it.dll_name;
		}
	}
}