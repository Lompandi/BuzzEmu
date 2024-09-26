
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

#include "../core/Memtypes.hpp"

namespace bzmu::pe {
	PIMAGE_NT_HEADERS get_nt_hdrs(std::vector<u8>& binary);
	PIMAGE_NT_HEADERS get_nt_hdrs(PIMAGE_DOS_HEADER dos_hdr);

	PIMAGE_DOS_HEADER get_dos_hdr(std::vector<u8>& binary);
}