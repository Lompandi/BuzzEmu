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

	/*Section(size_t file_off, VirtualAddr virtual_addr, size_t file_size, size_t mem_size, Permission permission) :
		file_off(file_off), virt_addr(virtual_addr), file_size(file_size), mem_size(mem_size), permission(permission) {}*/
};