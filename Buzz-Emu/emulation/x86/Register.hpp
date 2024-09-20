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

enum RFlagsRegister : u32 {
	CF = 0,   // Carry Flag
	PF = 2,   // Parity Flag
	AF = 4,   // Auxiliary Carry Flag
	ZF = 6,   // Zero Flag
	SF = 7,   // Sign Flag
	TF = 8,   // Trap Flag
	IF = 9,   // Interrupt Enable Flag
	DF = 10,  // Direction Flag
	OF = 11,  // Overflow Flag
	IOPL = 12,// I/O Privilege Level
	NT = 14,  // Nested Task
	RF = 16,  // Resume Flag
	VM = 17,  // Virtual-8086 Mode
	AC = 18,  // Alignment Check / Access Control
	VIF = 19, // Virtual Interrupt Flag
	VIP = 20, // Virtual Interrupt Pending
	ID = 21   // ID Flag
	// Bits 1, 3, 5, 15, and 22-63 are reserved
};

constexpr u64 register_mask_full = 0xFFFFFFFFFFFFFFFF;

enum ByteRegister : u8 {
	LowByte = 0x00,
	HighByte = 0xFF,
};

constexpr u64 mask_regs_high = 0x000000000000FF00;
constexpr u64 mask_regs_low = 0x00000000000000FF;

struct RegisterState {
	Register	 reg;
	ByteRegister h_l;
	u64			 val;
};

u8 get_byte_register(u64 data, u64 mask);
