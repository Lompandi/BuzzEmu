#pragma once

#include "Sib.hpp"
#include "ModRM.hpp"
#include "../../core/Fs.hpp"
#include "../../core/Memtypes.hpp"

/*
for displacement, there will be a varible name "disp" containing the displacement value
for SIB offset, it will be "calc_offset"
*/
//c++20 function def:

#define GET_L_REG(r) static_cast<u8>(r & 0xFF)
#define GET_H_REG(r) static_cast<u8>((r >> 8) & 0xFF)
#define GET_X_REG(r) static_cast<u16>(r & 0xFFFF)
#define GET_EXT_REG(r) static_cast<u32>(r & 0xFFFFFFFF)

#define GET_OPSIZE_ENUM(o) (o & 0x00000003)
#define GET_ADDRSIZE_ENUM(a) (a & 0x00000003)

//instruction position
#define _INSTR_POS(ctx, offset) (ctx)->pos_opcode + offset
#define INSTR_POS(offset) _INSTR_POS(ctx, offset)


//helper function to unpack :
// Helper function to unpack arguments from a tuple and call a function

//TODO: RECODE THIS TO FORM AN AL-AH register encoding instead of 32-64 bit register encoding
template <typename FuncType, typename... ExtraArgs>
void def_instruction_op2_MR8(Emulator& emu,
	x86Dcctx* ctx,
	const std::vector<u8>& inst,
	FuncType instr_emu_func,
	ModRM& mod_rm,
	u64 op1,
	u64 op2,
	ExtraArgs... extra_args) {

	if (!mod_rm.rm.disp_size && !mod_rm.rm.reg_set) return;

	ByteRegister reg_mask = ByteRegister::LowByte; //defualt: L registers

	u64 fetch_reg_mask_op1 = mask_regs_low;
	u64 fetch_reg_mask_op2 = (mod_rm.Reg.reg > 3 ? mask_regs_high : mask_regs_low);

	//if reg value is greater than 3, then we will then set it back to ax position
	mod_rm.Reg.reg = static_cast<Register>((mod_rm.Reg.reg > 3 ? mod_rm.Reg.reg - 4 : mod_rm.Reg.reg));

	if (mod_rm.rm.reg_set) {
		fetch_reg_mask_op1 = (mod_rm.rm.reg > 3 ? mask_regs_high : mask_regs_low);
		reg_mask = (mod_rm.rm.reg > 3 ? ByteRegister::HighByte : ByteRegister::LowByte);

		//if R/M-reg value is greater than 3, then we will then set it back to ax position
		mod_rm.rm.reg = static_cast<Register>((mod_rm.rm.reg > 3 ? mod_rm.rm.reg - 4 : mod_rm.rm.reg));
	}

	if (!ctx->p_sib) {
		if (!mod_rm.rm.is_addr && mod_rm.rm.reg_set) {
			emu.SetReg<u8>(mod_rm.rm.reg, call_function(
				instr_emu_func,
				get_byte_register(op1, fetch_reg_mask_op1),
				get_byte_register(op2, fetch_reg_mask_op2),
				extra_args...
			), reg_mask);
		}
		else if (!mod_rm.rm.disp_size && mod_rm.rm.reg_set) {
			emu.memory.Write<u8>(get_byte_register(emu.Reg(mod_rm.rm.reg), fetch_reg_mask_op1),
				static_cast<u8>(call_function(
					instr_emu_func,
					emu.memory.Read<u8>(get_byte_register(op1, fetch_reg_mask_op1)).value(),
					get_byte_register(op2, fetch_reg_mask_op2),
					extra_args...)
					));
		}
		else if (mod_rm.rm.reg_set && mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(2)).value();

			emu.memory.Write<u8>(get_byte_register(emu.Reg(mod_rm.rm.reg), fetch_reg_mask_op1) + disp,
				static_cast<u8>(call_function(
					instr_emu_func,
					emu.memory.Read<u8>(get_byte_register(op1, fetch_reg_mask_op1) + disp).value(),
					get_byte_register(op2, fetch_reg_mask_op2),
					extra_args...)
					));
		}
		else if (mod_rm.rm.disp_size) {
			s64 disp = emu.Reg(Register::Rip) + read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(2)).value() + inst.size();
			std::cout << "RM instruction: Accessing disp32: " << std::hex << disp << "\n";

			emu.memory.Write<u8>(disp,
				static_cast<u8>(call_function(
					instr_emu_func,
					emu.memory.Read<u8>(disp).value(),
					get_byte_register(op2, fetch_reg_mask_op2),
					extra_args...)
					));
		}
	}
	else {
		std::cout << "Entering SIB procession...\n";
		Sib sib_byte;
		u64 calc_offset = 0;
		set_sib_byte(emu, ctx, mod_rm, sib_byte, calc_offset);
		if (!sib_byte.valid)
			return;

		if (mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(3)).value();

			emu.memory.Write<u8>(calc_offset,
				static_cast<u8>(call_function(
					instr_emu_func,
					emu.memory.Read<u8>(calc_offset + disp).value(),
					get_byte_register(op2, fetch_reg_mask_op2),
					extra_args...)
					));
		}																																																					   \
		else {
			emu.memory.Write<u8>(calc_offset,
				static_cast<u8>(call_function(
					instr_emu_func,
					emu.memory.Read<u8>(calc_offset).value(),
					get_byte_register(op2, fetch_reg_mask_op2),
					extra_args...)
					));
		}
	}
	return;
}

template <typename FuncType,
	std::integral OPtype16, std::integral OPtype32, std::integral OPtype64,
	std::integral OP2type16, std::integral OP2type32, std::integral OP2type64,
	typename... ExtraArgs>
void def_instruction_op2_RM(Emulator& emu,
	x86Dcctx* ctx,
	const std::vector<u8>& inst,
	FuncType instr_emu_func,
	ModRM& mod_rm,
	u64 op1,
	u64 op2,
	ExtraArgs... extra_args) {

	/*auto execute_instruction_reg = [&](auto OperandType, auto Operand2Type) {
		if constexpr (std::is_void_v<FuncType>) {
			call_function(
				instr_emu_func,
				static_cast<decltype(OperandType)>(op1),
				static_cast<decltype(OperandType)>(static_cast<decltype(Operand2Type)>(op2)),
				std::forward<ExtraArgs>(extra_args)...);
		}
		else {
			emu.SetReg<decltype(OperandType)>(mod_rm.Reg.reg,
				call_function(
					instr_emu_func,
					static_cast<decltype(OperandType)>(op1),
					static_cast<decltype(OperandType)>(static_cast<decltype(Operand2Type)>(op2)),
					std::forward<ExtraArgs>(extra_args)...
				), mod_rm.Reg.h_l);
		}
	};*/

	if (!mod_rm.rm.disp_size && !mod_rm.rm.reg_set) return;
	if (!ctx->p_sib) {
		if (!mod_rm.rm.is_addr && mod_rm.rm.reg_set) {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.SetReg<OPtype16>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype16>(op1),
					static_cast<OPtype16>(static_cast<OP2type16>(op2)),
					extra_args...
				), mod_rm.Reg.h_l);
				break;
			case OperandSize::X86_Osize_32bit:
				emu.SetReg<OPtype32>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OP2type32>(op1),
					static_cast<OPtype32>(static_cast<OP2type32>(op2)),
					extra_args...), mod_rm.Reg.h_l);
				break;
			case OperandSize::X86_Osize_64bit:
				emu.SetReg<OPtype64>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype64>(op1),
					static_cast<OPtype64>(static_cast<OP2type64>(op2)),
					extra_args...), mod_rm.Reg.h_l);
				break;
			default:
				break;
			}
		}
		else if (!mod_rm.rm.disp_size) {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.SetReg<OPtype16>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype16>(op1),
					static_cast<OPtype16>(emu.memory.Read<OP2type16>(static_cast<OP2type16>(op2)).value()),
					extra_args...), mod_rm.Reg.h_l);
				break;
			case OperandSize::X86_Osize_32bit:
				emu.SetReg<OPtype32>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype32>(op1),
					static_cast<OPtype32>(emu.memory.Read<OP2type32>(static_cast<OP2type32>(op2)).value()),
					extra_args...), mod_rm.Reg.h_l);
				break;
			case OperandSize::X86_Osize_64bit:
				emu.SetReg<OPtype64>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype64>(op1),
					static_cast<OPtype64>(emu.memory.Read<OP2type64>(static_cast<OP2type64>(op2)).value()),
					extra_args...), mod_rm.Reg.h_l);
				break;
			default:
				break;
			}
		}
		else if (mod_rm.rm.reg_set && mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(2)).value();
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.SetReg<OPtype16>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype16>(op1),
					static_cast<OPtype16>(emu.memory.Read<OP2type16>(static_cast<OP2type16>(op2) + disp).value()),
					extra_args...), mod_rm.Reg.h_l);
				break;
			case OperandSize::X86_Osize_32bit:
				emu.SetReg<OPtype32>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype32>(op1),
					static_cast<OPtype32>(emu.memory.Read<OP2type32>(static_cast<OP2type32>(op2) + disp).value()),
					extra_args...), mod_rm.Reg.h_l);
				break;
			case OperandSize::X86_Osize_64bit:
				emu.SetReg<OPtype64>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype64>(op1),
					static_cast<OPtype64>(emu.memory.Read<OP2type64>(static_cast<OP2type64>(op2) + disp).value()),
					extra_args...), mod_rm.Reg.h_l);
				break;
			default:
				break;
			}
		}
		else if (mod_rm.rm.disp_size) {
			s64 disp = emu.Reg(Register::Rip) + read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(2)).value() + inst.size();
			std::cout << "RM instruction: Accessing disp32: " << std::hex << disp << "\n";

			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.SetReg<OPtype16>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype16>(op1),
					static_cast<OPtype16>(emu.memory.Read<OP2type16>(disp).value()),
					extra_args...), mod_rm.Reg.h_l);
				break;
			case OperandSize::X86_Osize_32bit:
				emu.SetReg<OPtype32>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype32>(op1),
					static_cast<OPtype32>(emu.memory.Read<OP2type32>(disp).value()),
					extra_args...), mod_rm.Reg.h_l);
				break;
			case OperandSize::X86_Osize_64bit:
				emu.SetReg<OPtype64>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype64>(op1),
					static_cast<OPtype64>(emu.memory.Read<OP2type64>(disp).value()),
					extra_args...), mod_rm.Reg.h_l);
				break;
			default:
				break;
			}
		}
	}																																																			\
	else {
		std::cout << "Entering SIB procession...\n";
		Sib sib_byte;
		u64 calc_offset = 0;
		set_sib_byte(emu, ctx, mod_rm, sib_byte, calc_offset);
		if (!sib_byte.valid)
			return;

		if (mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(3)).value();
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.SetReg<OPtype16>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype16>(op1),
					static_cast<OPtype16>(emu.memory.Read<OP2type16>(calc_offset + disp).value()),
					extra_args...
				));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.SetReg<OPtype32>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype32>(op1),
					static_cast<OPtype32>(emu.memory.Read<OP2type32>(calc_offset + disp).value()),
					extra_args...
				));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.SetReg<OPtype64>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype64>(op1),
					static_cast<OPtype64>(emu.memory.Read<OP2type64>(calc_offset + disp).value()),
					extra_args...
				));
				break;
			default:
				break;
			}
		}																																																					   \
		else {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.SetReg<OPtype16>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype16>(op1),
					static_cast<OPtype16>(emu.memory.Read<OP2type16>(calc_offset).value()),
					extra_args...
				));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.SetReg<OPtype32>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype32>(op1),
					static_cast<OPtype32>(emu.memory.Read<OP2type32>(calc_offset).value()),
					extra_args...
				));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.SetReg<OPtype64>(mod_rm.Reg.reg, call_function(
					instr_emu_func,
					static_cast<OPtype64>(op1),
					static_cast<OPtype64>(emu.memory.Read<OP2type64>(calc_offset).value()),
					extra_args...
				));
				break;
			default:
				break;
			}
		}
	}
	return;
}

enum class AddressMode : u8 {
	Access = 0,  // Access the value at the address (e.g., MOV)
	AsValue,     // Use the address itself as a value (e.g., LEA)
};
//op2 in here will be automatically placed by the offset, so we will only needs to be dealing with op1
template <typename FuncType,
	std::integral OPtype16, std::integral OPtype32, std::integral OPtype64,
	std::integral Immtype16, std::integral Immtype32, std::integral Immtype64,
	AddressMode amod = AddressMode::Access,
	typename... ExtraArgs>
void def_instruction_op2_MI(
	Emulator& emu,
	x86Dcctx* ctx,
	const std::vector<u8>& inst,
	FuncType instr_emu_func,
	ModRM& mod_rm,
	u64 op1,
	u32 offset_to_imm,
	ExtraArgs... extra_args)
{
	OperandSize opsize = ctx->osize;
	if (!mod_rm.rm.disp_size && !mod_rm.rm.reg_set)
		return;

	auto _offset_to_imm = offset_to_imm + (mod_rm.rm.disp_size / 8);
	std::cout << "\nMI instruction: processing IMM: " << std::hex << (s64)(read_from_vec<u8>(inst, _offset_to_imm)) << "\n";

	auto execute_instruction_reg = [&](auto OperandType, auto ImmType) {
		if constexpr (std::is_void_v<FuncType>) {
			call_function(
				instr_emu_func,
				static_cast<decltype(OperandType)>(op1),
				static_cast<decltype(OperandType)>(read_from_vec<decltype(ImmType)>(inst, _offset_to_imm)),
				std::forward<ExtraArgs>(extra_args)...);
		}
		else {
			emu.SetReg<decltype(OperandType)>(mod_rm.rm.reg,
				call_function(
					instr_emu_func,
					static_cast<decltype(OperandType)>(op1),
					static_cast<decltype(OperandType)>(read_from_vec<decltype(ImmType)>(inst, _offset_to_imm)),
					std::forward<ExtraArgs>(extra_args)...));
		}
	};

	auto execute_instruction_mem = [&](auto OperandType, auto ImmType, auto Op1, auto disp) {

		u64 _Op1 = 0;
		if constexpr (amod == AddressMode::Access)
			_Op1 = emu.memory.Read<decltype(OperandType)>(static_cast<decltype(OperandType)>(Op1) + disp).value();
		else
			_Op1 = static_cast<decltype(OperandType)>(Op1) + disp;

		if constexpr (std::is_void_v<FuncType>) {
			call_function(
				instr_emu_func,
				_Op1,
				static_cast<decltype(OperandType)>(read_from_vec<decltype(ImmType)>(inst, _offset_to_imm)),
				std::forward<ExtraArgs>(extra_args)...);
		}
		else {
			emu.memory.WriteFrom(static_cast<decltype(OperandType)>(Op1) + disp,
				to_byte_vec(
					call_function(
						instr_emu_func,
						_Op1,
						static_cast<decltype(OperandType)>(read_from_vec<decltype(ImmType)>(inst, _offset_to_imm)),
						std::forward<ExtraArgs>(extra_args)...)
				));
		}
	};
	

	if (!ctx->p_sib) {
		if (!mod_rm.rm.is_addr && mod_rm.rm.reg_set) {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				execute_instruction_reg(OPtype16{}, Immtype16{});
				break;
			case OperandSize::X86_Osize_32bit:
				execute_instruction_reg(OPtype32{}, Immtype32{});
				break;
			case OperandSize::X86_Osize_64bit:
				execute_instruction_reg(OPtype64{}, Immtype64{});
				break;
			default:
				std::cout << "MI: Unknown opsizen";
				break;
			}
		}
		else if (!mod_rm.rm.disp_size && mod_rm.rm.reg_set) {
			std::cout << "MI: Reading the op data...\n";
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				execute_instruction_mem(OPtype16{}, Immtype16{}, op1, 0);
				break;
			case OperandSize::X86_Osize_32bit:
				execute_instruction_mem(OPtype32{}, Immtype32{}, op1, 0);
				break;
			case OperandSize::X86_Osize_64bit:
				execute_instruction_mem(OPtype64{}, Immtype64{}, op1, 0);
				break;
			default:
				break;
			}
		}
		else if (mod_rm.rm.reg_set && mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, 2).value();

			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				execute_instruction_mem(OPtype16{}, Immtype16{}, op1, disp);
				break;
			case OperandSize::X86_Osize_32bit:
				execute_instruction_mem(OPtype32{}, Immtype32{}, op1, disp);
				break;
			case OperandSize::X86_Osize_64bit:
				execute_instruction_mem(OPtype64{}, Immtype64{}, op1, disp);
				break;
			default:
				break;
			}
		}
		/*Only displacement*/
		else if (!mod_rm.rm.reg_set && mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, 2).value();

			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				execute_instruction_mem(OPtype16{}, OPtype16{}, 0, disp);
				break;
			case OperandSize::X86_Osize_32bit:
				execute_instruction_mem(OPtype32{}, OPtype32{}, 0, disp);
				break;
			case OperandSize::X86_Osize_64bit:
				execute_instruction_mem(OPtype64{}, OPtype64{}, 0, disp);
				break;
			default:
				break;
			}
		}
	}
	else {
		Sib sib_byte;
		u64 calc_offset = 0;
		set_sib_byte(emu, ctx, mod_rm, sib_byte, calc_offset);
		if (!sib_byte.valid)
			return;

		_offset_to_imm += 1;

		if (mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(3)).value();

			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				execute_instruction_mem(OPtype16{}, Immtype16{}, 0, calc_offset + disp);
				break;
			case OperandSize::X86_Osize_32bit:
				execute_instruction_mem(OPtype32{}, Immtype16{}, 0, calc_offset + disp);
				break;
			case OperandSize::X86_Osize_64bit:
				execute_instruction_mem(OPtype64{}, Immtype64{}, 0, calc_offset + disp);
				break;
			}
		}
		else {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				execute_instruction_mem(OPtype16{}, Immtype16{}, 0, calc_offset);
				break;
			case OperandSize::X86_Osize_32bit:
				execute_instruction_mem(OPtype32{}, Immtype32{}, 0, calc_offset);
				break;
			case OperandSize::X86_Osize_64bit:
				execute_instruction_mem(OPtype16{}, Immtype16{}, 0, calc_offset);
				break;
			}
		}
	}
	return;
}

template <typename FuncType,
	std::integral Immtype16, std::integral Immtype32, std::integral Immtype64,
	typename... ExtraArgs>
void def_instruction_op2_I(Emulator& emu,
	x86Dcctx* ctx,
	const std::vector<u8>& inst,
	FuncType instr_emu_func,
	u32 offset_to_imm,
	ExtraArgs... extra_args) {

	auto execute_instruction_reg = [&](auto OperandType, auto ImmType) {
		if constexpr (std::is_void_v<FuncType>) {
			call_function(
				instr_emu_func,
				static_cast<decltype(OperandType)>(emu.Reg(Register::Rax)),
				static_cast<decltype(OperandType)>(read_from_vec<decltype(ImmType)>(inst, offset_to_imm)),
				std::forward<ExtraArgs>(extra_args)...);
		}
		else {
			emu.SetReg<decltype(OperandType)>(Register::Rax,
				call_function(
					instr_emu_func,
					static_cast<decltype(OperandType)>(emu.Reg(Register::Rax)),
					static_cast<decltype(OperandType)>(read_from_vec<decltype(ImmType)>(inst, offset_to_imm)),
					std::forward<ExtraArgs>(extra_args)...));
		}
	};

	switch (GET_OPSIZE_ENUM(ctx->osize)) {
	case OperandSize::X86_Osize_16bit:
		execute_instruction_reg(u16{}, Immtype16{});
		break;
	case OperandSize::X86_Osize_32bit:
		execute_instruction_reg(u32{}, Immtype32{});
		break;
	case OperandSize::X86_Osize_64bit:
		execute_instruction_reg(u64{}, Immtype64{});
		break;
	default:
		std::cout << "MI: Unknown opsize";
		break;
	}
	return;
}


template <typename FuncType,
	std::integral OPtype16, std::integral OPtype32, std::integral OPtype64,
	std::integral OP2type16, std::integral OP2type32, std::integral OP2type64,
	typename... ExtraArgs>
void def_instruction_op2_MR(Emulator& emu,
	x86Dcctx* ctx,
	const std::vector<u8>& inst,
	FuncType instr_emu_func,
	ModRM& mod_rm,
	u64 op1,
	u64 op2,
	ExtraArgs... extra_args)
{
	if (!mod_rm.rm.disp_size && !mod_rm.rm.reg_set)
		return;

	if (!ctx->p_sib) {
		if (!mod_rm.rm.is_addr && mod_rm.rm.reg_set) {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.SetReg<OPtype16>(mod_rm.rm.reg, call_function(
					instr_emu_func,
					static_cast<OPtype16>(op1),
					static_cast<OPtype16>(static_cast<OP2type16>(op2)),
					extra_args...
				), mod_rm.rm.h_l);
				break;
			case OperandSize::X86_Osize_32bit:
				emu.SetReg<OPtype64>(mod_rm.rm.reg, call_function(
					instr_emu_func,
					static_cast<OPtype32>(op1),
					static_cast<OPtype32>(static_cast<OP2type32>(op2)),
					extra_args...
				), mod_rm.rm.h_l);
				break;
			
			case OperandSize::X86_Osize_64bit:
				emu.SetReg<OPtype64>(mod_rm.rm.reg, call_function(
					instr_emu_func,
					static_cast<OPtype64>(op1),
					static_cast<OPtype64>(static_cast<OP2type64>(op2)),
					extra_args...
				), mod_rm.rm.h_l);
				break;
			default:
				break;
			}
		}
		else if (!mod_rm.rm.disp_size) {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(static_cast<OPtype16>(mod_rm.rm.reg_val),
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype16>(static_cast<OPtype16>(op1)).value(),
						static_cast<OPtype16>(static_cast<OP2type16>(op2)),
						extra_args...
					)));
				break;

			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(static_cast<OPtype32>(mod_rm.rm.reg_val),
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype32>(static_cast<OPtype32>(op1)).value(),
						static_cast<OPtype32>(static_cast<OP2type32>(op2)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(mod_rm.rm.reg_val, to_byte_vec(call_function(
					instr_emu_func,
					emu.memory.Read<OPtype64>(static_cast<OPtype64>(op1)).value(),
					static_cast<OPtype64>(static_cast<OP2type64>(op2)),
					extra_args...
				)));
				break;
			default:
				break;
			}
		}
		else if (mod_rm.rm.reg_set && mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(2)).value();								\
			
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(static_cast<OPtype16>(mod_rm.rm.reg_val) + disp, 
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype16>(static_cast<OPtype16>(op1) + disp).value(),
						static_cast<OPtype16>(static_cast<OP2type16>(op2)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(static_cast<OPtype32>(mod_rm.rm.reg_val) + disp, 
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype32>(static_cast<OPtype32>(op1) + disp).value(),
						static_cast<OPtype32>(static_cast<OP2type32>(op2)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(static_cast<OPtype64>(mod_rm.rm.reg_val) + disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype64>(static_cast<OPtype64>(op1) + disp).value(),
						static_cast<OPtype64>(static_cast<OP2type64>(op2)),
						extra_args...
					)));
				break;
			default:
				break;
			}
		}
		else if (mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(2)).value();								\

			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype16>(disp).value(),
						static_cast<OPtype16>(static_cast<OP2type16>(op2)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype32>(disp).value(),
						static_cast<OPtype32>(static_cast<OP2type32>(op2)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype64>(disp).value(),
						static_cast<OPtype64>(static_cast<OP2type64>(op2)),
						extra_args...
					)));
				break;
			default:
				break;
			}
		}
	}																										\
	else {																									\
		Sib sib_byte;
		u64 calc_offset = 0;
		set_sib_byte(emu, ctx, mod_rm, sib_byte, calc_offset);

		if (!sib_byte.valid)
			return;	

		if (mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(3)).value();

			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(calc_offset + disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype16>(calc_offset + disp).value(),
						static_cast<OPtype16>(static_cast<OP2type16>(op2)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(calc_offset + disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype32>(calc_offset + disp).value(),
						static_cast<OPtype32>(static_cast<OP2type32>(op2)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(calc_offset + disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype64>(calc_offset + disp).value(),
						static_cast<OPtype64>(static_cast<OP2type64>(op2)),
						extra_args...
					)));
				break;
			default:
				break;
			}
		}
		else {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(calc_offset,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype16>(calc_offset).value(),
						static_cast<OPtype16>(static_cast<OP2type16>(op2)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(calc_offset,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype32>(calc_offset).value(),
						static_cast<OPtype32>(static_cast<OP2type32>(op2)),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(calc_offset,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype64>(calc_offset).value(),
						static_cast<OPtype64>(static_cast<OP2type64>(op2)),
						extra_args...
					)));
				break;
			default:
				break;
			}
		}
	}
	return;
}

template <typename FuncType,
	std::integral OPtype16, std::integral OPtype32, std::integral OPtype64,
	typename... ExtraArgs>
void def_instruction_op1_M(Emulator& emu,
	x86Dcctx* ctx,
	const std::vector<u8>& inst,
	FuncType instr_emu_func,
	ModRM& mod_rm,
	u64 op1,
	ExtraArgs... extra_args) {

	if (!mod_rm.rm.disp_size && !mod_rm.rm.reg_set)
		return;

	if (!ctx->p_sib) {
		if (!mod_rm.rm.is_addr && mod_rm.rm.reg_set) {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.SetReg<OPtype16>(mod_rm.rm.reg, call_function(
					instr_emu_func,
					static_cast<OPtype16>(op1),
					extra_args...
				), mod_rm.rm.h_l);
				break;
			case OperandSize::X86_Osize_32bit:
				emu.SetReg<OPtype64>(mod_rm.rm.reg, call_function(
					instr_emu_func,
					static_cast<OPtype32>(op1),
					extra_args...
				), mod_rm.rm.h_l);
				break;

			case OperandSize::X86_Osize_64bit:
				emu.SetReg<OPtype64>(mod_rm.rm.reg, call_function(
					instr_emu_func,
					static_cast<OPtype64>(op1),
					extra_args...
				), mod_rm.rm.h_l);
				break;
			default:
				break;
			}
		}
		else if (!mod_rm.rm.disp_size) {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(static_cast<OPtype16>(mod_rm.rm.reg_val),
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype16>(static_cast<OPtype16>(op1)).value(),
						extra_args...
					)));
				break;

			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(static_cast<OPtype32>(mod_rm.rm.reg_val),
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype32>(static_cast<OPtype32>(op1)).value(),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(mod_rm.rm.reg_val, to_byte_vec(call_function(
					instr_emu_func,
					emu.memory.Read<OPtype64>(static_cast<OPtype64>(op1)).value(),
					extra_args...
				)));
				break;
			default:
				break;
			}
		}
		else if (mod_rm.rm.reg_set && mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(2)).value();								\

				switch (GET_OPSIZE_ENUM(ctx->osize)) {
				case OperandSize::X86_Osize_16bit:
					emu.memory.WriteFrom(static_cast<OPtype16>(mod_rm.rm.reg_val) + disp,
						to_byte_vec(call_function(
							instr_emu_func,
							emu.memory.Read<OPtype16>(static_cast<OPtype16>(op1) + disp).value(),
							extra_args...
						)));
					break;
				case OperandSize::X86_Osize_32bit:
					emu.memory.WriteFrom(static_cast<OPtype32>(mod_rm.rm.reg_val) + disp,
						to_byte_vec(call_function(
							instr_emu_func,
							emu.memory.Read<OPtype32>(static_cast<OPtype32>(op1) + disp).value(),
							extra_args...
						)));
					break;
				case OperandSize::X86_Osize_64bit:
					emu.memory.WriteFrom(static_cast<OPtype64>(mod_rm.rm.reg_val) + disp,
						to_byte_vec(call_function(
							instr_emu_func,
							emu.memory.Read<OPtype64>(static_cast<OPtype64>(op1) + disp).value(),
							extra_args...
						)));
					break;
				default:
					break;
				}
		}
		else if (mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(2)).value();

			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype16>(disp).value(),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype32>(disp).value(),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype64>(disp).value(),
						extra_args...
					)));
				break;
			default:
				break;
			}
		}
	}																										\
	else {
		Sib sib_byte;
		u64 calc_offset = 0;
		set_sib_byte(emu, ctx, mod_rm, sib_byte, calc_offset);

		if (!sib_byte.valid)
			return;

		if (mod_rm.rm.disp_size) {
			s64 disp = read_disp_from_inst<s64>(inst, mod_rm.rm.disp_size, INSTR_POS(3)).value();

			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(calc_offset + disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype16>(calc_offset + disp).value(),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(calc_offset + disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype32>(calc_offset + disp).value(),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(calc_offset + disp,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype64>(calc_offset + disp).value(),
						extra_args...
					)));
				break;
			default:
				break;
			}
		}
		else {
			switch (GET_OPSIZE_ENUM(ctx->osize)) {
			case OperandSize::X86_Osize_16bit:
				emu.memory.WriteFrom(calc_offset,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype16>(calc_offset).value(),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_32bit:
				emu.memory.WriteFrom(calc_offset,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype32>(calc_offset).value(),
						extra_args...
					)));
				break;
			case OperandSize::X86_Osize_64bit:
				emu.memory.WriteFrom(calc_offset,
					to_byte_vec(call_function(
						instr_emu_func,
						emu.memory.Read<OPtype64>(calc_offset).value(),
						extra_args...
					)));
				break;
			default:
				break;
			}
		}
	}
	return;
}