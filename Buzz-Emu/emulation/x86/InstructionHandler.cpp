
#include <iostream>
#include "Flags.hpp"
#include "../../core/Fs.hpp"
#include "InstructionHandler.hpp"

#pragma region Add (0x01)
void Add_01(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MR<decltype(AddAndSetFlags),
		u16, u32, u64,
		u16, u32, u64>(emu, ctx, inst, AddAndSetFlags, modrm, modrm.rm.reg_val, modrm.Reg.val, emu.flags);
}
#pragma endregion
#pragma region Add (0x03)
void Add_03(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_RM<decltype(AddAndSetFlags), 
		u16, u32, u64,
		u16, u32, u64>(emu, ctx, inst, AddAndSetFlags, modrm, modrm.Reg.val, modrm.rm.reg_val, emu.flags);
}
#pragma endregion
#pragma region Add (0x05)
void Add_05(BUZE_STANDARD_PARAM) {
	def_instruction_op2_I<decltype(AddAndSetFlags),
		imm16, imm32, imm32>(emu, ctx, inst, AddAndSetFlags, 1, emu.flags);
}
#pragma endregion
#pragma region Add (0x81)
void Add_81(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MI<decltype(AddAndSetFlags),
		u16, u32, u64,
		imm16, imm32, imm32>(emu, ctx, inst, AddAndSetFlags, modrm, modrm.rm.reg_val, 2, emu.flags);
}
#pragma endregion
#pragma region Add (0x83)
void Add_83(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MI<decltype(AddAndSetFlags),
		s16, s32, s64,
		s8, s8, s8>(emu, ctx, inst, AddAndSetFlags, modrm, modrm.rm.reg_val, INSTR_POS(2), emu.flags);
}
#pragma endregion

#pragma region Or (0x09)
void Or_09(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_I<decltype(OrAndSetFlags),
		imm16, imm32, imm32>(emu, ctx, inst, OrAndSetFlags, 1, emu.flags);
}
#pragma endregion
#pragma region Or (0x0D)
void Or_0D(BUZE_STANDARD_PARAM) {
	def_instruction_op2_I<decltype(OrAndSetFlags),
		imm16, imm32, imm32>(emu, ctx, inst, OrAndSetFlags, INSTR_POS(1), emu.flags);
}
#pragma endregion
#pragma region Or (0x81)
void Or_81(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MI<decltype(OrAndSetFlags),
		u16, u32, u64,
		imm16, imm32, imm32>(emu, ctx, inst, OrAndSetFlags, modrm, modrm.rm.reg_val, 2, emu.flags);
}
#pragma endregion
#pragma region Or (0x83)
void Or_83(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MI<decltype(OrAndSetFlags),
		u16, u32, u64,
		imm8, imm8, imm8>(emu, ctx, inst, OrAndSetFlags, modrm, modrm.rm.reg_val, 2, emu.flags);
}
#pragma endregion

#pragma region And (0x21)
void And_21(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MR<decltype(AndAndSetFlags),
		u16, u32, u64,
		u16, u32, u64>(emu, ctx, inst, AndAndSetFlags, modrm, modrm.rm.reg_val, modrm.Reg.val, emu.flags);
}
#pragma endregion
#pragma region And (0x25)
void And_25(BUZE_STANDARD_PARAM) {
	def_instruction_op2_I<decltype(AndAndSetFlags),
		imm16, imm32, imm32>(emu, ctx, inst, AndAndSetFlags, INSTR_POS(1), emu.flags);
}
#pragma endregion
#pragma region And (0x81)
void And_81(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MI<decltype(AndAndSetFlags),
		u16, u32, u64,
		imm16, imm32, imm32>(emu, ctx, inst, AndAndSetFlags, modrm, modrm.rm.reg_val, 2, emu.flags);
}
#pragma endregion
#pragma region And (0x83)
void And_83(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MI<decltype(AndAndSetFlags),
		u16, u32, u64,
		imm8, imm8, imm8>(emu, ctx, inst, AndAndSetFlags, modrm, modrm.rm.reg_val, 2, emu.flags);
}
#pragma endregion

#pragma region Sub (0x29)
void Sub_29(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MR<decltype(SubAndSetFlags),
		u16, u32, u64,
		u16, u32, u64>(emu, ctx, inst, SubAndSetFlags, modrm, modrm.rm.reg_val, modrm.Reg.val, emu.flags);
}
#pragma endregion
#pragma region Sub (0x2B)
void Sub_2B(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_RM<decltype(SubAndSetFlags),
		u16, u32, u64,
		u16, u32, u64>(emu, ctx, inst, SubAndSetFlags, modrm, modrm.Reg.val, modrm.rm.reg_val, emu.flags);
}
#pragma endregion
#pragma region Sub (0x81)
void Sub_81(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MI<decltype(SubAndSetFlags),
		u16, u32, u64,
		imm16, imm32, imm32>(emu, ctx, inst, SubAndSetFlags, modrm, modrm.rm.reg_val, 2, emu.flags);
}
#pragma endregion
#pragma region Sub (0x83)
void Sub_83(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MI<decltype(SubAndSetFlags),
		u16, u32, u64,
		imm8, imm8, imm8>(emu, ctx, inst, SubAndSetFlags, modrm, modrm.rm.reg_val, ctx->pos_opcode + 2, emu.flags);
}
#pragma endregion

#pragma region Xor (0x31)
void Xor_31(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MR<decltype(XorAndSetFlags),
		u16, u32, u64,
		u16, u32, u64>(emu, ctx, inst, XorAndSetFlags, modrm, modrm.rm.reg_val, modrm.Reg.val, emu.flags);
}
#pragma endregion
#pragma region Xor (0x33)
void Xor_33(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_RM<decltype(XorAndSetFlags),
		u16, u32, u64,
		u16, u32, u64>(emu, ctx, inst, XorAndSetFlags, modrm, modrm.Reg.val, modrm.rm.reg_val, emu.flags);
}
#pragma endregion
#pragma region Xor (0x35)
void Xor_35(BUZE_STANDARD_PARAM) {
	def_instruction_op2_I<decltype(XorAndSetFlags),
		imm16, imm32, imm32>(emu, ctx, inst, XorAndSetFlags, INSTR_POS(1), emu.flags);
}
#pragma endregion
#pragma region Xor (0x81)
void Xor_81(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MI<decltype(XorAndSetFlags),
		u16, u32, u64,
		imm16, imm32, imm32>(emu, ctx, inst, XorAndSetFlags, modrm, modrm.rm.reg_val, 2, emu.flags);
}
#pragma endregion
#pragma region Xor (0x83)
void Xor_83(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MI<decltype(XorAndSetFlags),
		u16, u32, u64,
		imm8, imm8, imm8>(emu, ctx, inst, XorAndSetFlags, modrm, modrm.rm.reg_val, INSTR_POS(2), emu.flags);
}
#pragma endregion

#pragma region Mov (0x88)
void Mov_88(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MR8<decltype(mov_operation)>
		(emu, ctx, inst, mov_operation, modrm, modrm.rm.reg_val, modrm.Reg.val);
}
#pragma endregion
#pragma region Mov (0x89)
void Mov_89(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst) {
	OperandSize opsize = ctx->osize; ModRM mod_rm; set_modrm_byte(emu, ctx, mod_rm); if (!mod_rm.rm.disp_size && !mod_rm.rm.reg_set) return; if (!ctx->p_sib) {
		if (!mod_rm.rm.is_addr && mod_rm.rm.reg_set) {
			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.SetReg<u16>(mod_rm.rm.reg, static_cast<u16>(mod_rm.Reg.val & 0xFFFF));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.SetReg<u32>(mod_rm.rm.reg, static_cast<u32>(mod_rm.Reg.val & 0xFFFFFFFF));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				emu.SetReg<u64>(mod_rm.rm.reg, mod_rm.Reg.val);
			}
		}
		else if (!mod_rm.rm.disp_size) {
			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(static_cast<u16>(mod_rm.rm.reg_val & 0xFFFF), to_byte_vec(static_cast<u16>(mod_rm.Reg.val & 0xFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(static_cast<u32>(mod_rm.rm.reg_val & 0xFFFFFFFF), to_byte_vec(static_cast<u32>(mod_rm.Reg.val & 0xFFFFFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				emu.memory.WriteFrom(mod_rm.rm.reg_val, to_byte_vec(mod_rm.Reg.val));
			}
		}
		else if (mod_rm.rm.reg_set && mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(2)).value(); if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(static_cast<u16>(mod_rm.rm.reg_val & 0xFFFF) + disp, to_byte_vec(static_cast<u16>(mod_rm.Reg.val & 0xFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(static_cast<u32>(mod_rm.rm.reg_val & 0xFFFFFFFF) + disp, to_byte_vec(static_cast<u32>(mod_rm.Reg.val & 0xFFFFFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				emu.memory.WriteFrom(mod_rm.rm.reg_val + disp, to_byte_vec(mod_rm.Reg.val));
			}
		}
	}
	else {
		Sib sib_byte; u64 calc_offset = 0; set_sib_byte(emu, ctx, mod_rm, sib_byte, calc_offset); if (!sib_byte.valid) return; std::cout << "entering sib proccessing...\n"; if (mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(3)).value(); if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(calc_offset + disp, to_byte_vec(static_cast<u16>(mod_rm.Reg.val & 0xFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(calc_offset + disp, to_byte_vec(static_cast<u32>(mod_rm.Reg.val & 0xFFFFFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				emu.memory.WriteFrom(calc_offset + disp, to_byte_vec(mod_rm.Reg.val));
			}
		}
		else {
			if (opsize == OperandSize::X86_Osize_16bit) {
				emu.memory.WriteFrom(calc_offset, to_byte_vec(static_cast<u16>(mod_rm.Reg.val & 0xFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				emu.memory.WriteFrom(calc_offset, to_byte_vec(static_cast<u32>(mod_rm.Reg.val & 0xFFFFFFFF)));
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				emu.memory.WriteFrom(calc_offset, to_byte_vec(mod_rm.Reg.val));
			}
		}
	} return;
}
#pragma endregion
#pragma region Mov (0x8B)
void Mov_8B(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);

	def_instruction_op2_RM<decltype(mov_operation),
		u16, u32, u64,
		u16, u32, u64>(emu, ctx, inst, mov_operation, modrm, modrm.Reg.val, modrm.rm.reg_val);
}
#pragma endregion
#pragma region Mov (0xB8 - 0xBF)
void Mov_B8_BF(BUZE_STANDARD_PARAM) {
	auto reg_extended = _REX_B(ctx->pfx_rex) << 3; // 1 * 8, 0 * 8
	auto reg = static_cast<Register>((inst[INSTR_POS(0)] - 0xB8) + reg_extended);

	if (ctx->osize == X86_Osize_16bit)
		emu.SetReg<u16>(reg, read_from_vec<u16>(inst, INSTR_POS(1)));
	else if (ctx->osize == X86_Osize_32bit)
		emu.SetReg<u32>(reg, read_from_vec<u32>(inst, INSTR_POS(1)));
	else if (ctx->osize == X86_Osize_64bit)
		emu.SetReg<u64>(reg, read_from_vec<u64>(inst, INSTR_POS(1)));
}
#pragma endregion
#pragma region Mov (0xC7)
void Mov_C7(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);

	def_instruction_op2_MI<decltype(mov_operation),
		u16, u32, s64,
		u16, u32, s32>(emu, ctx, inst, mov_operation, modrm, modrm.rm.reg_val, INSTR_POS(2));
}
#pragma endregion

#pragma region Lea (0x8D)
void Lea_8D(BUZE_STANDARD_PARAM, u64& pc) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);

	switch (GET_ADDRSIZE_ENUM(ctx->asize)) {
	case AddressingSize::X86_Asize_32bit:
		def_instruction_op2_RM<decltype(Lea),
			u16, u32, u64,
			u32, u32, u32,
			u64, AddressMode::AsValue>
			(emu, ctx, inst, Lea, modrm, modrm.Reg.val, modrm.rm.reg_val);
		break;
	case AddressingSize::X86_Asize_64bit:
		def_instruction_op2_RM<decltype(Lea),
			u16, u32, u64,
			u64, u64, u64,
			u64, AddressMode::AsValue>
			(emu, ctx, inst, Lea, modrm, modrm.Reg.val, modrm.rm.reg_val);
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region Movsxd (0x63)
void Movsxd_63(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);

	def_instruction_op2_RM<decltype(mov_operation),
		s16, s32, s64,
		s16, s32, s32>(emu, ctx, inst, mov_operation, modrm, modrm.Reg.val, modrm.rm.reg_val);
}
#pragma endregion

#pragma region Movzx(0x0FB6)
void Movzx_0FB6(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm, ModRegRMType::RM_8bit); //switch to 8-bit registering mode
	def_instruction_op2_RM<decltype(mov_operation),
		u16, u32, u64,
		u8, u8, u8>(emu, ctx, inst, mov_operation, modrm, modrm.Reg.val, modrm.rm.reg_val);
}
#pragma endregion

#pragma region Cmp (0x3B)
void Cmp_3B(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);

	def_instruction_op2_RM<decltype(CmpAndSetFlags),
		u16, u32, u64,
		u16, u32, u64>(emu, ctx, inst, CmpAndSetFlags, modrm, modrm.Reg.val, modrm.rm.reg_val, emu.flags);
}
#pragma endregion
#pragma region Cmp (0x83)
void Cmp_83(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);

	def_instruction_op2_MI<decltype(CmpAndSetFlags),
		u16, u32, u64,
		imm8, imm8, imm8>(emu, ctx, inst, CmpAndSetFlags, modrm, modrm.rm.reg_val, INSTR_POS(2), emu.flags);
}
#pragma endregion

#pragma region Test (0x84)
void Test_84(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	//full 8-bit mode
	redef_modrm_rm8(emu, modrm);
	redef_modrm_reg8(emu, modrm);

	def_instruction_op2_MR8<decltype(TestAndSetFlags)>
		(emu, ctx, inst, TestAndSetFlags, modrm, modrm.rm.reg_val, modrm.Reg.val, emu.flags);
}
#pragma endregion
#pragma region Test (0x85)
void Test_85(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op2_MR<decltype(TestAndSetFlags),
		u16, u32, u64,
		u16, u32, u64>(emu, ctx, inst, TestAndSetFlags, modrm, modrm.rm.reg_val, modrm.Reg.val, emu.flags);
}
#pragma endregion
//TODO: probably add an method called "Push" in emu to assist the operation for pushing the values onto the stack
#pragma region Push (0x50-0x57)
void Push_50_57(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst) {
	emu.SetReg<u64>(Register::Rsp, emu.Reg(Register::Rsp) - 8);
	if (!ctx->pfx_p_rex)
		emu.memory.Write(emu.Reg(Register::Rsp), emu.Reg(static_cast<Register>(inst[INSTR_POS(0)] - 0x50)));
	else if (_REX_B(ctx->pfx_rex)) {
		emu.memory.Write(emu.Reg(Register::Rsp), emu.Reg(static_cast<Register>((inst[INSTR_POS(0)] - 0x50) + 8)));
		std::cout << "PUSH: extended register used: R" << std::dec << (int)((inst[INSTR_POS(0)] - 0x50) + 8) << std::hex << "\n";
	}
}
#pragma endregion

#pragma region Pop (0x58-0x5F)
void Pop_58_5F(BUZE_STANDARD_PARAM) {
	if (!ctx->pfx_p_rex)
		emu.SetReg<u64>(static_cast<Register>(inst[INSTR_POS(0)] - 0x50), emu.memory.Read<u64>(emu.Reg(Register::Rsp)).value());
	else if (_REX_B(ctx->pfx_rex)) {
		emu.SetReg(static_cast<Register>((inst[INSTR_POS(0)] - 0x50) + 8), emu.memory.Read<u64>(emu.Reg(Register::Rsp)).value());
		std::cout << "POP: extended register used: R" << std::dec << (int)((inst[INSTR_POS(0)] - 0x50) + 8) << std::hex << "\n";
	}
}
#pragma endregion

#pragma region Call (0xE8)
void Call_E8(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst, u64& pc) {
	//Push the rip(add to next instruction after the call) onto the stack
	//rip = rip + instruction size(set to next instruction)
	emu.SetReg<u64>(Register::Rip, emu.Reg(Register::Rip) + inst.size());
	emu.SetReg<u64>(Register::Rsp, emu.Reg(Register::Rsp) - 8);
	emu.memory.Write(emu.Reg(Register::Rsp), emu.Reg(Register::Rip));

	pc = emu.Reg(Register::Rip) + read_from_vec<s32>(inst, INSTR_POS(1)) - inst.size(); //rel32
	std::cout << "\nSetting rip to: 0x" << std::hex << pc << "\n";
}
#pragma endregion
#pragma region Call (0xFF-reg2)
void Call_FF_reg2(Emulator& emu, x86Dcctx* ctx, const std::vector<u8>& inst, u64& pc) {

	auto next_instr_addr = emu.Reg(Register::Rip) + inst.size() + (ctx->pfx_p_rex ? 1 : 0);
	emu.SetReg<u64>(Register::Rsp, emu.Reg(Register::Rsp) - 8);
	emu.memory.Write(emu.Reg(Register::Rsp), next_instr_addr);

	VirtualAddr call_addr = 0;

	OperandSize opsize = ctx->osize;
	ModRM mod_rm;
	set_modrm_byte(emu, ctx, mod_rm);
	if (!mod_rm.rm.disp_size && !mod_rm.rm.reg_set) return;
	std::cout << "Calling functions...\n";
	if (!ctx->p_sib) {
		if (!mod_rm.rm.is_addr && mod_rm.rm.reg_set) {
			if (opsize == OperandSize::X86_Osize_16bit) {
				call_addr = static_cast <u16> (mod_rm.rm.reg_val & 0xFFFF);
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				call_addr = static_cast <u32> (mod_rm.rm.reg_val & 0xFFFFFFFF);
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				call_addr = mod_rm.rm.reg_val;
			}
		}
		else if (!mod_rm.rm.disp_size) {
			if (opsize == OperandSize::X86_Osize_16bit) {
				call_addr = emu.memory.Read < u16 >(static_cast <u16> (mod_rm.rm.reg_val & 0xFFFF)).value();
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				call_addr = emu.memory.Read < u32 >(static_cast <u32> (mod_rm.rm.reg_val & 0xFFFFFFFF)).value();
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				call_addr = emu.memory.Read < u64 >(mod_rm.rm.reg_val).value();
			}
		}
		else if (mod_rm.rm.reg_set && mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst < s64 >(inst, mod_rm.rm.disp_size, 2).value();
			if (opsize == OperandSize::X86_Osize_16bit) {
				call_addr = emu.memory.Read < u16 >(static_cast <u16> (mod_rm.rm.reg_val & 0xFFFF) + disp).value();
			}
			else if (opsize == OperandSize::X86_Osize_32bit) {
				call_addr = emu.memory.Read < u32 >(static_cast <u32> (mod_rm.rm.reg_val & 0xFFFFFFFF) + disp).value();
			}
			else if (opsize == OperandSize::X86_Osize_64bit && ((ctx->pfx_rex >> 3) & 1)) {
				call_addr = emu.memory.Read < u64 >(mod_rm.rm.reg_val + disp).value();
			}
		}
		else if (mod_rm.rm.disp_size) {
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
		set_sib_byte(emu, ctx, mod_rm, sib_byte, calc_offset);
		if (!sib_byte.valid) return;
		std::cout << "entering sib proccessing...\n";
		if (mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst < s64 >(inst, mod_rm.rm.disp_size, 3).value();
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

#pragma region Dec (0xFF)
void Dec_FF(BUZE_STANDARD_PARAM) {
	ModRM modrm;
	set_modrm_byte(emu, ctx, modrm);
	def_instruction_op1_M<decltype(dec_and_set_flags),
		u16, u32, u64>(emu, ctx, inst, dec_and_set_flags, modrm, modrm.rm.reg_val, emu.flags);
}
#pragma endregion

#pragma region Jmp (0xE9)
void Jmp_E9(BUZE_STANDARD_PARAM, u64& pc) {
	switch (GET_OPSIZE_ENUM(ctx->osize))
	{
	case OperandSize::X86_Osize_16bit:
		pc = emu.Reg(Register::Rip) + static_cast<s64>(read_from_vec<s16>(inst, 1));
		break;
	case OperandSize::X86_Osize_32bit:
		pc = emu.Reg(Register::Rip) + static_cast<s64>(read_from_vec<s32>(inst, 1));
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region Ret (0xC3)
void Ret_C3(BUZE_STANDARD_PARAM, u64& pc) {
	// Read the return address from the stack (top of stack)
	pc = emu.memory.Read<VirtualAddr>(emu.Reg(Register::Rsp)).value();

	// Adjust the stack pointer to remove the return address from the stack
	emu.SetReg<u64>(Register::Rsp, emu.Reg(Register::Rsp) + 8); // For 64-bit addresses
}
#pragma endregion