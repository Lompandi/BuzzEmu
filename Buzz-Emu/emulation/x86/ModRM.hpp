#pragma once

#include <array>
#include <vector>
#include <optional>
#include <type_traits>

#include "Register.hpp"
#include "../../core/Memtypes.hpp"
#include "../../emulator/Emulator.hpp"


using RmModArray = std::array<std::array<std::pair<std::optional<Register>, u8>, 4>, 8>;

struct RegisterTraits {
	bool		 reg_set : 1;
	bool		 is_addr : 1;
	u8			 disp_size;
	ByteRegister h_l;
	Register	 reg; //here
	u64			 reg_val;
};

struct ModRM {
	/*Register and its value*/
	RegisterState	Reg; 
	/*the bool is detrmin if the value is an ptr or not*/
	RegisterTraits	rm; //Register and displacement
};

enum ModRegRMType : u8{
	Normal = 0,
	RM_8bit = 0x0F,
	Reg_8bit = 0xF0,
	RegRM_8bit = 0xFF,
};

extern const RmModArray rm_mod_mapping;

void reset_modrm(ModRM* modrm);
void set_modrm_byte(Emulator& emu, x86Dcctx* ctx, ModRM& modrm, ModRegRMType hndl_type = ModRegRMType::Normal);

//helper function to get the 8 bit r/m (r/m8)
void redef_modrm_rm8(Emulator& emu, ModRM& modrm);
void redef_modrm_reg8(Emulator& emu, ModRM& modrm);