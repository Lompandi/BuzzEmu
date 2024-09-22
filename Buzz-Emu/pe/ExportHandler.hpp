#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <vector>
#include <unordered_map>

#include "../memory/VirtualAddr.hpp"

namespace bzmu::pe {
	struct export_function {
		size_t		ordinal_num;
		VirtualAddr address;
		std::string func_name;
	};

	struct export_container {
	public:
		void set_export_container(PIMAGE_DOS_HEADER dos_hdr);
		
		[[nodiscard]] std::vector<export_function> get_export_table() const { return _exported; }
		[[nodiscard]] VirtualAddr get_function_address(std::string_view func_name);
	private:
		void sync_hash_map() {
			for (const auto& it : _exported)
				_function_to_addr[it.func_name] = it.address;
		};

		std::unordered_map<std::string_view, VirtualAddr> _function_to_addr;
		std::vector<export_function> _exported;
	};
}