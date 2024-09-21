#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>

namespace bzmu::pe {
	struct import_function {
		std::wstring dll_name;
		std::string func_name;
	};

	struct import_container {
		void fetch_import_table(PIMAGE_DOS_HEADER dos_hdr);
	
		std::vector<import_function> _imported;
	};
}