#pragma once

#include <vector>

#include "../../core/Memtypes.hpp"

namespace bzmu::pe {
	class linker {
	public:
		linker(const std::vector<u8>& data) : pe_content(data) {}
		//TODO
	private:
		std::vector<u8> pe_content;
	};
}