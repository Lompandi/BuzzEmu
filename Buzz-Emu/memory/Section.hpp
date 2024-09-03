#pragma once

#include "Permission.hpp"

/*
Section information for a file
*/
struct Section {
	size_t			file_off;
	VirtualAddr		virt_addr;
	size_t			file_size;
	size_t			mem_size;
	Permission		permission;
};