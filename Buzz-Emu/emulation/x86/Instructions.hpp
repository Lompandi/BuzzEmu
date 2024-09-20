#pragma once

#include <vector>

#include "OperandEncoding.hpp"

//TODO: Add REX extends-register(R8-R15) processing code into macros

enum Instruction : u32 {
	ADD_01 = 0x01,
	ADD_03 = 0x03,
	ADD_05 = 0x05,
	OR_09 = 0x09,
	OR_0C = 0x0C,
	OR_0D = 0x0D,
	AND_21 = 0x21,
	AND_24 = 0x24,
	AND_25 = 0x25,
	SUB_29 = 0x29,
	SUB_2B = 0x2B,
	SUB_2C = 0x2C,
	SUB_2D = 0x2D,
	XOR_31 = 0x31,
	XOR_33 = 0x33,
	XOR_34 = 0x34,
	XOR_35 = 0X35,
	CMP_3B = 0x3B,
	PUSH_50 = 0x50,
	PUSH_51 = 0x51,
	PUSH_52 = 0x52,
	PUSH_53 = 0x53,
	PUSH_54 = 0x54,
	PUSH_55 = 0x55,
	PUSH_56 = 0x56,
	PUSH_57 = 0x57,
	POP_58 = 0x58,
	POP_59 = 0x59,
	POP_5A = 0x5A,
	POP_5B = 0x5B,
	POP_5C = 0x5C,
	POP_5D = 0x5D,
	POP_5E = 0x5E,
	POP_5F = 0x5F,
	MOVSXD_63 = 0x63,
	JZ_74 = 0x74,
	JNZ_75 = 0x75,
	JL_7C = 0x7C,
	JLE_7E = 0x7E,
	_81 = 0x81,
	_83 = 0x83,
	TEST_84 = 0x84,
	TEST_85 = 0x85,
	MOV_88 = 0x88,
	MOV_89 = 0x89,
	MOV_8B = 0x8B,
	LEA_8D = 0x8D,
	NOP = 0x90,
	TEST_A8 = 0xA8,
	TEST_A9 = 0xA9,
	MOV_B8 = 0xB8,
	MOV_B9 = 0xB9,
	MOV_BA = 0xBA,
	MOV_BB = 0xBB,
	MOV_BC = 0xBC,
	MOV_BD = 0xBD,
	MOV_BE = 0xBE,
	MOV_BF = 0xBF,
	RET_C3 = 0xC3,
	MOV_C7 = 0xC7,
	CALL_E8 = 0xE8,
	JMP_E9 = 0xE9,
	JMP_EB = 0xEB,
	_FF = 0xFF,
	SYSCALL_0F05 = 0x0F05,
	MOVZX_0FB6 = 0x0FB6,
};