
#include <iostream>

#include "Emulator.hpp"

#include "../core/Fs.hpp"
#include "../emulation/x86/ModRM.hpp"
#include "../emulation/x86/InstructionHandler.hpp"

#define EMU_CHECK_OP_SIZE(n) if (!lendec.GetDecoderCtx().pfx_p_osize || lendec.GetDecoderCtx().osize != ##n) \
break

bool Emulator::LoadExecutable(const std::string& filename, const std::vector<Section>& sections) {

	//Read the input file
	std::vector<u8> contents = read_file(filename);

	//Go thru each section and load it
	for (const auto& section : sections) {
		//Set memory to writable
		memory.SetPermission(section.virt_addr, section.mem_size, PERM_WRITE);

		//Write in th eoriginal file contents 
		memory.WriteFrom(section.virt_addr, get_range(contents, section.file_off, section.file_size));

		//Write in any paddings with zeros
		if (section.mem_size > section.file_size) [[likely]] {
			std::vector<u8> padding(section.mem_size - section.file_size);
			memory.WriteFrom(section.virt_addr + section.file_size, padding);
		}

		//Demote permissions to originals
		memory.SetPermission(section.virt_addr, section.mem_size, section.permission);

		//Update the allocator beyond any sections we load
		memory.cur_alloc = std::max(
			memory.cur_alloc, 
			(section.virt_addr + section.mem_size + 0xf) & ~0xf
		);
	}

	return true;
}

u64 Emulator::Reg(Register reg) {
	return registers[reg];
}

void Emulator::SetReg(Register reg, u64 val) {
	registers[reg] = val;
}

void Emulator::Run() {
	//Fetch the current instructions
	Ldasm lendec;

	while(true) {
		auto pc = Reg(Register::Rip);
		std::vector<u8> inst;

		memory.ReadInstruction(lendec, pc, inst, (PERM_READ | PERM_EXEC));
		
		u32 opcode;//will opcode length variable?? we'll find out

		u8 modrm;
		if (lendec.GetDecoderCtx().p_modrm) {
			if (lendec.GetDecoderCtx().pfx_p_rex)
				inst.erase(inst.begin()); //exclude the rex byte, we will get it using the decode context

			std::copy(inst.begin(), inst.begin() + lendec.GetDecoderCtx().pos_modrm, &opcode);
			modrm = lendec.GetDecoderCtx().modrm;
		}
		else
			std::copy(inst.begin(), inst.begin() + 1, &opcode); //then we'll assume the opcode is only one bytes


		//Start to emulate instructions
		switch (opcode) {
/*======================= ADD instruction(0x01) ========================*/
		case Instruction::ADD_01:
			EMU_CHECK_OP_SIZE(2);
			
			Add_01(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= ADD instruction(0x03) ========================*/
		case Instruction::ADD_03:
			EMU_CHECK_OP_SIZE(2);

			Add_03(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Logical Or(0x09) ===================================*/
		case Instruction::OR_09:
			EMU_CHECK_OP_SIZE(2);

			Or_09(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Logical Or(0x0C) ===================================*/
		case Instruction::OR_0C:
			EMU_CHECK_OP_SIZE(2);

			SetReg(Register::Rax, GET_L_REG(Reg(Register::Rax)) | ReadFromVec<u8>(inst, 1));
			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) | ReadFromVec<u8>(inst, 1));
			break;
/*======================= Logical Or(0x0D) ===================================*/
		case Instruction::OR_0D:
			EMU_CHECK_OP_SIZE(2);

			Or_0D(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Logical And(0x24) ==================================*/
		case Instruction::AND_21:
			EMU_CHECK_OP_SIZE(2);

			And_21(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Logical And(0x24) ==================================*/
		case Instruction::AND_24:
			EMU_CHECK_OP_SIZE(2);

			SetReg(Register::Rax, GET_L_REG(Reg(Register::Rax)) & ReadFromVec<u8>(inst, 1));
			SetReg(Register::Rax, GET_L_REG(Reg(Register::Rax)) & ReadFromVec<u8>(inst, 1));
			break;
/*======================= Logical And(0x25) ==================================*/
		case Instruction::AND_25:
			EMU_CHECK_OP_SIZE(2);

			And_25(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Subtract(0x29) =====================================*/
		case Instruction::SUB_29:
			EMU_CHECK_OP_SIZE(2);

			Sub_29(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Subtract(0x2B) =====================================*/
		case Instruction::SUB_2B:
			EMU_CHECK_OP_SIZE(2);

			Sub_2B(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Subtract(0x2C) =====================================*/
		case Instruction::SUB_2C:
			EMU_CHECK_OP_SIZE(2);

			SetReg(Register::Rax, SubAndSetFlags(flags, GET_L_REG(Reg(Register::Rax)), ReadFromVec<u8>(inst, 1)));
			break;
/*======================= Subtract(0x2D) =====================================*/
		case Instruction::SUB_2D:
			EMU_CHECK_OP_SIZE(2);

			if (lendec.GetDecoderCtx().osize == X86_Osize_16bit && inst.size() == 3)
				SetReg(Register::Rax, SubAndSetFlags(flags, GET_X_REG(Reg(Register::Rax)), ReadFromVec<u16>(inst, 1)));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_32bit && inst.size() == 5)
				SetReg(Register::Rax, SubAndSetFlags(flags, GET_EXT_REG(Reg(Register::Rax)), ReadFromVec<u32>(inst, 1)));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_64bit && inst.size() == 5)
				SetReg(Register::Rax, SubAndSetFlags(flags, Reg(Register::Rax), static_cast<u64>(ReadFromVec<u32>(inst, 1))));
			break;
/*======================= Exclusive or operation(0x31) =======================*/
		case Instruction::XOR_31:
			EMU_CHECK_OP_SIZE(2);

			Xor_31(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Exclusive or operation(0x33) =======================*/
		case Instruction::XOR_33:
			EMU_CHECK_OP_SIZE(2);

			Xor_33(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Exclusive or operation(0x34) =======================*/
		case Instruction::XOR_34:
			EMU_CHECK_OP_SIZE(2);

			SetReg(Register::Rax, GET_L_REG(Reg(Register::Rax)) ^ ReadFromVec<u8>(inst, 1));
			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) ^ ReadFromVec<u8>(inst, 1));
			break;
/*======================= Exclusive or operation(0x35) =======================*/
		case Instruction::XOR_35:
			EMU_CHECK_OP_SIZE(2);

			Xor_35(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::_81:
			EMU_CHECK_OP_SIZE(2);

			switch (MODRM_REG(lendec.GetDecoderCtx().modrm))
			{
			case 0x00:
				Add_81(*this, &lendec.GetDecoderCtx(), inst);
				break;
			default:
				break;
			}
		case Instruction::_83:
			EMU_CHECK_OP_SIZE(2);

			switch (MODRM_REG(lendec.GetDecoderCtx().modrm))
			{
			case 0x00:
				Add_83(*this, &lendec.GetDecoderCtx(), inst);
				break;
			case 0x05:
				Sub_83(*this, &lendec.GetDecoderCtx(), inst);
				break;
			default:
				break;
			}
			break;
/*======================= No operation instruction =======================*/
		case Instruction::NOP:
			break;
/*======================= Test(0xA8) ===========================================*/
		case Instruction::TEST_A8:
			EMU_CHECK_OP_SIZE(2);

			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) & ReadFromVec<u8>(inst, 1));
			break;
/*======================= Test(0xA9) ===========================================*/
		case Instruction::TEST_A9:
			EMU_CHECK_OP_SIZE(2);

			if (lendec.GetDecoderCtx().osize == X86_Osize_16bit && inst.size() == 3)
				SetLogicOpFlags(flags, GET_X_REG(Reg(Register::Rax)) & ReadFromVec<u16>(inst, 1));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_32bit && inst.size() == 5)
				SetLogicOpFlags(flags, GET_EXT_REG(Reg(Register::Rax)) & ReadFromVec<u32>(inst, 1));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_64bit && inst.size() == 5)
				SetLogicOpFlags(flags, Reg(Register::Rax) & static_cast<u64>(ReadFromVec<u32>(inst, 1)));
			break;
/*======================= Move instruction(0xB8) ===============================*/
		case Instruction::MOV_B8:
			EMU_CHECK_OP_SIZE(2);

			if (lendec.GetDecoderCtx().osize == X86_Osize_16bit && inst.size() == 3)
				SetReg(Register::Rax, ReadFromVec<u16>(inst, 1));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_32bit && inst.size() == 5)
				SetReg(Register::Rax, ReadFromVec<u32>(inst, 1));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_64bit && inst.size() == 9)
				SetReg(Register::Rax, ReadFromVec<u64>(inst, 1));
			break;
/*======================= Move instruction(0x8B) ===============================*/
		case Instruction::MOV_8B:
			EMU_CHECK_OP_SIZE(2);

			Mov_8B(*this, &lendec.GetDecoderCtx(), inst);
			break;
		}
/*========================================================================*/

		//Increment the rip to get next instruction
		SetReg(Register::Rip, pc + inst.size());
	}
}

void Emulator::Reset(const Emulator& other) {
	//Reset memory state
	memory.Reset(other.memory);
	registers = other.registers;
}