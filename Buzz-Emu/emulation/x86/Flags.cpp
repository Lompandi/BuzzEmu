
#include <bit>
#include <iostream>

#include "Flags.hpp"

u64 mov_operation(u64 dst, u64 src) {
    std::cout << "Moving 0x" << std::hex << src << "\n";
    return src;
}

u64 SubAndSetFlags(RflagsRegister& flags, uint64_t minuend, uint64_t subtrahend) {
    // Perform the subtraction
    uint64_t result = minuend - subtrahend;

    // Set the Carry Flag (CF)
    flags.CF = (minuend < subtrahend) ? 1 : 0;

    // Set the Overflow Flag (OF)
    bool signMinuend = (minuend & 0x8000000000000000) != 0;
    bool signSubtrahend = (subtrahend & 0x8000000000000000) != 0;
    bool signResult = (result & 0x8000000000000000) != 0;
    flags.OF = (signMinuend != signSubtrahend && signResult != signMinuend) ? 1 : 0;

    // Set the Zero Flag (ZF)
    flags.ZF = (result == 0) ? 1 : 0;

    // Set the Sign Flag (SF)
    flags.SF = (result & 0x8000000000000000) ? 1 : 0;

    // Set the Parity Flag (PF)
    uint64_t ones = std::popcount(result);
    flags.PF = (ones % 2 == 0) ? 1 : 0; // Even parity

    // Set the Auxiliary Carry Flag (AF)
    // AF is set if there is a borrow from the lower nibble to the upper nibble in any byte of the result
    flags.AF = 0;
    for (int i = 0; i < 8; ++i) {
        uint8_t byteMinuend = (minuend >> (i * 8)) & 0xFF;
        uint8_t byteSubtrahend = (subtrahend >> (i * 8)) & 0xFF;
        uint8_t byteResult = (result >> (i * 8)) & 0xFF;

        // Check if there is a borrow between the lower and upper nibbles
        if (((byteMinuend & 0x0F) < (byteSubtrahend & 0x0F)) ||
            ((byteMinuend & 0xF0) < (byteSubtrahend & 0xF0))) {
            flags.AF = 1;
            break;
        }
    }

    return result;
}

void test_void_func(u64 i, u64 j, RflagsRegister& flags) {

}

//overload func 2:
u64 SubAndSetFlags(uint64_t minuend, uint64_t subtrahend, RflagsRegister& flags) {
    // Perform the subtraction
    uint64_t result = minuend - subtrahend;

    // Set the Carry Flag (CF)
    flags.CF = (minuend < subtrahend) ? 1 : 0;

    // Set the Overflow Flag (OF)
    bool signMinuend = (minuend & 0x8000000000000000) != 0;
    bool signSubtrahend = (subtrahend & 0x8000000000000000) != 0;
    bool signResult = (result & 0x8000000000000000) != 0;
    flags.OF = (signMinuend != signSubtrahend && signResult != signMinuend) ? 1 : 0;

    // Set the Zero Flag (ZF)
    flags.ZF = (result == 0) ? 1 : 0;

    // Set the Sign Flag (SF)
    flags.SF = (result & 0x8000000000000000) ? 1 : 0;

    // Set the Parity Flag (PF)
    uint64_t ones = std::popcount(result);
    flags.PF = (ones % 2 == 0) ? 1 : 0; // Even parity

    // Set the Auxiliary Carry Flag (AF)
    // AF is set if there is a borrow from the lower nibble to the upper nibble in any byte of the result
    flags.AF = 0;
    for (int i = 0; i < 8; ++i) {
        uint8_t byteMinuend = (minuend >> (i * 8)) & 0xFF;
        uint8_t byteSubtrahend = (subtrahend >> (i * 8)) & 0xFF;
        uint8_t byteResult = (result >> (i * 8)) & 0xFF;

        // Check if there is a borrow between the lower and upper nibbles
        if (((byteMinuend & 0x0F) < (byteSubtrahend & 0x0F)) ||
            ((byteMinuend & 0xF0) < (byteSubtrahend & 0xF0))) {
            flags.AF = 1;
            break;
        }
    }

    return result;
}

/*return = operand1 + operand2*/
u64 AddAndSetFlags(u64 operand1, u64 operand2, RflagsRegister& flags) {
    u64 result = operand1 + operand2;

    // Set the Carry Flag (CF)
    flags.CF = (result < operand1) ? 1 : 0;

    // Set the Overflow Flag (OF)
    bool sign1 = (operand1 & 0x8000000000000000) != 0;
    bool sign2 = (operand2 & 0x8000000000000000) != 0;
    bool signResult = (result & 0x8000000000000000) != 0;
    flags.OF = (sign1 == sign2 && signResult != sign1) ? 1 : 0;

    // Set the Zero Flag (ZF)
    flags.ZF = (result == 0) ? 1 : 0;

    // Set the Sign Flag (SF)
    flags.SF = (result & 0x8000000000000000) ? 1 : 0;

    // Set the Parity Flag (PF)
    u64 ones = std::popcount(result);
    flags.PF = (ones % 2 == 0) ? 1 : 0; // Even parity

    // Set the Auxiliary Carry Flag (AF)
    flags.AF = 0;
    for (int i = 0; i < 8; ++i) {
        u8 byteOperand1 = (operand1 >> (i * 8)) & 0xFF;
        u8 byteOperand2 = (operand2 >> (i * 8)) & 0xFF;
        u8 byteResult = (result >> (i * 8)) & 0xFF;

        // Check if there is a carry between the lower and upper nibbles
        if (((byteOperand1 & 0x0F) + (byteOperand2 & 0x0F)) > 0x0F ||
            ((byteOperand1 & 0xF0) + (byteOperand2 & 0xF0)) > 0xF0) {
            flags.AF = 1;
            break;
        }
    }

    return result;
}

u64 XorAndSetFlags(u64 dst, u64 src, RflagsRegister& flags) {
    auto result = dst ^ src;
    SetLogicOpFlags(flags, result);
    return result;
}

u64 AndAndSetFlags(u64 dst, u64 src, RflagsRegister& flags) {
    auto result = dst & src;
    SetLogicOpFlags(flags, result);
    return result;
}

u64 OrAndSetFlags(u64 dst, u64 src, RflagsRegister& flags) {
    auto result = dst | src;
    SetLogicOpFlags(flags, result);
    return result;
}

void SetLogicOpFlags(RflagsRegister& flags, u64 value) {
    flags.ZF = (value == 0) ? 1 : 0;
    flags.SF = (value & 0x8000000000000000) ? 1 : 0;

    uint64_t ones = std::popcount(value);
    flags.PF = (ones % 2 == 0) ? 1 : 0;

    // Other flags can be set based on specific conditions.
    // Set Carry Flag (CF) based on some condition (example)
    flags.CF = 0; // Typically set based on arithmetic operations
    flags.OF = 0; // Overflow Flag, set based on arithmetic operations
}

//Cmp one, two
u64 CmpAndSetFlags(uint64_t src1, uint64_t src2, RflagsRegister& flags) {
    // Perform the subtraction
    uint64_t result = src1 - src2;

    // Set the Carry Flag (CF)
    flags.CF = (src1 < src2) ? 1 : 0;

    // Set the Overflow Flag (OF)
    bool signMinuend = (src1 & 0x8000000000000000) != 0;
    bool signSubtrahend = (src2 & 0x8000000000000000) != 0;
    bool signResult = (result & 0x8000000000000000) != 0;
    flags.OF = (signMinuend != signSubtrahend && signResult != signMinuend) ? 1 : 0;

    // Set the Zero Flag (ZF)
    flags.ZF = (result == 0) ? 1 : 0;

    // Set the Sign Flag (SF)
    flags.SF = (result & 0x8000000000000000) ? 1 : 0;

    // Set the Parity Flag (PF)
    uint64_t ones = std::popcount(result);
    flags.PF = (ones % 2 == 0) ? 1 : 0; // Even parity

    flags.AF = 0;
    for (int i = 0; i < 8; ++i) {
        uint8_t byteMinuend = (src1 >> (i * 8)) & 0xFF;
        uint8_t byteSubtrahend = (src2 >> (i * 8)) & 0xFF;
        uint8_t byteResult = (result >> (i * 8)) & 0xFF;

        if (((byteMinuend & 0x0F) < (byteSubtrahend & 0x0F)) ||
            ((byteMinuend & 0xF0) < (byteSubtrahend & 0xF0))) {
            flags.AF = 1;
            break;
        }
    }

    return src1;//return original, not setting the register
}

u64 TestAndSetFlags(u64 src1, u64 src2, RflagsRegister& flags) {
    u64 result = src1 & src2;
    SetLogicOpFlags(flags, result);
    return src1; //return original, not changine the register
}

u64 dec_and_set_flags(u64 src1, RflagsRegister& flags){
    // Perform the decrement
    uint64_t result = src1 - 1;

    // Set the Overflow Flag (OF)
    bool signValue = (src1 & 0x8000000000000000) != 0;
    bool signResult = (result & 0x8000000000000000) != 0;
    flags.OF = (signValue && !signResult) ? 1 : 0;

    // Set the Zero Flag (ZF)
    flags.ZF = (result == 0) ? 1 : 0;
    // Set the Sign Flag (SF)
    flags.SF = (result & 0x8000000000000000) ? 1 : 0;
    // Set the Parity Flag (PF)
    uint64_t ones = std::popcount(result);
    flags.PF = (ones % 2 == 0) ? 1 : 0; // Even parity

    // Set the Auxiliary Carry Flag (AF)
    // AF is set if there is a borrow from the lower nibble to the upper nibble
    uint8_t lowerNibbleBefore = src1 & 0x0F;
    uint8_t lowerNibbleAfter = result & 0x0F;
    flags.AF = (lowerNibbleBefore < lowerNibbleAfter) ? 1 : 0;

    return result;
}

u64 Lea(u64 dst, u64 src) {
    std::cout << "[LEA] loading address: 0x" << std::hex << src << "\n";
    return src;
}