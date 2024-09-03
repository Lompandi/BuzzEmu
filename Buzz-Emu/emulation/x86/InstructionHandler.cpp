
#include "Sib.hpp"
#include "ModRM.hpp"


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
			s64 disp = ReadFromVec<s64>(inst, mod_rm.RM_Mod.disp, 2).value();

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

		HandleSib(emu, ctx, mod_rm, sib_byte, calc_offset);
		if (!sib_byte.valid)
			return;

		if (mod_rm.RM_Mod.disp) { //SIB + displacement
			s64 disp = ReadFromVec<s64>(inst, mod_rm.RM_Mod.disp, 3).value();
			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(calc_offset + disp,
					ToByteVector(emu.memory.Read<u16>(calc_offset + disp).value() +
						GET_X_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(calc_offset + disp,
					ToByteVector(emu.memory.Read<u32>(calc_offset + disp).value() +
						GET_EXT_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {
				emu.memory.WriteFrom(calc_offset + disp,
					ToByteVector(emu.memory.Read<u64>(calc_offset + disp).value() +
						mod_rm.Reg.val));
			}
		}
		else { //direct sib addressing
			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(calc_offset,
					ToByteVector(emu.memory.Read<u16>(calc_offset).value() +
						GET_X_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(calc_offset,
					ToByteVector(emu.memory.Read<u32>(calc_offset).value() +
						GET_EXT_REG(mod_rm.Reg.val)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && _REX_W(ctx->pfx_rex)) {
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
INSTRUCTION_OP2(Add, 03, 
	/*store to register from register*/
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags, 
		GET_X_REG(mod_rm.RM_Mod.reg_val),
		GET_X_REG(mod_rm.Reg.val)));,
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		GET_EXT_REG(mod_rm.RM_Mod.reg_val),
		GET_EXT_REG(mod_rm.Reg.val)));,
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		mod_rm.RM_Mod.reg_val,
		mod_rm.Reg.val));,
	/*store to register from memory, no displacement*/
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val)).value(),
		GET_X_REG(mod_rm.Reg.val)));,
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val)).value(),
		GET_EXT_REG(mod_rm.Reg.val)));,
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val).value(),
		mod_rm.Reg.val));,
	/*store to register from memory, displacement*/
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(GET_X_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
		GET_X_REG(mod_rm.Reg.val)));,
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(GET_EXT_REG(mod_rm.RM_Mod.reg_val) + disp).value(),
		GET_EXT_REG(mod_rm.Reg.val)));,
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(mod_rm.RM_Mod.reg_val + disp).value(),
		mod_rm.Reg.val));,
	/*SIB byte extension, displacement*/
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(calc_offset + disp).value(),
		GET_X_REG(mod_rm.Reg.val)));,
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(calc_offset + disp).value(),
		GET_EXT_REG(mod_rm.Reg.val)));, 
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(calc_offset + disp).value(), 
		mod_rm.Reg.val));, 
	/*SIB byte extension, no displacement*/
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u16>(calc_offset).value(),
		GET_X_REG(mod_rm.Reg.val)));,
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u32>(calc_offset).value(),
		GET_EXT_REG(mod_rm.Reg.val)));,
	emu.SetReg(mod_rm.Reg.reg,
		AddAndSetFlags(emu.flags,
		emu.memory.Read<u64>(calc_offset).value(),
		mod_rm.Reg.val));)
#pragma endregion

#pragma region Or (0x09)
INSTRUCTION_LOGICAL_OP2_RM_REG(Or, 09, |)
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
INSTRUCTION_ARITH_OP2_RM_REG(Sub, 29, SubAndSetFlags)
#pragma endregion

#pragma region Xor (0x31)
INSTRUCTION_LOGICAL_OP2_RM_REG(Xor, 31, ^)
#pragma endregion
#pragma region Xor (0x35)
INSTRUCTION_LOGICAL_AX_IMM(Xor, 35, ^)
#pragma endregion

