#pragma once

#include <vector>

#include "../ImportHandler.hpp"
#include "../ExportHandler.hpp"
#include "../../core/Memtypes.hpp"
#include "../../include/buzzemu/Results.hpp"

//the value linker should change in runtime is `import_desc->FirstThunk`

namespace bzmu::pe {
	class linker {
	public:
		linker() {}
		linker(const std::vector<u8>& data) : pe_content(data) {}
		//TODO
		void link_runtime(std::vector<u8>& binary, PIMAGE_DOS_HEADER dos_hdr, export_container& _exports);
	private:
		std::vector<u8> pe_content;
	};
}