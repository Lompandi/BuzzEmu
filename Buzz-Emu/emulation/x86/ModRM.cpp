
#include <iostream>

#include "ModRM.hpp"

constexpr std::array<std::pair<std::optional<Register>, u8>, 4> MakeModArray(Register reg) {
	return { {
		{reg, 0},
		{reg, 8},
		{reg, 32},
		{reg, 0}
	} };
}

constexpr std::array<std::pair<std::optional<Register>, u8>, 4> MakeRm101Array(Register reg) {
	return { {
		{std::nullopt, 32},
		{reg, 8},
		{reg, 32},
		{reg, 0}
	} };
}

const RmModArray rm_mod_mapping = {
	//R/M = 000
	MakeModArray(Register::Rax),
	//R/M = 001
	MakeModArray(Register::Rcx),
	//R/M = 010
	MakeModArray(Register::Rdx),
	//R/M = 011
	MakeModArray(Register::Rbx),
	//TODO R/M = 100
	MakeModArray(Register::Rsp),
	//R/M = 101
	MakeRm101Array(Register::Rbp),
	//R/M = 110
	MakeModArray(Register::Rsi),
	//R/M = 111
	MakeModArray(Register::Rdi)
} ;


void Clear_ModRM(ModRM* modrm) {
	modrm->Reg.reg = Register::Rax;
	modrm->Reg.val = 0;
	modrm->RM_Mod.disp = 0;
	modrm->RM_Mod.IsPtr = false;
	modrm->RM_Mod.reg_val = 0;
	modrm->RM_Mod.RMRegSet = false;
}

#define DEBUG_LOG(msg, ...) \
    do { \
        std::cout << "DEBUG: " << __FUNCTION__ << ": " << msg << "\n"; \
    } while (0)

void Handle_ModRM(Emulator& emu, x86Dcctx* ctx, ModRM& modrm) {
    // Assume modrm already has a modrm byte
    Clear_ModRM(&modrm);

    DEBUG_LOG("Processing ModRM data");

    if (!ctx->p_modrm) {
        DEBUG_LOG("No ModRM data to process (p_modrm = false)");
        return;
    }

    std::cout << "opsize: " << (u8)ctx->osize << "\n";

    // Extract and log the register part from ModRM byte
    modrm.Reg.reg = static_cast<Register>(MODRM_REG(ctx->modrm));
    modrm.Reg.val = emu.Reg(modrm.Reg.reg);
    DEBUG_LOG("Register extracted: reg = " << static_cast<int>(modrm.Reg.reg) << ", val = " << std::hex << modrm.Reg.val);

    // Determine if there is a SIB byte and log the result
    const auto& val = rm_mod_mapping[MODRM_RM(ctx->modrm)][MODRM_MOD(ctx->modrm)];
    modrm.RM_Mod.IsPtr = MODRM_MOD(ctx->modrm) != 0b11;
    DEBUG_LOG("IsPtr: " << std::boolalpha << modrm.RM_Mod.IsPtr);

    // Handle the register value if present and log
    if (val.first) {
        modrm.RM_Mod.reg = val.first.value(); // Register
        modrm.RM_Mod.reg_val = emu.Reg(val.first.value());
        modrm.RM_Mod.RMRegSet = true;
        DEBUG_LOG("RM_Mod register: reg = " << static_cast<int>(modrm.RM_Mod.reg) << ", reg_val = " << std::hex << modrm.RM_Mod.reg_val);
    }
    else {
        DEBUG_LOG("No register value found in ModRM");
    }

    // Handle the displacement if present and log
    if (val.second) {
        modrm.RM_Mod.disp = val.second; // Displacement
        DEBUG_LOG("Displacement: " << std::hex << (int)modrm.RM_Mod.disp);
    }
    else {
        DEBUG_LOG("No displacement value found in ModRM");
    }
}