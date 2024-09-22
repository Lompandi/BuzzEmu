#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Results.hpp"
#include "../../core/Memtypes.hpp"
#include "../../memory/VirtualAddr.hpp"

namespace bzmu::pe {
	enum class address_error {
		foa_not_found,
	};
	[[nodiscard]] result<u64, address_error> rva_to_foa(PIMAGE_NT_HEADERS nt_hdr, VirtualAddr rva);
}