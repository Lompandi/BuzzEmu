
#include "PeUtils.hpp"

namespace bzmu::pe {
	PIMAGE_NT_HEADERS get_nt_hdrs(std::vector<u8>& binary) {
		PIMAGE_DOS_HEADER dos_hdr =
			reinterpret_cast<PIMAGE_DOS_HEADER>(binary.data());

		return reinterpret_cast<PIMAGE_NT_HEADERS>(
			reinterpret_cast<u8*>(dos_hdr) + dos_hdr->e_lfanew);
	}

	PIMAGE_NT_HEADERS get_nt_hdrs(PIMAGE_DOS_HEADER dos_hdr) {
		return reinterpret_cast<PIMAGE_NT_HEADERS>(
			reinterpret_cast<u8*>(dos_hdr) + dos_hdr->e_lfanew);
	}

	PIMAGE_DOS_HEADER get_dos_hdr(std::vector<u8>& binary) {
		return reinterpret_cast<PIMAGE_DOS_HEADER>(binary.data());
	}
}