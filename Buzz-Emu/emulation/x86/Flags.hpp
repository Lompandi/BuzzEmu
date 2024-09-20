#pragma once

#include <vector>

#include "../../core/Memtypes.hpp"

struct RflagsRegister {
    u64 CF : 1; // Carry Flag
    u64 Reserved1 : 1; // Reserved
    u64 PF : 1; // Parity Flag
    u64 Reserved2 : 1; // Reserved
    u64 AF : 1; // Auxiliary Carry Flag
    u64 Reserved3 : 1; // Reserved
    u64 ZF : 1; // Zero Flag
    u64 SF : 1; // Sign Flag
    u64 Reserved4 : 1; // Reserved
    u64 TF : 1; // Trap Flag
    u64 IF : 1; // Interrupt Enable Flag
    u64 DF : 1; // Direction Flag
    u64 OF : 1; // Overflow Flag
    u64 IOPL : 2; // I/O Privilege Level
    u64 NT : 1; // Nested Task
    u64 Reserved5 : 1; // Reserved
    u64 RF : 1; // Resume Flag
    u64 VM : 1; // Virtual 8086 Mode
    u64 AC : 1; // Alignment Check
    u64 VIF : 1; // Virtual Interrupt Flag
    u64 VIP : 1; // Virtual Interrupt Pending
    u64 ID : 1; // Identification Flag
    u64 Reserved6 : 43; // Reserved (for 64-bit mode)

    RflagsRegister()
        : CF(0), Reserved1(0), PF(0), Reserved2(0),
        AF(0), Reserved3(0), ZF(0), SF(0), Reserved4(0),
        TF(0), IF(0), DF(0), OF(0), IOPL(0), NT(0),
        Reserved5(0), RF(0), VM(0), AC(0), VIF(0),
        VIP(0), ID(0), Reserved6(0) {}
};

u64 XorAndSetFlags(u64 dst, u64 src, RflagsRegister& flags);
u64 AndAndSetFlags(u64 dst, u64 src, RflagsRegister& flags);
u64 OrAndSetFlags(u64 dst, u64 src, RflagsRegister& flags);

//will be segment register
u64 mov_operation(u64 dst, u64 src);
//u64 SubAndSetFlags(RflagsRegister& flags, uint64_t minuend, uint64_t subtrahend);
u64 SubAndSetFlags(uint64_t minuend, uint64_t subtrahend, RflagsRegister& flags);

u64 AddAndSetFlags(u64 operand1, u64 operand2, RflagsRegister& flags);

u64 CmpAndSetFlags(uint64_t src1, uint64_t src2, RflagsRegister& flags);

u64 TestAndSetFlags(u64 src1, u64 src2, RflagsRegister& flags);

u64 dec_and_set_flags(u64 src1, RflagsRegister& flags);

void SetLogicOpFlags(RflagsRegister& flags, u64 value);

u64 Lea(u64 dst, u64 src, size_t inst_size, u64& pc, s64 disp);