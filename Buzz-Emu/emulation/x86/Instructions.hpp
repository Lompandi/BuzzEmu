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
	CALL_E8 = 0xE8,
	JMP_E9 = 0xE9,
	JMP_EB = 0xEB,
	_FF = 0xFF,
	SYSCALL_0F05 = 0x0F05,
	MOVZX_0FB6 = 0x0FB6,
};

#define INSTRUCTION_OP2_MR(name, opcode, \
exp_2reg16, \
exp_2reg32,	\
exp_2reg64, \
\
exp_mem_reg16, \
exp_mem_reg32, \
exp_mem_reg64, \
\
exp_memdisp_reg16, \
exp_memdisp_reg32, \
exp_memdisp_reg64, \
\
sib_disp_16,\
sib_disp_32,\
sib_disp_64,\
\
sib_16,\
sib_32,\
sib_64\
) \
void name##_##opcode(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst) {								\
OperandSize opsize = ctx->osize;	\
																										\
ModRM mod_rm;																							\
Handle_ModRM(emu, ctx, mod_rm);																			\
																										\
if (!mod_rm.RM_Mod.disp && !mod_rm.RM_Mod.RMRegSet)		\
return;																									\
if (!ctx->p_sib) {																						\
	if (!mod_rm.RM_Mod.IsPtr && mod_rm.RM_Mod.RMRegSet) { 													\
		if (opsize == OperandSize::X86_Osize_16bit) {													\
			emu.SetReg<OPtype64>(mod_rm.RM_Mod.reg, ##exp_2reg16);																\
		}																								\
		else if (opsize == OperandSize::X86_Osize_32bit) {												\
			emu.SetReg<OPtype64>(mod_rm.RM_Mod.reg, ##exp_2reg32);															\
		}																								\
		else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {						\
			emu.SetReg<OPtype64>(mod_rm.RM_Mod.reg,##exp_2reg64);																		\
		}																								\
	}																									\
	else if (!mod_rm.RM_Mod.disp) { 								\
		if (opsize == OperandSize::X86_Osize_16bit) {													\
			emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val), ToByteVector(##exp_mem_reg16));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_32bit) {												\
			emu.memory.WriteFrom(GET_EXT_REG(mod_rm.RM_Mod.reg_val), ToByteVector(##exp_mem_reg32));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {						\
			emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val,	ToByteVector(##exp_mem_reg64));																	\
		}																								\
	}																									\
	else if (mod_rm.RM_Mod.RMRegSet && mod_rm.RM_Mod.disp) { 			\
		s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 2).value();								\
																										\
		if (opsize == OperandSize::X86_Osize_16bit) {													\
			emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp,								\
				ToByteVector(##exp_memdisp_reg16));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_32bit) {												\
			emu.memory.WriteFrom(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp,								\
				ToByteVector(##exp_memdisp_reg32));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {						\
			emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val + disp,											\
				ToByteVector(##exp_memdisp_reg64));																	\
		}																								\
	}																									\
																			\
}																										\
else {																									\
	Sib sib_byte;\
	u64 calc_offset = 0;\
	HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);\
	if (!sib_byte.valid)\
		return;	\
	std::cout << "entering sib proccessing...\n";\
																										\
	if (mod_rm.RM_Mod.disp) { 													\
		s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 3).value();								\
		if (opsize == OperandSize::X86_Osize_16bit) {\
			emu.memory.WriteFrom(calc_offset + disp,ToByteVector(##sib_disp_16));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_32bit) {		\
			emu.memory.WriteFrom(calc_offset + disp,ToByteVector(##sib_disp_32));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {\
			emu.memory.WriteFrom(calc_offset + disp,ToByteVector(##sib_disp_64));																	\
		}																								\
	}																									\
	else { 																	\
		if (opsize == OperandSize::X86_Osize_16bit) {\
			emu.memory.WriteFrom(calc_offset,ToByteVector(##sib_16));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_32bit) {	\
			emu.memory.WriteFrom(calc_offset,ToByteVector(##sib_32));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {\
			emu.memory.WriteFrom(calc_offset,ToByteVector(##sib_64));																	\
		}																								\
	}																									\
}																										\
return;	\
}

#define INSTRUCTION_LOGICAL_OP2_RM_REG(name, opcode, op_operator) \
void name##_##opcode(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst) { \
    OperandSize opsize = ctx->osize; \
 \
    ModRM mod_rm; \
    Handle_ModRM(emu, ctx, mod_rm); \
 \
    if (!mod_rm.RM_Mod.disp && !mod_rm.RM_Mod.RMRegSet) \
        return; \
    uint64_t result; \
 \
    if (!ctx->p_sib) { \
        if (!mod_rm.RM_Mod.IsPtr) { \
            if (opsize == OperandSize::X86_Osize_16bit) { \
                result = GET_X_REG(mod_rm.RM_Mod.reg_val) ##op_operator GET_X_REG(mod_rm.Reg.val); \
                emu.SetReg<OPtype64>(mod_rm.RM_Mod.reg, result); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_32bit) { \
                result = GET_EXT_REG(mod_rm.RM_Mod.reg_val) ##op_operator GET_EXT_REG(mod_rm.Reg.val); \
                emu.SetReg<OPtype64>(mod_rm.RM_Mod.reg, result); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) { \
                result = mod_rm.RM_Mod.reg_val ##op_operator mod_rm.Reg.val; \
                emu.SetReg<OPtype64>(mod_rm.RM_Mod.reg, result); \
                SetLogicOpFlags(emu.flags, result); \
            } \
        } \
        else if (!mod_rm.RM_Mod.disp) { \
            if (opsize == OperandSize::X86_Osize_16bit) { \
                result = emu.memory.Read<uint16_t>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value() ##op_operator GET_X_REG(mod_rm.Reg.val); \
                emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val), ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_32bit) { \
                result = emu.memory.Read<uint32_t>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value() ##op_operator GET_EXT_REG(mod_rm.Reg.val); \
                emu.memory.WriteFrom(GET_EXT_REG(mod_rm.RM_Mod.reg_val), ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) { \
                result = emu.memory.Read<uint64_t>(mod_rm.RM_Mod.reg_val).value() ##op_operator mod_rm.Reg.val; \
                emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val, ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
        } \
        else if (mod_rm.RM_Mod.reg && mod_rm.RM_Mod.disp) { \
            int64_t disp = ReadDispFromVec<int64_t>(inst, mod_rm.RM_Mod.disp, 2).value(); \
 \
            if (opsize == OperandSize::X86_Osize_16bit) { \
                result = emu.memory.Read<uint16_t>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value() ##op_operator GET_X_REG(mod_rm.Reg.val); \
                emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp, ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_32bit) { \
                result = emu.memory.Read<uint32_t>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value() ##op_operator GET_EXT_REG(mod_rm.Reg.val); \
                emu.memory.WriteFrom(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp, ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) { \
                result = emu.memory.Read<uint64_t>(mod_rm.RM_Mod.reg_val + disp).value() ##op_operator mod_rm.Reg.val; \
                emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val + disp, ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
        } \
    } \
    else { \
        Sib sib_byte; \
        uint64_t calc_offset = 0; \
		HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset); if(!sib_byte.valid) return;\
        if (mod_rm.RM_Mod.disp) { \
            int64_t disp = ReadDispFromVec<int64_t>(inst, mod_rm.RM_Mod.disp, 3).value(); \
            if (opsize == OperandSize::X86_Osize_16bit) { \
                result = emu.memory.Read<uint16_t>(calc_offset + disp).value() ##op_operator GET_X_REG(mod_rm.Reg.val); \
                emu.memory.WriteFrom(calc_offset + disp, ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_32bit) { \
                result = emu.memory.Read<uint32_t>(calc_offset + disp).value() ##op_operator GET_EXT_REG(mod_rm.Reg.val); \
                emu.memory.WriteFrom(calc_offset + disp, ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) { \
                result = emu.memory.Read<uint64_t>(calc_offset + disp).value() ##op_operator mod_rm.Reg.val; \
                emu.memory.WriteFrom(calc_offset + disp, ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
        } \
        else { \
            if (opsize == OperandSize::X86_Osize_16bit) { \
                result = emu.memory.Read<uint16_t>(calc_offset).value() ##op_operator GET_X_REG(mod_rm.Reg.val); \
                emu.memory.WriteFrom(calc_offset, ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_32bit) { \
                result = emu.memory.Read<uint32_t>(calc_offset).value() ##op_operator GET_EXT_REG(mod_rm.Reg.val); \
                emu.memory.WriteFrom(calc_offset, ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) { \
                result = emu.memory.Read<uint64_t>(calc_offset).value() ##op_operator mod_rm.Reg.val; \
                emu.memory.WriteFrom(calc_offset, ToByteVector(result)); \
                SetLogicOpFlags(emu.flags, result); \
            } \
        } \
    } \
    return; \
}