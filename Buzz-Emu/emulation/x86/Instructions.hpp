#pragma once

#include <vector>
#include "Sib.hpp"
#include "ModRM.hpp"
#include "../../core/Fs.hpp"
#include "../../core/Memtypes.hpp"

//TODO: Add REX extends-register(R8-R15) processing code into macros

enum Instruction : u32 {
	ADD_01 = 0x01,
	ADD_03 = 0x03,
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
	PUSH_50 = 0x50,
	PUSH_51 = 0x51,
	PUSH_52 = 0x52,
	PUSH_53 = 0x53,
	PUSH_54 = 0x54,
	PUSH_55 = 0x55,
	PUSH_56 = 0x56,
	PUSH_57 = 0x57,
	MOVSXD_63 = 0x63,
	JZ_74 = 0x74,
	JL_7C = 0x7C,
	_81 = 0x81,
	_83 = 0x83,
	TEST_85 = 0x85,
	MOV_89 = 0x89,
	MOV_8B = 0x8B,
	NOP = 0x90,
	TEST_A8 = 0xA8,
	TEST_A9 = 0xA9,
	MOV_B8 = 0xB8,
	CALL_E8 = 0xE8,
	JMP_EB = 0xEB,
	_FF = 0xFF,
};

#define GET_L_REG(r) static_cast<u8>(r & 0xFF)
#define GET_H_REG(r) static_cast<u8>((r >> 8) & 0xFF)
#define GET_X_REG(r) static_cast<u16>(r & 0xFFFF)
#define GET_EXT_REG(r) static_cast<u32>(r & 0xFFFFFFFF)

#define GET_OPSIZE_ENUM(o) (o & 0x00000003)

#define INSTRUCTION_LOGICAL_AX_IMM(name, opcode, op_operator) \
void name##_##opcode(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst) { \
u64 result; \
if (ctx->osize == X86_Osize_16bit && inst.size() == 3) { \
	result = GET_X_REG(emu.Reg(Register::Rax)) ##op_operator ReadFromVec<u16>(inst, 1); \
	emu.SetReg(Register::Rax, result); \
	SetLogicOpFlags(emu.flags, result); \
} \
else if (ctx->osize == X86_Osize_32bit && inst.size() == 5) { \
	result = GET_EXT_REG(emu.Reg(Register::Rax)) ##op_operator ReadFromVec<u32>(inst, 1); \
	emu.SetReg(Register::Rax, result); \
	SetLogicOpFlags(emu.flags, result); \
} \
else if (ctx->osize == X86_Osize_64bit && inst.size() == 5) { \
	result = emu.Reg(Register::Rax) ##op_operator static_cast<u64>(ReadFromVec<u32>(inst, 1)); \
	emu.SetReg(Register::Rax, result); \
	SetLogicOpFlags(emu.flags, result); \
} \
} \

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
			emu.SetReg(mod_rm.RM_Mod.reg, ##exp_2reg16);																\
		}																								\
		else if (opsize == OperandSize::X86_Osize_32bit) {												\
			emu.SetReg(mod_rm.RM_Mod.reg, ##exp_2reg32);															\
		}																								\
		else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {						\
			emu.SetReg(mod_rm.RM_Mod.reg,##exp_2reg64);																		\
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

//exclude imm8
//TODO: use GET_OSIZE_ENUM on all of the operand size switches(and ofc, change osize from if-else to switch first if not done)
#define INSTRUCTION_OP2_MI(name, opcode, \
exp_reg16_imm, \
exp_reg32_imm,	\
exp_reg64_imm, \
\
exp_mem16_imm, \
exp_mem32_imm, \
exp_mem64_imm, \
\
exp_memdisp16_imm, \
exp_memdisp32_imm, \
exp_memdisp64_imm, \
\
sib_disp16_imm,\
sib_disp32_imm,\
sib_disp64_imm,\
\
sib16_imm,\
sib32_imm,\
sib64_imm\
) \
void name##_##opcode(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst) {								\
OperandSize opsize = ctx->osize;																		\
																										\
ModRM mod_rm;																							\
Handle_ModRM(emu, ctx, mod_rm);														\
																										\
if (!mod_rm.RM_Mod.disp && !mod_rm.RM_Mod.RMRegSet)		\
return;\
std::cout << "ModR/M rmreg set: " << (bool)mod_rm.RM_Mod.RMRegSet << "\n";\
std::cout << "ModR/M disp: " << (bool)mod_rm.RM_Mod.disp << "\n"; \
std::cout << "\nMI instruction: processing...\n";	\
if (!ctx->p_sib) {																						\
	if (!mod_rm.RM_Mod.IsPtr && mod_rm.RM_Mod.RMRegSet) { 		\
		switch(GET_OPSIZE_ENUM(ctx->osize))	{									\
		case OperandSize::X86_Osize_16bit :\
			emu.SetReg(mod_rm.RM_Mod.reg, ##exp_reg16_imm);\
			break;\
		case OperandSize::X86_Osize_32bit:	\
			emu.SetReg(mod_rm.RM_Mod.reg, ##exp_reg32_imm);\
			break;\
		case OperandSize::X86_Osize_64bit:	\
			emu.SetReg(mod_rm.RM_Mod.reg,##exp_reg64_imm);\
			break;\
		default: \
			std::cout << "MI: Unknown opsize \n";\
			break;\
		}\
	}																									\
	else if (!mod_rm.RM_Mod.disp && mod_rm.RM_Mod.RMRegSet) { 								\
		std::cout << "MI: Reading the op data...\n";\
		switch(GET_OPSIZE_ENUM(ctx->osize)){				\
		case OperandSize::X86_Osize_16bit:													\
			emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val), ToByteVector(##exp_mem16_imm));														\
			break;									\
		case OperandSize::X86_Osize_32bit:												\
			emu.memory.WriteFrom(GET_EXT_REG(mod_rm.RM_Mod.reg_val), ToByteVector(##exp_mem32_imm));														\
			break;																								\
		case OperandSize::X86_Osize_64bit:		\
			emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val,	ToByteVector(##exp_mem64_imm));	\
			break; \
		default: \
			break;\
		}\
	}																									\
	else if (mod_rm.RM_Mod.RMRegSet && mod_rm.RM_Mod.disp) { 			\
		s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 2).value();								\
																										\
		if (opsize == OperandSize::X86_Osize_16bit) {													\
			emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp,								\
				ToByteVector(##exp_memdisp16_imm));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_32bit) {												\
			emu.memory.WriteFrom(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp,								\
				ToByteVector(##exp_memdisp32_imm));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {						\
			emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val + disp,											\
				ToByteVector(##exp_memdisp64_imm));																	\
		}																								\
	}		\
	else if(!mod_rm.RM_Mod.RMRegSet && mod_rm.RM_Mod.disp) {\
		s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 2).value();								\
																										\
		if (opsize == OperandSize::X86_Osize_16bit) {													\
			emu.memory.WriteFrom(disp,								\
				ToByteVector(##exp_memdisp16_imm));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_32bit) {												\
			emu.memory.WriteFrom(disp,								\
				ToByteVector(##exp_memdisp32_imm));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {						\
			emu.memory.WriteFrom(disp,											\
				ToByteVector(##exp_memdisp64_imm));																	\
		}		\
	}																							\
																			\
}																										\
else {																									\
	Sib sib_byte;																						\
	u64 calc_offset = 0;																				\
	HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);													\
	if (!sib_byte.valid)																				\
		return;\
																										\
	if (mod_rm.RM_Mod.disp) { 													\
		s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 3).value();								\
		if (opsize == OperandSize::X86_Osize_16bit) {\
			emu.memory.WriteFrom(calc_offset + disp,ToByteVector(##sib_disp16_imm));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_32bit) {		\
			emu.memory.WriteFrom(calc_offset + disp,ToByteVector(##sib_disp32_imm));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {\
			emu.memory.WriteFrom(calc_offset + disp,ToByteVector(##sib_disp64_imm));																	\
		}																								\
	}																									\
	else { 																	\
		if (opsize == OperandSize::X86_Osize_16bit) {\
			emu.memory.WriteFrom(calc_offset,ToByteVector(##sib16_imm));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_32bit) {	\
			emu.memory.WriteFrom(calc_offset,ToByteVector(##sib32_imm));														\
		}																								\
		else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {\
			emu.memory.WriteFrom(calc_offset,ToByteVector(##sib64_imm));																	\
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
                emu.SetReg(mod_rm.RM_Mod.reg, result); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_32bit) { \
                result = GET_EXT_REG(mod_rm.RM_Mod.reg_val) ##op_operator GET_EXT_REG(mod_rm.Reg.val); \
                emu.SetReg(mod_rm.RM_Mod.reg, result); \
                SetLogicOpFlags(emu.flags, result); \
            } \
            else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) { \
                result = mod_rm.RM_Mod.reg_val ##op_operator mod_rm.Reg.val; \
                emu.SetReg(mod_rm.RM_Mod.reg, result); \
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


/*
for displacement, there will be a varible name "disp" containing the displacement value
for SIB offset, it will be "calc_offset"
*/
#define INSTRUCTION_OP2_RM(name, opcode, \
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
void name##_##opcode(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst) {\
OperandSize opsize = ctx->osize; ModRM mod_rm; Handle_ModRM(emu, ctx, mod_rm); if (!mod_rm.RM_Mod.disp && !mod_rm.RM_Mod.RMRegSet) return;if (!ctx->p_sib) {				\
	if (!mod_rm.RM_Mod.IsPtr && mod_rm.RM_Mod.RMRegSet) {																																				\
		if (opsize == OperandSize::X86_Osize_16bit) {																														\
			emu.SetReg(mod_rm.Reg.reg, ##exp_2reg16);				\
		}																																									\
		else if (opsize == OperandSize::X86_Osize_32bit) {																													\
			emu.SetReg(mod_rm.Reg.reg, ##exp_2reg32);		\
		}																																									\
		else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {																						\
			emu.SetReg(mod_rm.Reg.reg, ##exp_2reg64);																	\
		}																																									\
	}																																										\
	else if (!mod_rm.RM_Mod.disp) {																																			\
		if (opsize == OperandSize::X86_Osize_16bit) {																														\
			emu.SetReg(mod_rm.Reg.reg, ##exp_mem_reg16);\
		}																																													 \
		else if (opsize == OperandSize::X86_Osize_32bit) {																																	 \
			emu.SetReg(mod_rm.Reg.reg, ##exp_mem_reg32);\
		}																																															 \
		else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {																												 \
			emu.SetReg(mod_rm.Reg.reg, ##exp_mem_reg64);																 \
		}																																															 \
	}																																																 \
	else if (mod_rm.RM_Mod.RMRegSet && mod_rm.RM_Mod.disp) {																																				 \
		s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 2).value(); if (opsize == OperandSize::X86_Osize_16bit) {																				 \
			emu.SetReg(mod_rm.Reg.reg, ##exp_memdisp_reg16); \
		}																																															 \
		else if (opsize == OperandSize::X86_Osize_32bit) {																																			 \
			emu.SetReg(mod_rm.Reg.reg, ##exp_memdisp_reg32);\
		}																																																	\
		else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {																														\
			emu.SetReg(mod_rm.Reg.reg, ##exp_memdisp_reg64);																\
		}																																																	\
	}																																																		\
}																																																			\
else {											\
	std::cout << "Entering SIB procession...\n";																																							\
	Sib sib_byte;u64 calc_offset = 0;HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);if(!sib_byte.valid) return; if (mod_rm.RM_Mod.disp) {																												\
		s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 3).value(); if (opsize == OperandSize::X86_Osize_16bit) {																						\
			emu.SetReg(mod_rm.Reg.reg, ##sib_disp_16);\
		}																																																			   \
		else if (opsize == OperandSize::X86_Osize_32bit) {																																							   \
			emu.SetReg(mod_rm.Reg.reg, ##sib_disp_32);\
		}																																																				   \
		else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {																																	   \
			emu.SetReg(mod_rm.Reg.reg, ##sib_disp_64);							   \
		}																																																				   \
	}																																																					   \
	else {																																																				   \
		if (opsize == OperandSize::X86_Osize_16bit) {																																									   \
			emu.SetReg(mod_rm.Reg.reg, ##sib_16);		   \
		}																																																				   \
		else if (opsize == OperandSize::X86_Osize_32bit) {																																								   \
			emu.SetReg(mod_rm.Reg.reg, ##sib_32);	   \
		}																																																				   \
		else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {																																	   \
			emu.SetReg(mod_rm.Reg.reg, ##sib_64);									   \
		}																																																				   \
	}																																																					   \
} return;\
}


//c++20 function def:

//helper function to unpack :
// Helper function to unpack arguments from a tuple and call a function
template <typename FuncType, typename... ExtraArgs>
auto CallInstrEmuFunc(FuncType func, u64 value1, u64 value2, ExtraArgs... extraArgs)
-> decltype(func(value1, value2, extraArgs...)) {
    return func(value1, value2, extraArgs...);
}

//template <typename FuncType, typename OPtype16, typename OPtype32, typename OPtype64, typename... ExtraArgs>
//void def_instruction_op2_RM(FuncType instr_emu_func, u64 op1, u64 op2, ExtraArgs... extra_args) {

//}

//op2 in here will be automatically placed by the offset, so we will only needs to be dealing with op1
template <typename FuncType,
	typename OPtype16, typename OPtype32, typename OPtype64,
	typename Immtype16, typename Immtype32, typename Immtype64,
	typename... ExtraArgs>
void def_instruction_op2_MI(
    Emulator& emu,
    x86Dcctx* ctx,
    const std::vector<u8>& inst,
    FuncType instr_emu_func,
    ModRM& mod_rm,
    u64 op1,
    u32 offset_to_imm,
    ExtraArgs... extra_args)
{
	OperandSize opsize = ctx->osize;
	if (!mod_rm.RM_Mod.disp && !mod_rm.RM_Mod.RMRegSet)
		return;

	std::cout << "ModR/M rmreg set: " << (bool)mod_rm.RM_Mod.RMRegSet << "\n";
	std::cout << "ModR/M disp: " << (bool)mod_rm.RM_Mod.disp << "\n";
	std::cout << "\nMI instruction: processing...\n";
	if (!ctx->p_sib) {
		if (!mod_rm.RM_Mod.IsPtr && mod_rm.RM_Mod.RMRegSet) {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.SetReg(mod_rm.RM_Mod.reg, CallInstrEmuFunc(
					instr_emu_func,
					static_cast<OPtype16>(op1),
					static_cast<OPtype16>(ReadFromVec<Immtype16>(inst, offset_to_imm)),
					extra_args...
				));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.SetReg(mod_rm.RM_Mod.reg, CallInstrEmuFunc(
					instr_emu_func,
					static_cast<OPtype32>(op1),
					static_cast<OPtype32>(ReadFromVec<Immtype32>(inst, offset_to_imm)),
					extra_args...
				));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.SetReg(mod_rm.RM_Mod.reg, CallInstrEmuFunc(
					instr_emu_func,
					static_cast<OPtype64>(op1),
					static_cast<OPtype64>(ReadFromVec<Immtype64>(inst, offset_to_imm)),
					extra_args...
				));
				break;
			default:
				std::cout << "MI: Unknown opsizen";
				break;
			}
		}
		else if (!mod_rm.RM_Mod.disp && mod_rm.RM_Mod.RMRegSet) {
			std::cout << "MI: Reading the op data...\n";
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val), ToByteVector(CallInstrEmuFunc(
					instr_emu_func,
					emu.memory.Read<OPtype16>(static_cast<OPtype16>(op1)).value(),
					static_cast<OPtype16>(ReadFromVec<Immtype16>(inst, offset_to_imm)),
					extra_args...
				)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val), ToByteVector(CallInstrEmuFunc(
					instr_emu_func,
					emu.memory.Read<OPtype32>(static_cast<OPtype32>(op1)).value(),
					static_cast<OPtype32>(ReadFromVec<Immtype32>(inst, offset_to_imm)),
					extra_args...
				)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val), ToByteVector(CallInstrEmuFunc(
					instr_emu_func,
					emu.memory.Read<OPtype64>(static_cast<OPtype64>(op1)).value(),
					static_cast<OPtype64>(ReadFromVec<Immtype64>(inst, offset_to_imm)),
					extra_args...
				)));
				break;
			default:
				break;
			}
		}
		else if (mod_rm.RM_Mod.RMRegSet && mod_rm.RM_Mod.disp) {
			s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 2).value();

			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp,
					ToByteVector(CallInstrEmuFunc(
						instr_emu_func,
						emu.memory.Read<OPtype16>(static_cast<OPtype16>(op1) + disp).value(),
						static_cast<OPtype16>(ReadFromVec<Immtype16>(inst, offset_to_imm)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp,
					ToByteVector(CallInstrEmuFunc(
						instr_emu_func,
						emu.memory.Read<OPtype32>(static_cast<OPtype32>(op1) + disp).value(),
						static_cast<OPtype32>(ReadFromVec<Immtype32>(inst, offset_to_imm)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val + disp,
					ToByteVector(CallInstrEmuFunc(
						instr_emu_func,
						emu.memory.Read<OPtype64>(static_cast<OPtype64>(op1) + disp).value(),
						static_cast<OPtype64>(ReadFromVec<Immtype64>(inst, offset_to_imm)),
						extra_args...
					)));
				break;
			default:
				break;
			}
		}		
		/*Only displacement*/
		else if (!mod_rm.RM_Mod.RMRegSet && mod_rm.RM_Mod.disp) {
			s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 2).value();

			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(disp,
					ToByteVector(CallInstrEmuFunc(
						instr_emu_func,
						emu.memory.Read<OPtype16>(disp).value(),
						static_cast<OPtype16>(ReadFromVec<Immtype16>(inst, offset_to_imm)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(disp,
					ToByteVector(CallInstrEmuFunc(
						instr_emu_func,
						emu.memory.Read<OPtype32>(disp).value(),
						static_cast<OPtype32>(ReadFromVec<Immtype32>(inst, offset_to_imm)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(disp,
					ToByteVector(CallInstrEmuFunc(
						instr_emu_func,
						emu.memory.Read<OPtype64>(disp).value(),
						static_cast<OPtype64>(ReadFromVec<Immtype64>(inst, offset_to_imm)),
						extra_args...
					)));
				break;
			default:
				break;
			}
		}
	}
	else {
		Sib sib_byte;
		u64 calc_offset = 0;
		HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);
		if (!sib_byte.valid)
			return;
		
		if (mod_rm.RM_Mod.disp) {
			s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 3).value();
			
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(calc_offset + disp, ToByteVector(CallInstrEmuFunc(
					instr_emu_func,
					emu.memory.Read<OPtype16>(calc_offset + disp).value(),
					static_cast<OPtype16>(ReadFromVec<Immtype16>(inst, offset_to_imm)),
					extra_args...
				)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(calc_offset + disp, ToByteVector(CallInstrEmuFunc(
					instr_emu_func,
					emu.memory.Read<OPtype32>(calc_offset + disp).value(),
					static_cast<OPtype32>(ReadFromVec<Immtype32>(inst, offset_to_imm)),
					extra_args...
				)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(calc_offset + disp, ToByteVector(CallInstrEmuFunc(
					instr_emu_func,
					emu.memory.Read<OPtype64>(calc_offset + disp).value(),
					static_cast<OPtype64>(ReadFromVec<Immtype64>(inst, offset_to_imm)),
					extra_args...
				)));
				break;
			}
		}
		else {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(calc_offset, ToByteVector(CallInstrEmuFunc(
					instr_emu_func,
					emu.memory.Read<OPtype16>(calc_offset).value(),
					static_cast<OPtype16>(ReadFromVec<Immtype16>(inst, offset_to_imm)),
					extra_args...
				)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(calc_offset, ToByteVector(CallInstrEmuFunc(
					instr_emu_func,
					emu.memory.Read<OPtype32>(calc_offset).value(),
					static_cast<OPtype32>(ReadFromVec<Immtype32>(inst, offset_to_imm)),
					extra_args...
				)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(calc_offset, ToByteVector(CallInstrEmuFunc(
					instr_emu_func,
					emu.memory.Read<OPtype64>(calc_offset).value(),
					static_cast<OPtype64>(ReadFromVec<Immtype64>(inst, offset_to_imm)),
					extra_args...
				)));
				break;
			}
		}
	}
	return;	
}
