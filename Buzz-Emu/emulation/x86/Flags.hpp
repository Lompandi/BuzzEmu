#pragma once

#include "../../core/Memtypes.hpp"

struct FlagsRegister32 {
    u32 CF : 1; // Carry Flag
    u32 Reserved1 : 1; // Reserved
    u32 PF : 1; // Parity Flag
    u32 Reserved2 : 1; // Reserved
    u32 AF : 1; // Auxiliary Carry Flag
    u32 Reserved3 : 1; // Reserved
    u32 ZF : 1; // Zero Flag
    u32 SF : 1; // Sign Flag
    u32 Reserved4 : 1; // Reserved
    u32 TF : 1; // Trap Flag
    u32 IF : 1; // Interrupt Enable Flag
    u32 DF : 1; // Direction Flag
    u32 OF : 1; // Overflow Flag
    u32 IOPL : 2; // I/O Privilege Level
    u32 NT : 1; // Nested Task
    u32 Reserved5 : 1; // Reserved
    u32 RF : 1; // Resume Flag
    u32 VM : 1; // Virtual 8086 Mode
    u32 AC : 1; // Alignment Check
    u32 VIF : 1; // Virtual Interrupt Flag
    u32 VIP : 1; // Virtual Interrupt Pending
    u32 ID : 1; // Identification Flag
    u32 Reserved6 : 10; // Reserved
};

struct FlagsRegister64 {
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

    FlagsRegister64()
        : CF(0), Reserved1(0), PF(0), Reserved2(0),
        AF(0), Reserved3(0), ZF(0), SF(0), Reserved4(0),
        TF(0), IF(0), DF(0), OF(0), IOPL(0), NT(0),
        Reserved5(0), RF(0), VM(0), AC(0), VIF(0),
        VIP(0), ID(0), Reserved6(0) {}
};

//we need it!
unsigned long long popcountll(unsigned long long x);


u64 SubAndSetFlags(FlagsRegister64& flags, uint64_t minuend, uint64_t subtrahend);
u64 AddAndSetFlags(FlagsRegister64& flags, u64 operand1, u64 operand2);

void SetLogicOpFlags(FlagsRegister64& flags, u64 value);