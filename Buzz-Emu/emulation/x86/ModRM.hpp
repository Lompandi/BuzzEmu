#pragma once

#include <array>
#include <vector>
#include <optional>
#include <type_traits>

#include "Register.hpp"
#include "AddressingMode.hpp"
#include "../../core/Memtypes.hpp"
#include "../../emulator/Emulator.hpp"


using RmModArray = std::array<std::array<std::pair<std::optional<Register>, u8>, 4>, 8>;

struct RegisterTraits {
	bool		 RMRegSet : 1;
	bool		 IsPtr : 1;
	u8			 disp;
	ByteRegister h_l;
	Register	 reg; //here
	u64			 reg_val;
};

struct ModRM {
	/*Register and its value*/
	RegisterState	Reg; 
	/*the bool is detrmin if the value is an ptr or not*/
	RegisterTraits	RM_Mod; //Register and displacement
};

extern const RmModArray rm_mod_mapping;

void Clear_ModRM(ModRM* modrm);
void Handle_ModRM(Emulator& emu, x86Dcctx* ctx, ModRM& modrm);

//helper function to get the 8 bit r/m (r/m8)
void redef_modrm_rm8(Emulator& emu, ModRM& modrm);
void redef_modrm_reg8(Emulator& emu, ModRM& modrm);