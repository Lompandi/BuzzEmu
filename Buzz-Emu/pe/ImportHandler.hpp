#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>
#include <unordered_map>

#include "../include/buzzemu/Results.hpp"
#include "../include/buzzemu/Defines.hpp"

namespace bzmu::pe {
	struct import_function {
		std::wstring dll_name;
		std::string func_name;
	};

	enum class search_error {
		not_found = 0,
	};

	struct import_container {
		void fetch_import_table(PIMAGE_DOS_HEADER dos_hdr);
		
		[[nodiscard]] result<std::wstring, search_error> get_dll_by_function(std::string_view func_name);
		[[nodiscard]] std::vector<std::string> get_functions_by_dll(std::wstring_view dll_name);
		[[nodiscard]] std::vector<import_function> get_imported_functions() {
			return _imported;
		}
	private:
		void sync_hash_table();

		std::unordered_map<std::string_view, std::wstring> _function_to_dll;
		std::vector<import_function> _imported;
	};
}