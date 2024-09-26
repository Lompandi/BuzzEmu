
#include "Linker.hpp"
#include "../../core/Fs.hpp"
#include "../../emulator/Emulator.hpp"
#include "../../include/buzzemu/Strings.hpp"
#include "../../include/buzzemu/Address.hpp"

namespace bzmu::pe {
	void linker::link_runtime(
		std::vector<u8>& binary,
		PIMAGE_DOS_HEADER dos_hdr, 
		export_container& _exports) {

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

				std::wstring dll_name;
				std::wstring func_name;

				dll_name = bytes_to_wstring(reinterpret_cast<LPSTR>(reinterpret_cast<u8*>(dos_hdr) +
					rva_to_foa(nt_header, import_desc->Name).value()));

				thunk_data64 = reinterpret_cast<PIMAGE_THUNK_DATA64>(
					reinterpret_cast<u8*>(dos_hdr) +
					rva_to_foa(nt_header, import_desc->FirstThunk).value());

				size_t thunk_offset = 0;
				while (thunk_data64->u1.AddressOfData != 0) {
					import_by_name = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(
						reinterpret_cast<u8*>(dos_hdr) +
						rva_to_foa(nt_header, thunk_data64->u1.AddressOfData).value());
					//we will currently handle the imports via names, 
					//TODO: add ordinal number imports handle code 
					
					std::wstring func_name = bytes_to_wstring(import_by_name->Name);
					auto imp_func_runtime_addr = _exports.get_function_address(
						wstring_to_string(func_name));
					if (imp_func_runtime_addr) {
						auto ptr_to_entry = rva_to_foa(nt_header, import_desc->FirstThunk
							+ thunk_offset).value();
						*reinterpret_cast<uint64_t*>(binary.data() + ptr_to_entry)
							= imp_func_runtime_addr;
						std::cout << "resolving rva: 0x" << std::hex << import_desc->FirstThunk
							+ thunk_offset << " to -> 0x" << imp_func_runtime_addr << '\n';
					}
					//switch to next thunk
					thunk_offset += 8;
					thunk_data64++;
				}
				import_desc++;
			}
		}
		return;
	}
}