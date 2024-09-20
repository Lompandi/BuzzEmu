#pragma once

#include "ModRM.hpp"
#include "Register.hpp"
#include "DecodeContext.hpp"
#include "../../emulator/Emulator.hpp"

struct Sib {
	u8			  scale;
	bool		  valid;
	bool		  use_indx;
	bool	      use_base;
	Register	  index_reg;
	Register      base_reg;
};

void set_sib_byte(Emulator& emu, x86Dcctx* ctx, ModRM& modrm, Sib& sib, u64& calc_offset);