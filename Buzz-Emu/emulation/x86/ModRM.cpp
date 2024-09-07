
#include "ModRM.hpp"

constexpr std::array<std::pair<std::optional<Register>, u8>, 4> MakeModArray(Register reg) {
	return { {
		{reg, 0},
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
	{ {
		{std::nullopt, 32},
		{Register::Rbp, 8},
		{Register::Rbp, 32},
		{Register::Rbp, 0}
	} },
	//R/M = 110
	MakeModArray(Register::Rsi),
	//R/M = 111
	MakeModArray(Register::Rdi)
} ;


/* process the modrm data, the return value will be is the modrm contains sib byte*/
void Handle_ModRM(Emulator& emu, x86Dcctx* ctx, ModRM& modrm) {
	//we will assume it alreaft have a modrm byte
	if (!ctx->p_modrm)
		return;

	modrm.Reg.reg = static_cast<Register>(MODRM_REG(ctx->modrm));
	modrm.Reg.val = emu.Reg(modrm.Reg.reg);

	const auto& val = rm_mod_mapping[MODRM_RM(ctx->modrm)][MODRM_MOD(ctx->modrm)];

	modrm.RM_Mod.IsPtr = MODRM_MOD(ctx->modrm) != 0b11 ? true : false;

	if (val.first) {
		modrm.RM_Mod.reg = val.first.value(); //Register
		modrm.RM_Mod.reg_val = emu.Reg(val.first.value());
		modrm.RM_Mod.RMRegSet = true;
	}
	if (val.second)
		modrm.RM_Mod.disp = val.second;		//Displacement
}