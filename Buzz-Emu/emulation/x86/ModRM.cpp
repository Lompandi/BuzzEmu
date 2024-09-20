
#include <iostream>
#include <type_traits>

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


void reset_modrm(ModRM* modrm) {
	modrm->Reg.reg = Register::Rax;
	modrm->Reg.val = 0;
	modrm->rm.disp_size = 0;
	modrm->rm.is_addr = false;
	modrm->rm.reg_val = 0;
	modrm->rm.reg_set = false;
	modrm->Reg.h_l = ByteRegister::LowByte;
	modrm->rm.h_l = ByteRegister::LowByte;
}

#define DEBUG_LOG(msg, ...) \
    do { \
        std::cout << "DEBUG: " << __FUNCTION__ << ": " << msg << "\n"; \
    } while (0)

void set_modrm_byte(Emulator& emu, x86Dcctx* ctx, ModRM& modrm, ModRegRMType hndl_type) {
    reset_modrm(&modrm);

	if (hndl_type & ModRegRMType::Reg_8bit) {
		redef_modrm_reg8(emu, modrm);
		DEBUG_LOG("using 8-bit register\n");
	}
	if (hndl_type & ModRegRMType::RM_8bit) {
		redef_modrm_rm8(emu, modrm);
		DEBUG_LOG("using 8-bit register/memory\n");
	}

    DEBUG_LOG("Processing ModRM data");

    if (!ctx->p_modrm) {
        DEBUG_LOG("No ModRM data to process (p_modrm = false)");
        return;
    }

    std::cout << "opsize: " << (u8)ctx->osize << "\n";

    // Extract and log the register part from ModRM byte
    modrm.Reg.reg = static_cast<Register>(MODRM_REG(ctx->modrm));
    if (_REX_R(ctx->pfx_rex)) {
        /*REX.R will extend the reg field in ModR/M*/
        modrm.Reg.reg = static_cast<Register>(std::to_underlying(modrm.Reg.reg) + 8);
    }

    modrm.Reg.val = emu.Reg(modrm.Reg.reg);
    DEBUG_LOG("Register extracted: reg = " << static_cast<int>(modrm.Reg.reg) << ", val = " << std::hex << modrm.Reg.val);

    // Determine if there is a SIB byte and log the result
    const auto& val = rm_mod_mapping[MODRM_RM(ctx->modrm)][MODRM_MOD(ctx->modrm)];
    modrm.rm.is_addr = MODRM_MOD(ctx->modrm) != 0b11;
    DEBUG_LOG("is_addr: " << std::boolalpha << modrm.rm.is_addr);

    // Handle the register value if present and log
    if (val.first) {
        modrm.rm.reg = val.first.value(); // Register
        if (_REX_B(ctx->pfx_rex)) {
            /*REX.B field will extends the RM field in ModR/M*/
            modrm.rm.reg = static_cast<Register>(std::to_underlying(modrm.rm.reg) + 8);
        }

        modrm.rm.reg_val = emu.Reg(modrm.rm.reg);
        modrm.rm.reg_set = true;
        DEBUG_LOG("rm register: reg = " << static_cast<int>(modrm.rm.reg) << ", reg_val = " << std::hex << modrm.rm.reg_val);
    }
    else {
        DEBUG_LOG("No register value found in ModRM");
    }

    // Handle the displacement if present and log
    if (val.second) {
        modrm.rm.disp_size = val.second; // Displacement
        DEBUG_LOG("Displacement: " << std::hex << (int)modrm.rm.disp_size);
    }
    else {
        DEBUG_LOG("No displacement value found in ModRM");
    }
}

//helper function to get the 8 bit r/m (r/m8)
//tables for resetting 
constexpr Register redef_8bit_regs[8] = {
		Register::Rax,
		Register::Rcx,
		Register::Rdx,
		Register::Rbx,
		Register::Rax,
		Register::Rcx,
		Register::Rdx,
		Register::Rbx
};

constexpr u64 redef_8bit_mask[8] = {
	0x00000000000000FF,
	0x00000000000000FF,
	0x00000000000000FF,
	0x00000000000000FF,
	0x000000000000FF00,
	0x000000000000FF00,
	0x000000000000FF00,
	0x000000000000FF00,
};

constexpr ByteRegister redef_8bit_setreg_mask[8] = {
	ByteRegister::LowByte,
	ByteRegister::LowByte,
	ByteRegister::LowByte,
	ByteRegister::LowByte,
	ByteRegister::HighByte,
	ByteRegister::HighByte,
	ByteRegister::HighByte,
	ByteRegister::HighByte,
};

constexpr u8 shifts_table[8] = {
	0, 0, 0, 0,
	8, 8, 8, 8,
};

void redef_modrm_rm8(Emulator& emu, ModRM& modrm) {
	auto mask = redef_8bit_mask[modrm.rm.reg];
	auto state_before = modrm.rm.reg;
	modrm.rm.h_l = redef_8bit_setreg_mask[modrm.rm.reg];
	modrm.rm.reg = redef_8bit_regs[std::to_underlying(modrm.rm.reg)];
	modrm.rm.reg_val =
		(emu.Reg(modrm.rm.reg) & mask) >> shifts_table[state_before];
}

void redef_modrm_reg8(Emulator& emu, ModRM& modrm) {
	auto mask = redef_8bit_mask[modrm.Reg.reg];
	auto state_before = modrm.Reg.reg;
	modrm.Reg.h_l = redef_8bit_setreg_mask[modrm.Reg.reg];
	/*swaps the registers into 8-bit mode*/
	modrm.Reg.reg = redef_8bit_regs[std::to_underlying(modrm.Reg.reg)];
	modrm.Reg.val =
		(emu.Reg(modrm.Reg.reg) & mask) >> shifts_table[state_before];
}