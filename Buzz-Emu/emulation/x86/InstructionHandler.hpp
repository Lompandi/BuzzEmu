#pragma once

#include "DecodeContext.hpp"
#include "Instructions.hpp"
#include "../../emulator/Emulator.hpp"

#define BUZE_STANDARD_PARAM Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst

#define push(expr) emu.memory.Write(emu.Reg(Register::Rsp) - sizeof(expr), ##expr); emu.SetReg64(Register::Rsp, emu.Reg(Register::Rsp) - sizeof(expr))

#define pop(emu) _pop(##emu)

BZMU_FORCEINLINE u64 _pop(Emulator& emu) {
	auto pop_val = emu.memory.Read<u64>(emu.Reg(Register::Rsp)).value();
	emu.SetReg<u64>(Register::Rsp, emu.Reg(Register::Rsp) + sizeof(u64));
	return pop_val;
}
//if memonic collides, the number will be its opcode
// 
// TODO Instructions: 
// {0xCC (INT3)}, 
// {0xB0 (MOV r8, imm8  (OI))}, 
// 
//Addition
void Add_01(BUZE_STANDARD_PARAM);
void Add_03(BUZE_STANDARD_PARAM);
void Add_05(BUZE_STANDARD_PARAM);
void Add_81(BUZE_STANDARD_PARAM);
void Add_83(BUZE_STANDARD_PARAM);
//Logical AND
void And_21(BUZE_STANDARD_PARAM);
void And_25(BUZE_STANDARD_PARAM);
void And_81(BUZE_STANDARD_PARAM);
void And_83(BUZE_STANDARD_PARAM);
//Subtraction
void Sub_29(BUZE_STANDARD_PARAM);
void Sub_2B(BUZE_STANDARD_PARAM);
void Sub_81(BUZE_STANDARD_PARAM);
void Sub_83(BUZE_STANDARD_PARAM);
//Exclusive OR
void Xor_31(BUZE_STANDARD_PARAM);
void Xor_33(BUZE_STANDARD_PARAM);
void Xor_35(BUZE_STANDARD_PARAM);
void Xor_81(BUZE_STANDARD_PARAM);
void Xor_83(BUZE_STANDARD_PARAM);
//Compare two operands
void Cmp_3B(BUZE_STANDARD_PARAM);
void Cmp_83(BUZE_STANDARD_PARAM);
//Logical OR
void Or_09(BUZE_STANDARD_PARAM);
void Or_0D(BUZE_STANDARD_PARAM);
void Or_81(BUZE_STANDARD_PARAM);
void Or_83(BUZE_STANDARD_PARAM);
//Move
void Mov_88(BUZE_STANDARD_PARAM);
void Mov_89(BUZE_STANDARD_PARAM);
void Mov_8B(BUZE_STANDARD_PARAM);
void Mov_B8_BF(BUZE_STANDARD_PARAM);
void Mov_C7(BUZE_STANDARD_PARAM);
//Load effective address
void Lea_8D(BUZE_STANDARD_PARAM, u64& pc);
//Move signed extended
void Movsxd_63(BUZE_STANDARD_PARAM);
//Move zero-extended
void Movzx_0FB6(BUZE_STANDARD_PARAM);
//Logical compare
void Test_84(BUZE_STANDARD_PARAM);
void Test_85(BUZE_STANDARD_PARAM);
//Push values onto the stack
void Push_50_57(BUZE_STANDARD_PARAM); 
//Pop values from the stack
void Pop_58_5F(BUZE_STANDARD_PARAM);
//Call procedure
void Call_E8(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst, u64& pc);
void Call_FF_reg2(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst, u64& pc);
//Decrement by 1
void Dec_FF(BUZE_STANDARD_PARAM);
//Jump to address
void Jmp_E9(BUZE_STANDARD_PARAM, u64& pc);
//Return from procedure
void Ret_C3(BUZE_STANDARD_PARAM, u64& pc);