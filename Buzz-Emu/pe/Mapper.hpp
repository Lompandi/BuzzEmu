#pragma once

#include <vector>

#include "../core/Memtypes.hpp"
#include "../emulator/Emulator.hpp"
#include "../include/buzzemu/Results.hpp"

namespace bzmu::pe {
	enum class map_result {
		success = 0,
		map_pe_not_valid,
		sysdir_not_found,
		mem_no_space,
	};

	class pe_mapper {
		map_result map_into_mem(Emulator& emu, std::vector<u8>& binary);
	};
}