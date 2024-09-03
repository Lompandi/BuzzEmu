#pragma once

//64-bit x86 registers

#include "../../core/Memtypes.hpp"

enum Register : u32 {
	Rax = 0,
	Rcx,
	Rdx,
	Rbx,
	Rsp,
	Rbp,
	Rsi,
	Rdi,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
	Rip,
};

struct RegisterState {
	Register	reg;
	u64			val;
};