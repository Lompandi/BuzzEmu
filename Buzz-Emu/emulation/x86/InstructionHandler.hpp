#pragma once

#include "DecodeContext.hpp"
#include "Instructions.hpp"
#include "../../emulator/Emulator.hpp"

#define BUZE_STANDARD_PARAM Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst

//if memonic collides, the number will be its opcode
//Addition
void Add_01(BUZE_STANDARD_PARAM);
void Add_03(BUZE_STANDARD_PARAM);
void Add_81(BUZE_STANDARD_PARAM);
void Add_83(BUZE_STANDARD_PARAM);
//Logical AND
void And_21(BUZE_STANDARD_PARAM);
void And_25(BUZE_STANDARD_PARAM);
//Subtraction
void Sub_29(BUZE_STANDARD_PARAM);
void Sub_2B(BUZE_STANDARD_PARAM);
void Sub_83(BUZE_STANDARD_PARAM);
//Exclusive OR
void Xor_31(BUZE_STANDARD_PARAM);
void Xor_33(BUZE_STANDARD_PARAM);
void Xor_35(BUZE_STANDARD_PARAM);
//Compare two operands

//Logical OR
void Or_09(BUZE_STANDARD_PARAM);
void Or_0D(BUZE_STANDARD_PARAM);

//Move
void Mov_8B(BUZE_STANDARD_PARAM);