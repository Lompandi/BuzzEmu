
#include "Address.hpp"

namespace bzmu::pe {
	result<u64, address_error> rva_to_foa(PIMAGE_NT_HEADERS nt_hdr, VirtualAddr rva) {
		u64 target_foa = 0;
		
		PIMAGE_SECTION_HEADER section_hdr = reinterpret_cast<PIMAGE_SECTION_HEADER>(
			reinterpret_cast<u8*>(nt_hdr) + sizeof(IMAGE_NT_HEADERS64));

		for (auto i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++) {
			if ((rva >= section_hdr->VirtualAddress) &&
				(rva <= (section_hdr->VirtualAddress +
					section_hdr->SizeOfRawData))) {
				target_foa = rva - section_hdr->VirtualAddress;
				target_foa += section_hdr->PointerToRawData;
				return target_foa;
			}
			section_hdr++;
		}

		return result_error{ address_error::foa_not_found };
	}
}