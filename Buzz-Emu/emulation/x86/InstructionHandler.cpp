
#include "Sib.hpp"
#include "ModRM.hpp"

#include <iostream>
#include "../../core/Fs.hpp"
#include "InstructionHandler.hpp"

#pragma region Add (0x01)
void Add_01(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst) {
	OperandSize opsize = ctx->osize;

	ModRM mod_rm;
	Handle_ModRM(emu, ctx, mod_rm);

	if (!mod_rm.RM_Mod.disp && !mod_rm.RM_Mod.RMRegSet)
		return;

	//=====================No SIB===============================
	//from Reg to RM -> ADD R/M, REG
	if (!ctx->p_sib) {
		if (!mod_rm.RM_Mod.IsPtr) { //store to register
			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.SetReg(mod_rm.RM_Mod.reg,
					GET_X_REG(mod_rm.RM_Mod.reg_val) +
					GET_X_REG(mod_rm.Reg.val));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.SetReg(mod_rm.RM_Mod.reg,
					GET_EXT_REG(mod_rm.RM_Mod.reg_val) +
					GET_EXT_REG(mod_rm.Reg.val));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {
				emu.SetReg(mod_rm.RM_Mod.reg,
					mod_rm.RM_Mod.reg_val +
					mod_rm.Reg.val);
			}
		}
		else if (!mod_rm.RM_Mod.disp) { //store to memory, no displacement
			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val),
					ToByteVector(emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value() +
						GET_X_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(GET_EXT_REG(mod_rm.RM_Mod.reg_val),
					ToByteVector(emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value() +
						GET_EXT_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {
				emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val,
					ToByteVector(emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value() +
						mod_rm.Reg.val));
			}
		}
		else if (mod_rm.RM_Mod.reg && mod_rm.RM_Mod.disp) { //store to memory, displacement
			s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 2).value();

			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp,
					ToByteVector(emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value() +
						GET_X_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp,
					ToByteVector(emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value() +
						GET_EXT_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {
				emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val + disp,
					ToByteVector(emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value() +
						mod_rm.Reg.val));
			}
		}
		 //store to displacement
	}
	//=================================SIB==============================
	else {
		Sib sib_byte;
		u64 calc_offset = 0;

		if (mod_rm.RM_Mod.disp) { //SIB + displacement
			s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 3).value();
			if (opsize == OperandSize::X86_Osize_16bit) {
				HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);
				if (!sib_byte.valid)
					return;
				emu.memory.WriteFrom(calc_offset + disp,
					ToByteVector(emu.memory.Read<u16>(calc_offset + disp).value() +
						GET_X_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);
				if (!sib_byte.valid)
					return;
				emu.memory.WriteFrom(calc_offset + disp,
					ToByteVector(emu.memory.Read<u32>(calc_offset + disp).value() +
						GET_EXT_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {
				HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);
				if (!sib_byte.valid)
					return;
				emu.memory.WriteFrom(calc_offset + disp,
					ToByteVector(emu.memory.Read<u64>(calc_offset + disp).value() +
						mod_rm.Reg.val));
			}
		}
		else { //direct sib addressing
			if (opsize == OperandSize::X86_Osize_16bit) {
				HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);
				if (!sib_byte.valid)
					return;
				emu.memory.WriteFrom(calc_offset,
					ToByteVector(emu.memory.Read<u16>(calc_offset).value() +
						GET_X_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);
				if (!sib_byte.valid)
					return;
				emu.memory.WriteFrom(calc_offset,
					ToByteVector(emu.memory.Read<u32>(calc_offset).value() +
						GET_EXT_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {
				HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);
				if (!sib_byte.valid)
					return;
				emu.memory.WriteFrom(calc_offset,
					ToByteVector(emu.memory.Read<u64>(calc_offset).value() +
						mod_rm.Reg.val));
			}
		}
	}
	return;
}

#pragma endregion
#pragma region Add (0x03)
INSTRUCTION_OP2_RM(Add, 03,
	/*store to register from register*/
	AddAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		GET_X_REG(mod_rm.RM_Mod.reg_val)),
	AddAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		GET_EXT_REG(mod_rm.RM_Mod.reg_val)),
	AddAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		mod_rm.RM_Mod.reg_val),
	/*store to register from memory, no displacement*/
	AddAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value()),
	AddAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value()),
	AddAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value()),
	/*store to register from memory, displacement*/
	AddAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value()),
	AddAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value()),
	AddAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value()),
	/*SIB byte extension, displacement*/
	AddAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(calc_offset + disp).value()),
	AddAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(calc_offset + disp).value()),
	AddAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(calc_offset + disp).value()),
	/*SIB byte extension, no displacement*/
	AddAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(calc_offset).value()),
	AddAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(calc_offset).value()),
	AddAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(calc_offset).value()))
#pragma endregion
#pragma region Add (0x81)
INSTRUCTION_OP2_MI(Add, 81, 
	AddAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.RM_Mod.reg_val),
		ReadFromVec<u16>(inst, 2)),
	AddAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.RM_Mod.reg_val),
		ReadFromVec<u32>(inst, 2)),
	AddAndSetFlags(emu.flags,
		mod_rm.RM_Mod.reg_val,
		static_cast<u64>(ReadFromVec<u32>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value(),
		ReadFromVec<u16>(inst, 2)),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value(),
		ReadFromVec<u32>(inst, 2)),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value(),
		static_cast<u64>(ReadFromVec<u32>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
		ReadFromVec<u16>(inst, 2)),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
		ReadFromVec<u32>(inst, 2)),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value(),
		static_cast<u64>(ReadFromVec<u32>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(calc_offset + disp).value(),
		ReadFromVec<u16>(inst, 2)),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(calc_offset + disp).value(),
		ReadFromVec<u32>(inst, 2)),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(calc_offset + disp).value(),
		static_cast<u64>(ReadFromVec<u32>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(calc_offset).value(),
		ReadFromVec<u16>(inst, 2)),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(calc_offset).value(),
		ReadFromVec<u32>(inst, 2)), 
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(calc_offset).value(),
		static_cast<u64>(ReadFromVec<u32>(inst, 2))))
#pragma endregion
#pragma region Add (0x83)
INSTRUCTION_OP2_MI(Add, 83, 
	AddAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.RM_Mod.reg_val),
		static_cast<u16>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.RM_Mod.reg_val),
		static_cast<u32>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		mod_rm.RM_Mod.reg_val,
		static_cast<u64>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value(),
		static_cast<u16>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value(),
		static_cast<u32>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value(),
		static_cast<u64>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
		static_cast<u16>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
		static_cast<u32>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value(),
		static_cast<u64>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(calc_offset + disp).value(),
		static_cast<u16>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(calc_offset + disp).value(),
		static_cast<u32>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(calc_offset + disp).value(),
		static_cast<u64>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(calc_offset).value(),
		static_cast<u16>(ReadFromVec<u8>(inst, 2))),
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(calc_offset).value(),
		static_cast<u32>(ReadFromVec<u8>(inst, 2))), 
	AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(calc_offset).value(),
		static_cast<u64>(ReadFromVec<u8>(inst, 2))))
#pragma endregion

#pragma region Or (0x09)
INSTRUCTION_LOGICAL_OP2_RM_REG(Or, 09, | )
#pragma endregion
#pragma region Or (0x0D)
INSTRUCTION_LOGICAL_AX_IMM(Or, 0D, | )
#pragma endregion

#pragma region And (0x21)
INSTRUCTION_LOGICAL_OP2_RM_REG(And, 21, &)
#pragma endregion
#pragma region And (0x25)
INSTRUCTION_LOGICAL_AX_IMM(And, 25, &)
#pragma endregion

#pragma region Sub (0x29)
INSTRUCTION_OP2_MR(Sub, 29,
	SubAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.RM_Mod.reg_val),
		GET_X_REG(mod_rm.Reg.val)),
	SubAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.RM_Mod.reg_val),
		GET_EXT_REG(mod_rm.Reg.val)),
	SubAndSetFlags(emu.flags,
		mod_rm.RM_Mod.reg_val,
		mod_rm.Reg.val),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value(),
		GET_X_REG(mod_rm.Reg.val)),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value(),
		GET_EXT_REG(mod_rm.Reg.val)),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value(),
		mod_rm.Reg.val),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
		GET_X_REG(mod_rm.Reg.val)),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
		GET_EXT_REG(mod_rm.Reg.val)),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value(),
		mod_rm.Reg.val),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u16>(calc_offset + disp).value(),
		GET_X_REG(mod_rm.Reg.val)),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u32>(calc_offset + disp).value(),
		GET_EXT_REG(mod_rm.Reg.val)),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u64>(calc_offset + disp).value(),
		mod_rm.Reg.val),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u16>(calc_offset).value(),
		GET_X_REG(mod_rm.Reg.val)),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u32>(calc_offset).value(),
		GET_EXT_REG(mod_rm.Reg.val)), 
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u64>(calc_offset).value(),
		mod_rm.Reg.val))

#pragma endregion
#pragma region Sub (0x2B)
INSTRUCTION_OP2_RM(Sub, 2B,
	/*store to register from register*/
	SubAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		GET_X_REG(mod_rm.RM_Mod.reg_val)),
	SubAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		GET_EXT_REG(mod_rm.RM_Mod.reg_val)),
	SubAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		mod_rm.RM_Mod.reg_val),
	/*store to register from memory, no displacement*/
	SubAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val), 
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value()) ,
	SubAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value()) ,
	SubAndSetFlags(emu.flags,
		mod_rm.Reg.val, 
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value()) ,
	/*store to register from memory, displacement*/
	SubAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value()) ,
	SubAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value()) ,
	SubAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value()) ,
	/*SIB byte extension, displacement*/
	SubAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(calc_offset + disp).value()),
	SubAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(calc_offset + disp).value()) ,
	SubAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(calc_offset + disp).value()),
	/*SIB byte extension, no displacement*/
	SubAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(calc_offset).value()),
	SubAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(calc_offset).value()),
	SubAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(calc_offset).value()))
#pragma endregion
#pragma region Sub (0x83)
INSTRUCTION_OP2_MI(Sub, 83,
	SubAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.RM_Mod.reg_val),
		static_cast<u16>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.RM_Mod.reg_val),
		static_cast<u32>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		mod_rm.RM_Mod.reg_val,
		static_cast<u64>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value(),
		static_cast<u16>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value(),
		static_cast<u32>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value(),
		static_cast<u64>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
		static_cast<u16>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
		static_cast<u32>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value(),
		static_cast<u64>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u16>(calc_offset + disp).value(),
		static_cast<u16>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u32>(calc_offset + disp).value(),
		static_cast<u32>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u64>(calc_offset + disp).value(),
		static_cast<u64>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u16>(calc_offset).value(),
		static_cast<u16>(ReadFromVec<u8>(inst, 2))),
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u32>(calc_offset).value(),
		static_cast<u32>(ReadFromVec<u8>(inst, 2))), 
	SubAndSetFlags(emu.flags,
		emu.memory.Read<u64>(calc_offset).value(),
		static_cast<u64>(ReadFromVec<u8>(inst, 2))))
#pragma endregion

#pragma region Xor (0x31)
INSTRUCTION_LOGICAL_OP2_RM_REG(Xor, 31, ^)
#pragma endregion
#pragma region Xor (0x33)
INSTRUCTION_OP2_RM(Xor, 33, 
	/*store to register from register*/
	XorAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		GET_X_REG(mod_rm.RM_Mod.reg_val)),
	XorAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		GET_EXT_REG(mod_rm.RM_Mod.reg_val)),
	XorAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		mod_rm.RM_Mod.reg_val),
	/*store to register from memory, no displacement*/
	XorAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value()),
	XorAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value()),
	XorAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value()),
	/*store to register from memory, displacement*/
	XorAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value()),
	XorAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value()),
	XorAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value()),
	/*SIB byte extension, displacement*/
	XorAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(calc_offset + disp).value()),
	XorAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(calc_offset + disp).value()),
	XorAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(calc_offset + disp).value()),
	/*SIB byte extension, no displacement*/
	XorAndSetFlags(emu.flags,
		GET_X_REG(mod_rm.Reg.val),
		emu.memory.Read<u16>(calc_offset).value()),
	XorAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.Reg.val),
		emu.memory.Read<u32>(calc_offset).value()),
	XorAndSetFlags(emu.flags,
		mod_rm.Reg.val,
		emu.memory.Read<u64>(calc_offset).value()))
#pragma endregion
#pragma region Xor (0x35)
INSTRUCTION_LOGICAL_AX_IMM(Xor, 35, ^)
#pragma endregion

#pragma region Mov (0x89)
void Mov_89(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst) {
	OperandSize opsize = ctx->osize; ModRM mod_rm; Handle_ModRM(emu, ctx, mod_rm); if (!mod_rm.RM_Mod.disp && !mod_rm.RM_Mod.RMRegSet) return; if (!ctx->p_sib) {
		if (!mod_rm.RM_Mod.IsPtr && mod_rm.RM_Mod.RMRegSet) {
			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.SetReg(mod_rm.RM_Mod.reg, static_cast<u16>(mod_rm.Reg.val & 0xFFFF));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.SetReg(mod_rm.RM_Mod.reg, static_cast<u32>(mod_rm.Reg.val & 0xFFFFFFFF));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				emu.SetReg(mod_rm.RM_Mod.reg, mod_rm.Reg.val);
			}
		}
		else if (!mod_rm.RM_Mod.disp) {
			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(static_cast<u16>(mod_rm.RM_Mod.reg_val & 0xFFFF), ToByteVector(static_cast<u16>(mod_rm.Reg.val & 0xFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(static_cast<u32>(mod_rm.RM_Mod.reg_val & 0xFFFFFFFF), ToByteVector(static_cast<u32>(mod_rm.Reg.val & 0xFFFFFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val, ToByteVector(mod_rm.Reg.val));
			}
		}
		else if (mod_rm.RM_Mod.RMRegSet && mod_rm.RM_Mod.disp) {
			s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 2).value(); if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(static_cast<u16>(mod_rm.RM_Mod.reg_val & 0xFFFF) + disp, ToByteVector(static_cast<u16>(mod_rm.Reg.val & 0xFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(static_cast<u32>(mod_rm.RM_Mod.reg_val & 0xFFFFFFFF) + disp, ToByteVector(static_cast<u32>(mod_rm.Reg.val & 0xFFFFFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				emu.memory.WriteFrom(mod_rm.RM_Mod.reg_val + disp, ToByteVector(mod_rm.Reg.val));
			}
		}
	}
	else {
		Sib sib_byte; u64 calc_offset = 0; HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset); if (!sib_byte.valid) return; std::cout << "entering sib proccessing...\n"; if (mod_rm.RM_Mod.disp) {
			s64 disp = ReadDispFromVec<s64>(inst, mod_rm.RM_Mod.disp, 3).value(); if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(calc_offset + disp, ToByteVector(static_cast<u16>(mod_rm.Reg.val & 0xFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(calc_offset + disp, ToByteVector(static_cast<u32>(mod_rm.Reg.val & 0xFFFFFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				emu.memory.WriteFrom(calc_offset + disp, ToByteVector(mod_rm.Reg.val));
			}
		}
		else {
			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(calc_offset, ToByteVector(static_cast<u16>(mod_rm.Reg.val & 0xFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(calc_offset, ToByteVector(static_cast<u32>(mod_rm.Reg.val & 0xFFFFFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				emu.memory.WriteFrom(calc_offset, ToByteVector(mod_rm.Reg.val));
			}
		}
	} return;
}
#pragma endregion
#pragma region Mov (0x8B)
INSTRUCTION_OP2_RM(Mov, 8B, 

GET_X_REG(mod_rm.RM_Mod.reg_val),
GET_EXT_REG(mod_rm.RM_Mod.reg_val),
mod_rm.RM_Mod.reg_val,

emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value(),
emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value(),
emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value(),

emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value(),

emu.memory.Read<u16>(calc_offset + disp).value(),
emu.memory.Read<u32>(calc_offset + disp).value(),
emu.memory.Read<u64>(calc_offset + disp).value(),

emu.memory.Read<u16>(calc_offset).value(),
emu.memory.Read<u32>(calc_offset).value(),
emu.memory.Read<u64>(calc_offset).value())
#pragma endregion

#pragma region Movsxd (0x63)
INSTRUCTION_OP2_RM(Movsxd, 63,
GET_X_REG(mod_rm.RM_Mod.reg_val),
GET_EXT_REG(mod_rm.RM_Mod.reg_val),
static_cast<u64>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)),

emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value(),
emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value(),
static_cast<u64>(emu.memory.Read<u32>(mod_rm.RM_Mod.reg_val).value()),

emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
static_cast<u64>(emu.memory.Read<u32>(mod_rm.RM_Mod.reg_val + disp).value()),

emu.memory.Read<u16>(calc_offset + disp).value(),
emu.memory.Read<u32>(calc_offset + disp).value(),
static_cast<u64>(emu.memory.Read<u32>(calc_offset + disp).value()),

emu.memory.Read<u16>(calc_offset).value(),
emu.memory.Read<u32>(calc_offset).value(),
static_cast<u64>(emu.memory.Read<u32>(calc_offset).value()))
#pragma endregion

#pragma region Cmp (0x83)
INSTRUCTION_OP2_MI(Cmp, 83, 
CmpAndSetFlags(emu.flags,
GET_X_REG(mod_rm.RM_Mod.reg_val),
static_cast<u16>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
GET_EXT_REG(mod_rm.RM_Mod.reg_val),
static_cast<u32>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
mod_rm.RM_Mod.reg_val,
static_cast<u64>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value(),
static_cast<u16>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value(),
static_cast<u32>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value(),
static_cast<u64>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
static_cast<u16>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
static_cast<u32>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value(),
static_cast<u64>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u16>(calc_offset + disp).value(),
static_cast<u16>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u32>(calc_offset + disp).value(),
static_cast<u32>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u64>(calc_offset + disp).value(),
static_cast<u64>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u16>(calc_offset).value(),
static_cast<u16>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u32>(calc_offset).value(),
static_cast<u32>(ReadFromVec<u8>(inst, 2))),
CmpAndSetFlags(emu.flags,
emu.memory.Read<u64>(calc_offset).value(),
static_cast<u64>(ReadFromVec<u8>(inst, 2))))
#pragma endregion

#pragma region Test (0x85)
INSTRUCTION_OP2_MR(Test, 85, 
TestAndSetFlags(emu.flags,
GET_X_REG(mod_rm.RM_Mod.reg_val),
GET_X_REG(mod_rm.Reg.val)),
TestAndSetFlags(emu.flags,
GET_EXT_REG(mod_rm.RM_Mod.reg_val),
GET_EXT_REG(mod_rm.Reg.val)),
TestAndSetFlags(emu.flags,
mod_rm.RM_Mod.reg_val,
mod_rm.Reg.val),
TestAndSetFlags(emu.flags,
emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value(),
GET_X_REG(mod_rm.Reg.val)),
TestAndSetFlags(emu.flags,
emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value(),
GET_EXT_REG(mod_rm.Reg.val)),
TestAndSetFlags(emu.flags,
emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value(),
mod_rm.Reg.val),
TestAndSetFlags(emu.flags,
emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
GET_X_REG(mod_rm.Reg.val)),
TestAndSetFlags(emu.flags,
emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
GET_EXT_REG(mod_rm.Reg.val)),
TestAndSetFlags(emu.flags,
emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value(),
mod_rm.Reg.val),
TestAndSetFlags(emu.flags,
emu.memory.Read<u16>(calc_offset + disp).value(),
GET_X_REG(mod_rm.Reg.val)),
TestAndSetFlags(emu.flags,
emu.memory.Read<u32>(calc_offset + disp).value(),
GET_EXT_REG(mod_rm.Reg.val)),
TestAndSetFlags(emu.flags,
emu.memory.Read<u64>(calc_offset + disp).value(),
mod_rm.Reg.val),
TestAndSetFlags(emu.flags,
emu.memory.Read<u16>(calc_offset).value(),
GET_X_REG(mod_rm.Reg.val)),
TestAndSetFlags(emu.flags,
emu.memory.Read<u32>(calc_offset).value(),
GET_EXT_REG(mod_rm.Reg.val)),
TestAndSetFlags(emu.flags,
emu.memory.Read<u64>(calc_offset).value(),
mod_rm.Reg.val))
#pragma endregion

//TODO: probably add an method called "Push" in emu to assist the operation for pushing the values onto the stack
#pragma region Push (0x50-0x57)
void Push_50_57(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst) {
	emu.SetReg(Register::Rsp, emu.Reg(Register::Rsp) - 8);
	if (!ctx->pfx_p_rex)
		emu.memory.Write(emu.Reg(Register::Rsp), emu.Reg(static_cast<Register>(inst[0] - 0x50)));
	else if (_REX_B(ctx->pfx_rex))
		emu.memory.Write(emu.Reg(Register::Rsp), emu.Reg(static_cast<Register>((inst[0] - 0x50) + 8)));
}		
#pragma endregion

#pragma region Call (0xE8)
void Call_E8(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst, u64& pc) {
	//Push the rip(add to next instruction after the call) onto the stack
	//rip = rip + instruction size(set to next instruction)
	emu.SetReg(Register::Rip, emu.Reg(Register::Rip) + inst.size());
	emu.SetReg(Register::Rsp, emu.Reg(Register::Rsp) - 8);
	emu.memory.Write(emu.Reg(Register::Rsp), emu.Reg(Register::Rip));

	pc = emu.Reg(Register::Rip) + ReadFromVec<s32>(inst, 1) - inst.size(); //rel32
	std::cout << "\nSetting rip to: 0x" << std::hex << pc << "\n";
}
#pragma endregion
#pragma region Call (0xFF-reg2)
void Call_FF_reg2(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst, u64& pc) {

	auto next_instr_addr = emu.Reg(Register::Rip) + inst.size() + (ctx->pfx_p_rex ? 1 : 0);
	emu.SetReg(Register::Rsp, emu.Reg(Register::Rsp) - 8);
	emu.memory.Write(emu.Reg(Register::Rsp), next_instr_addr);

	VirtualAddr call_addr = 0;

	OperandSize opsize = ctx->osize;
	ModRM mod_rm;
	Handle_ModRM(emu, ctx, mod_rm);
	if (!mod_rm.RM_Mod.disp && !mod_rm.RM_Mod.RMRegSet) return;
	std::cout << "Calling functions...\n";
	if (!ctx->p_sib) {
		if (!mod_rm.RM_Mod.IsPtr && mod_rm.RM_Mod.RMRegSet) {
			if (opsize == OperandSize::X86_Osize_16bit) {
				call_addr = static_cast <u16> (mod_rm.RM_Mod.reg_val & 0xFFFF);
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				call_addr = static_cast <u32> (mod_rm.RM_Mod.reg_val & 0xFFFFFFFF);
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				call_addr = mod_rm.RM_Mod.reg_val;
			}
		}
		else if (!mod_rm.RM_Mod.disp) {
			if (opsize == OperandSize::X86_Osize_16bit) {
				call_addr = emu.memory.Read < u16 >(static_cast <u16> (mod_rm.RM_Mod.reg_val & 0xFFFF)).value();
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				call_addr = emu.memory.Read < u32 >(static_cast <u32> (mod_rm.RM_Mod.reg_val & 0xFFFFFFFF)).value();
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				call_addr = emu.memory.Read < u64 >(mod_rm.RM_Mod.reg_val).value();
			}
		}
		else if (mod_rm.RM_Mod.RMRegSet && mod_rm.RM_Mod.disp) {
			s64 disp = ReadDispFromVec < s64 >(inst, mod_rm.RM_Mod.disp, 2).value();
			if (opsize == OperandSize::X86_Osize_16bit) {
				call_addr = emu.memory.Read < u16 >(static_cast <u16> (mod_rm.RM_Mod.reg_val & 0xFFFF) + disp).value();
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				call_addr = emu.memory.Read < u32 >(static_cast <u32> (mod_rm.RM_Mod.reg_val & 0xFFFFFFFF) + disp).value();
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				call_addr = emu.memory.Read < u64 >(mod_rm.RM_Mod.reg_val + disp).value();
			}
		}
		else if (mod_rm.RM_Mod.disp) {
			s64 disp = CastFromVec<s32>(inst, 2);
			std::cout << std::hex << "0x" << disp << '\n';
			if (opsize == OperandSize::X86_Osize_16bit) {
				call_addr = emu.memory.Read <u16>(next_instr_addr + disp).value();
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				std::cout << "Next instruction after call at: 0x" << next_instr_addr << "\n";
				std::cout << "Displacement: 0x" << disp << "\n";

				std::cout << "Read from: 0x" << std::hex << next_instr_addr + disp << "\n";

				call_addr = emu.memory.Read <u32>(next_instr_addr + disp).value();

				std::cout << "Next instruction: 0x" << std::hex << emu.memory.Read <u32>(next_instr_addr + static_cast<u32>(disp)).value() << "\n";
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				call_addr = emu.memory.Read <u64>(next_instr_addr + disp).value();
			}
		}
	}
	else {
		Sib sib_byte;
		u64 calc_offset = 0;
		HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);
		if (!sib_byte.valid) return;
		std::cout << "entering sib proccessing...\n";
		if (mod_rm.RM_Mod.disp) {
			s64 disp = ReadDispFromVec < s64 >(inst, mod_rm.RM_Mod.disp, 3).value();
			if (opsize == OperandSize::X86_Osize_16bit) {
				call_addr = emu.memory.Read < u16 >(calc_offset + disp).value();
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				call_addr = emu.memory.Read < u32 >(calc_offset + disp).value();
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				call_addr = emu.memory.Read < u64 >(calc_offset + disp).value();
			}
		}
		else {
			if (opsize == OperandSize::X86_Osize_16bit) {
				call_addr = emu.memory.Read < u16 >(next_instr_addr + calc_offset).value();
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				call_addr = emu.memory.Read < u32 >(next_instr_addr + calc_offset).value();
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				call_addr = emu.memory.Read < u64 >(next_instr_addr + calc_offset).value();
			}
		}
	}
	pc = emu.Reg(Register::Rip) + call_addr;
	std::cout << "\nSetting rip to: 0x" << std::hex << pc << "\n";
	return;
}
#pragma endregion