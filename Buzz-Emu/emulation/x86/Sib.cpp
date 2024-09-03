
#include "Sib.hpp"
#include "ModRM.hpp"

constexpr std::array<u8, 4> scale_table = { 1, 2, 4, 8 };

void HandleSib(Emulator& emu, x86Dcctx* ctx, ModRM& modrm, Sib& sib, u64& clac_offset) {
	u8 sib_byte = ctx->sib;
	sib.scale = scale_table[SIB_SCALE(sib_byte)];

	if (SIB_INDEX(sib_byte) == 0b100 || SIB_BASE(sib_byte) == 0b101) { //No rsp and rbp 
		sib.valid = false;
		return;
	}

	sib.valid = true;
	if (SIB_INDEX(sib_byte) == 0b000)
		sib.use_indx = false;
	else
		sib.index_reg = static_cast<Register>(SIB_INDEX(sib_byte));


	if (SIB_BASE(sib_byte) == 0b000)
		sib.use_base = false;
	else
		sib.base_reg = static_cast<Register>(SIB_BASE(sib_byte));

	if (SIB_BASE(sib_byte) == 0b101 && !modrm.RM_Mod.disp) {
		clac_offset = (sib.scale * emu.Reg(sib.index_reg));
		modrm.RM_Mod.disp = 32;
		return;
	}

	clac_offset = (sib.scale * emu.Reg(sib.index_reg)) + emu.Reg(sib.base_reg);
}