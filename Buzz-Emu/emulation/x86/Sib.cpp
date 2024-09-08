
#include <iostream>

#include "Sib.hpp"
#include "ModRM.hpp"

constexpr std::array<uint8_t, 4> scale_table = { 1, 2, 4, 8 };

#include <iostream> // For std::cout

void HandleSib(Emulator& emu, x86Dcctx* ctx, ModRM& modrm, Sib& sib, u64& clac_offset) {
    auto sib_byte = ctx->sib;

    // Log the SIB byte value
    std::cout << "SIB Byte: " << std::hex << static_cast<int>(sib_byte) << std::dec << std::endl;

    // Extract scale from the SIB byte
    std::cout << "scale bits: " << SIB_SCALE(sib_byte) << "\n";
    sib.scale = scale_table[SIB_SCALE(sib_byte)];
    std::cout << "Scale: " << static_cast<int>(sib.scale) << std::endl;

    // Check and handle index and base register
    if (SIB_INDEX(sib_byte) == 0b100) {
        // Index register cannot be ESP (RSP) when used with a SIB byte
        sib.use_indx = false;
        std::cout << "No index register used" << std::endl;
    } else {
        sib.index_reg = static_cast<Register>(SIB_INDEX(sib_byte));
        sib.use_indx = true;
        std::cout << "Index Register: " << static_cast<int>(sib.index_reg) << std::endl;
    }

    if (SIB_BASE(sib_byte) == 0b101) {
        // Special case: BASE = 101 means no base register, use displacement
        if (modrm.RM_Mod.disp == 0) {
            // If no displacement is present, use a 32-bit displacement
            clac_offset = sib.scale * emu.Reg(sib.index_reg);
            modrm.RM_Mod.disp = 32;
            std::cout << "Offset calculation (base = 101, no displacement): " << clac_offset << std::endl;
            std::cout << "Using 32-bit displacement" << std::endl;
            sib.valid = true;
            return;
        }
    } else {
        // Handle base register (excluding the special case above)
        if (SIB_BASE(sib_byte) == 0b000) {
            sib.use_base = false;
            std::cout << "No base register used" << std::endl;
        } else {
            sib.base_reg = static_cast<Register>(SIB_BASE(sib_byte));
            sib.use_base = true;
            std::cout << "Base Register: " << static_cast<int>(sib.base_reg) << std::endl;
        }

        // Calculate offset with base register
        clac_offset = (sib.scale * (sib.use_indx ? emu.Reg(sib.index_reg) : 0)) + (sib.use_base ? emu.Reg(sib.base_reg) : 0);
        std::cout << "Offset calculation: " << clac_offset << std::endl;
        std::cout << "sib stynax: [r" << (int)sib.base_reg << " + disp" <<  (int)modrm.RM_Mod.disp << "]\n";
    }

    // Final log for SIB validity
    sib.valid = sib.use_indx || sib.use_base;
    std::cout << "SIB valid: " << (sib.valid ? "true" : "false") << std::endl;
}
