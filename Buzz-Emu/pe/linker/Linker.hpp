#pragma once

#include <vector>

#include "../ImportHandler.hpp"
#include "../ExportHandler.hpp"
#include "../../core/Memtypes.hpp"
#include "../../include/buzzemu/Results.hpp"

namespace bzmu::pe {
	class linker {
	public:
		linker(const std::vector<u8>& data) : pe_content(data) {}
		//TODO
		void link_runtime(const std::vector<u8>& data,
		const export_container& _exports);
	private:
		std::vector<u8> pe_content;
	};
}