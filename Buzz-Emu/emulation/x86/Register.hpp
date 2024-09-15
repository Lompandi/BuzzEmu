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


constexpr u64 register_mask_full = 0xFFFFFFFFFFFFFFFF;

enum RegisterMask : u8{
	LowByte = 0x00,
	HighByte = 0xFF
};

constexpr u64 mask_regs_high = 0x000000000000FF00;
constexpr u64 mask_regs_low = 0x00000000000000FF;
u8 FetchByteRegs(u64 data, u64 mask);