#pragma once

#include "../../core/Memtypes.hpp"

enum Flags: u8 {
	x86_Flag_F64 = (1 << 0),
	x86_Flag_D64 = (1 << 1),
};

enum DasmMode : u8 {
	X86_Dmode_16bit = 0,    /* real mode / virtual 8086 mode (16-bit) */
	X86_Dmode_32bit,        /* protected mode / long compatibility mode (32-bit) */
	X86_Dmode_64bit,        /* long mode (64-bit) */
};

enum OperandSize {
	X86_Osize_16bit = 0,
	X86_Osize_32bit,
	X86_Osize_64bit,
	X86_Osize_8bit,	//MANUAL modified code
};

enum AddressingSize : u8 {
	X86_Asize_16bit = 0,
	X86_Asize_32bit,
	X86_Asize_64bit,
};

enum DisplacementSize : u8 {
	X86_Disp_None = 0,      /* no displacement */
	X86_Disp_8,             /* byte displacement */
	X86_Disp_16,            /* word displacement */
	X86_Disp_32             /* dword displacement */
};