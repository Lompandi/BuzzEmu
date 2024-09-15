
#include <iostream>

#include "Emulator.hpp"

#include "VmExit.hpp"
#include "../core/Fs.hpp"
#include "../emulation/x86/ModRM.hpp"
#include "../emulation/x86/InstructionHandler.hpp"

//#define EMU_CHECK_OP_SIZE(n) if (!lendec.GetDecoderCtx().pfx_p_osize || lendec.GetDecoderCtx().osize != ##n) \
//break
//use this aftrer jump to not affect the pc 
#define jmp_cleanup inst.clear(); \
erased = 0

//TUTORIAL AT: https://youtu.be/iM3s8-umRO0?t=28749 

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

void Emulator::SetReg64(Register reg, u64 val) {
	registers[reg] = val;
}

void DEBUG_FUNC CrashDump(Emulator& emu) {
	//dump general pourpose registers
	std::cout << "RAX: 0x" << std::hex << emu.Reg(Register::Rax);
	std::cout << " RCX: 0x" << std::hex << emu.Reg(Register::Rcx) << "\n";
	std::cout << "RDX: 0x" << std::hex << emu.Reg(Register::Rdx);
	std::cout << " RBX: 0x" << std::hex << emu.Reg(Register::Rbx) << "\n";
	std::cout << "RSP: 0x" << std::hex << emu.Reg(Register::Rsp);
	std::cout << " RBP: 0x" << std::hex << emu.Reg(Register::Rbp) << "\n";
	std::cout << "RSI: 0x" << std::hex << emu.Reg(Register::Rsi);
	std::cout << " RDI: 0x" << std::hex << emu.Reg(Register::Rdi) << "\n\n";

	//dump Cpu flags
	std::cout << "CF: " << std::hex <<emu.flags.CF; // Carry Flag
	std::cout << " PF: " << std::hex <<emu.flags.PF; // Parity Flag
	std::cout << " AF: " << std::hex <<emu.flags.AF; // Auxiliary Carry Flag
	std::cout << " ZF: " << std::hex <<emu.flags.ZF << "\n"; // Zero Flag
	std::cout << "SF: " << std::hex <<emu.flags.SF; // Sign Flag
	std::cout << " TF: " << std::hex <<emu.flags.TF; // Trap Flag
	std::cout << " IF: " << std::hex <<emu.flags.IF; // Interrupt Enable Flag
	std::cout << " DF: " << std::hex <<emu.flags.DF << "\n"; // Direction Flag
	std::cout << "OF: " << std::hex <<emu.flags.OF << "\n\n"; // Overflow Flag
}

//TODO: jump currently only increment the offset by rel8 from current pc, 
//but i need to change it to match pc + instruction length + rel8
VmExit Emulator::Run() {
	Ldasm lendec;

	u64 debug_instr_count = 0;

	while(true) {
		CrashDump(*this);

		auto pc = Reg(Register::Rip);
		std::cout << "[EMU] Executing instruction at 0x" << std::hex << pc;
		std::vector<u8> inst;
		//Fetch the current instructions
		memory.ReadInstruction(lendec, pc, inst, (PERM_READ | PERM_EXEC));
		
		u32 opcode;
		u8 modrm;
		if (lendec.GetDecoderCtx().pfx_p_rex)
			std::cout << " prefixed REX";
		
		if (lendec.GetDecoderCtx().p_sib)
			std::cout << " have SIB";

		std::copy(inst.begin() + lendec.GetDecoderCtx().pos_opcode,
			inst.begin() + lendec.GetDecoderCtx().pos_opcode + lendec.GetDecoderCtx().opcode_size, 
			&opcode); //copy the opcode  

		std::cout << " with opcode 0x" << std::hex << opcode << "\n";

		//Start to emulate instructions
		switch (opcode) {
/*======================= ADD instruction(0x01) ========================*/
		case Instruction::ADD_01:
	
			Add_01(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= ADD instruction(0x03) ========================*/
		case Instruction::ADD_03:
			Add_03(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::ADD_05:
			Add_05(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Logical Or(0x09) ===================================*/
		case Instruction::OR_09:
			Or_09(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Logical Or(0x0C) ===================================*/
		case Instruction::OR_0C:
			SetReg<u8>(Register::Rax, GET_L_REG(Reg(Register::Rax)) | ReadFromVec<u8>(inst, 1));
			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) | ReadFromVec<u8>(inst, 1));
			break;
/*======================= Logical Or(0x0D) ===================================*/
		case Instruction::OR_0D:
			Or_0D(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Logical And(0x24) ==================================*/
		case Instruction::AND_21:
			
			And_21(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Logical And(0x24) ==================================*/
		case Instruction::AND_24:
			SetReg<u8>(Register::Rax, GET_L_REG(Reg(Register::Rax)) & ReadFromVec<u8>(inst, 1));
			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) & ReadFromVec<u8>(inst, 1));
			break;
/*======================= Logical And(0x25) ==================================*/
		case Instruction::AND_25:
			
			And_25(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Subtract(0x29) =====================================*/
		case Instruction::SUB_29:
			
			Sub_29(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Subtract(0x2B) =====================================*/
		case Instruction::SUB_2B:
			
			Sub_2B(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Subtract(0x2C) =====================================*/
		case Instruction::SUB_2C:
			SetReg<u8>(Register::Rax, SubAndSetFlags(GET_L_REG(Reg(Register::Rax)), ReadFromVec<u8>(inst, 1), flags));
			break;
/*======================= Subtract(0x2D) =====================================*/
		case Instruction::SUB_2D:
			if (lendec.GetDecoderCtx().osize == X86_Osize_16bit && inst.size() == 3)
				SetReg<u16>(Register::Rax, SubAndSetFlags(GET_X_REG(Reg(Register::Rax)), ReadFromVec<u16>(inst, 1), flags));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_32bit && inst.size() == 5)
				SetReg<u32>(Register::Rax, SubAndSetFlags(GET_EXT_REG(Reg(Register::Rax)), ReadFromVec<u32>(inst, 1), flags));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_64bit && inst.size() == 5)
				SetReg<u64>(Register::Rax, SubAndSetFlags(Reg(Register::Rax), static_cast<u64>(ReadFromVec<u32>(inst, 1)), flags));
			break;
/*======================= Exclusive or operation(0x31) =======================*/
		case Instruction::XOR_31:
			
			Xor_31(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Exclusive or operation(0x33) =======================*/
		case Instruction::XOR_33:
			
			Xor_33(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Exclusive or operation(0x34) =======================*/
		case Instruction::XOR_34:
			SetReg<u8>(Register::Rax, GET_L_REG(Reg(Register::Rax)) ^ ReadFromVec<u8>(inst, 1));
			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) ^ ReadFromVec<u8>(inst, 1));
			break;
/*======================= Exclusive or operation(0x35) =======================*/
		case Instruction::XOR_35:
			Xor_35(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Compare two operands (0x35) ========================*/
		case Instruction::CMP_3B:
			Cmp_3B(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*==================== Push onto the stack (0x50-0x57) =======================*/
		case Instruction::PUSH_50:
			Push_50_57(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::PUSH_51:
			Push_50_57(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::PUSH_52:
			Push_50_57(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::PUSH_53:
			Push_50_57(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::PUSH_54:
			Push_50_57(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::PUSH_55:
			Push_50_57(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::PUSH_56:
			Push_50_57(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::PUSH_57:
			Push_50_57(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Move with sign-extended (0x63) =====================*/
		case Instruction::MOVSXD_63:
			Movsxd_63(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Jump if zero (0x74) ================================*/
		case Instruction::JZ_74:
			//Signed-extended offset
			pc = Reg(Register::Rip) + (this->flags.ZF ? ReadFromVec<s8>(inst, 1) : 0);
			break;
		case Instruction::JNZ_75:
			pc = Reg(Register::Rip) + (this->flags.ZF ? 0 : ReadFromVec<s8>(inst, 1));
			break;
/*======================= Jump if less (0x7C) ================================*/
		case Instruction::JL_7C:
			//Signed-extended offset
			pc = Reg(Register::Rip) + (this->flags.SF != this->flags.OF ? ReadFromVec<s8>(inst, 1) : 0);
			std::cout << "JL Rel8: 0x" << std::hex << (int)ReadFromVec<s8>(inst, 1) << "\n";
			std::cout << "Pc after JL: 0x" << std::hex << pc << "\n";
			break;
		case Instruction::_81:
			switch (MODRM_REG(lendec.GetDecoderCtx().modrm))
			{
			case 0x00:
				Add_81(*this, &lendec.GetDecoderCtx(), inst);
				break;
			case 0x01:
				Or_81(*this, &lendec.GetDecoderCtx(), inst);
				break;
			case 0x04:
				And_81(*this, &lendec.GetDecoderCtx(), inst);
				break;
			case 0x05:
				Sub_81(*this, &lendec.GetDecoderCtx(), inst);
				break;
			case 0x06:
				Xor_81(*this, &lendec.GetDecoderCtx(), inst);
				break;
			default:
				break;
			}
			break;
		case Instruction::_83:
			switch (MODRM_REG(lendec.GetDecoderCtx().modrm))
			{
			case 0x00:
				Add_83(*this, &lendec.GetDecoderCtx(), inst);
				break;
			case 0x01:
				Or_83(*this, &lendec.GetDecoderCtx(), inst);
				break;
			case 0x04:
				And_83(*this, &lendec.GetDecoderCtx(), inst);
				break;
			case 0x05:
				Sub_83(*this, &lendec.GetDecoderCtx(), inst);
				break;
			case 0x06:
				Xor_83(*this, &lendec.GetDecoderCtx(), inst);
				break;
			case 0x07:
				Cmp_83(*this, &lendec.GetDecoderCtx(), inst);
				break;
			default:
				break;
			}
			break;
/*======================= Logical Compare =====================================*/
		case Instruction::TEST_84:
			Test_84(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::TEST_85:
			Test_85(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Move instruction		 ===============================*/
		case Instruction::MOV_88:
			Mov_88(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::MOV_89:
			Mov_89(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::MOV_8B:
			Mov_8B(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= No operation instruction =======================*/
		case Instruction::NOP:
			break;
/*======================= Test(0xA8) ===========================================*/
		case Instruction::TEST_A8:
			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) & ReadFromVec<u8>(inst, 1));
			break;
/*======================= Test(0xA9) ===========================================*/
		case Instruction::TEST_A9:
			if (lendec.GetDecoderCtx().osize == X86_Osize_16bit && inst.size() == 3)
				SetLogicOpFlags(flags, GET_X_REG(Reg(Register::Rax)) & ReadFromVec<u16>(inst, 1));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_32bit && inst.size() == 5)
				SetLogicOpFlags(flags, GET_EXT_REG(Reg(Register::Rax)) & ReadFromVec<u32>(inst, 1));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_64bit && inst.size() == 5)
				SetLogicOpFlags(flags, Reg(Register::Rax) & static_cast<u64>(ReadFromVec<u32>(inst, 1)));
			break;
/*======================= Move instruction(0xB8) ===============================*/
		case Instruction::MOV_B8:
			if (lendec.GetDecoderCtx().osize == X86_Osize_16bit && inst.size() == 3)
				SetReg<u16>(Register::Rax, ReadFromVec<u16>(inst, 1));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_32bit && inst.size() == 5)
				SetReg<u32>(Register::Rax, ReadFromVec<u32>(inst, 1));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_64bit && inst.size() == 9)
				SetReg<u64>(Register::Rax, ReadFromVec<u64>(inst, 1));
			break;
		case Instruction::RET_C3:
			std::cout << "Returning from procedure...\n";

			Ret_C3(*this, &lendec.GetDecoderCtx(), inst, pc);
			break;
/*============================ Call procedure (0xE8) ===========================*/
		case Instruction::CALL_E8:
			std::cout << "Calling functions\n";
			Call_E8(*this, &lendec.GetDecoderCtx(), inst, pc);
			break;
/*======================= Jump short        ====================================*/
		case Instruction::JMP_E9:
			Jmp_E9(*this, &lendec.GetDecoderCtx(), inst, pc);
			break;
		case Instruction::JMP_EB:
			pc = Reg(Register::Rip) + static_cast<s64>(ReadFromVec<s8>(inst, 1));
			break;
		case Instruction::_FF:
			switch (MODRM_REG(lendec.GetDecoderCtx().modrm))
			{
			case 0x02:
				Call_FF_reg2(*this, &lendec.GetDecoderCtx(), inst, pc);
				break;
			default:
				break;
			}
			break;
		case Instruction::SYSCALL_0F05:
			return VmExit::Syscall;
		default:
			std::cout << "\n[EMU] Error at 0x" << std::hex << pc << ", unknown opcode 0x" << std::hex << opcode << "\n";
			std::cout << "[EMU] " << std::dec << debug_instr_count << " instructions executed before crashing.\n\n";
			CrashDump(*this);
			break;
		}
/*========================================================================*/
		//Increment the rip to get next instruction
		debug_instr_count++;
		SetReg<u64>(Register::Rip, pc + inst.size());
	}
}


//Also, these are safty checked version for the opcode copying..., add it back to the poriginal code
void Emulator::TestRun() {
	Ldasm lendec;
	u64 debug_instr_count = 0;
	u64 pc = 0;
	u32 opcode = 0;

	while (true) {
		if (pc > 0x1243) break;

		pc = Reg(Register::Rip);

		std::vector<u8> inst;
		// Fetch the current instructions
		memory.ReadInstruction(lendec, pc, inst, (PERM_READ | PERM_EXEC));

		// Extract context information
		auto pos_opcode = lendec.GetDecoderCtx().pos_opcode;
		auto opcode_size = lendec.GetDecoderCtx().opcode_size;

		// Copy the opcode from the instruction vector
		std::memcpy(&opcode, inst.data() + pos_opcode, opcode_size);

		//std::cout << "0x" <<std::hex << pc << " with opcode 0x" << std::hex << opcode << "\n";

		//Start to emulate instructions
		switch (opcode) {
			/*======================= ADD instruction(0x01) ========================*/
		case Instruction::ADD_01:
			break;
			/*======================= ADD instruction(0x03) ========================*/
		case Instruction::ADD_03:
			break;
			/*======================= Logical Or(0x09) ===================================*/
		case Instruction::OR_09:
			break;
			/*======================= Logical Or(0x0C) ===================================*/
		case Instruction::OR_0C:
			break;
			/*======================= Logical Or(0x0D) ===================================*/
		case Instruction::OR_0D:
			break;
			/*======================= Logical And(0x24) ==================================*/
		case Instruction::AND_21:
			break;
			/*======================= Logical And(0x24) ==================================*/
		case Instruction::AND_24:
			break;
			/*======================= Logical And(0x25) ==================================*/
		case Instruction::AND_25:
			break;
			/*======================= Subtract(0x29) =====================================*/
		case Instruction::SUB_29:
			break;
			/*======================= Subtract(0x2B) =====================================*/
		case Instruction::SUB_2B:
			break;
			/*======================= Subtract(0x2C) =====================================*/
		case Instruction::SUB_2C:
			break;
			/*======================= Subtract(0x2D) =====================================*/
		case Instruction::SUB_2D:
			break;
			/*======================= Exclusive or operation(0x31) =======================*/
		case Instruction::XOR_31:
			break;
			/*======================= Exclusive or operation(0x33) =======================*/
		case Instruction::XOR_33:
			break;
			/*======================= Exclusive or operation(0x34) =======================*/
		case Instruction::XOR_34:
			break;
			/*======================= Exclusive or operation(0x35) =======================*/
		case Instruction::XOR_35:
			break;
		case Instruction::CMP_3B:
			break;
			/*==================== Push onto the stack (0x50-0x57) =======================*/
		case Instruction::PUSH_50:
			break;
		case Instruction::PUSH_51:
			break;
		case Instruction::PUSH_52:
			break;
		case Instruction::PUSH_53:
			break;
		case Instruction::PUSH_54:
			break;
		case Instruction::PUSH_55:
			break;
		case Instruction::PUSH_56:
			break;
		case Instruction::PUSH_57:
			break;
			/*======================= Move with sign-extended (0x63) =====================*/
		case Instruction::MOVSXD_63:
			break;
			/*======================= Jump if zero (0x74) ================================*/
		case Instruction::JZ_74:
			break;
		case Instruction::JNZ_75:
			break;
			/*======================= Jump if less (0x7C) ================================*/
		case Instruction::JL_7C:
			break;
		case Instruction::_81:
			break;
		case Instruction::_83:
			break;
		case 0x84:
			break;
			/*======================= Logical Compare (0x85) ===============================*/
		case Instruction::TEST_85:
			break;
		case 0x88:
			break;
			/*======================= Move instruction(0x89) ===============================*/
		case Instruction::MOV_89:
			break;
			/*======================= Move instruction(0x8B) ===============================*/
		case Instruction::MOV_8B:
			break;
			/*======================= No operation instruction =======================*/
		case Instruction::NOP:
			break;
			/*======================= Test(0xA8) ===========================================*/
		case Instruction::TEST_A8:
			break;
			/*======================= Test(0xA9) ===========================================*/
		case Instruction::TEST_A9:
			break;
		case 0xB0:
			break;
			/*======================= Move instruction(0xB8) ===============================*/
		case Instruction::MOV_B8:
			break;
		case Instruction::RET_C3:
			break;
		case 0xCC:
			break;
			/*============================ Call procedure (0xE8) ===========================*/
		case Instruction::CALL_E8:
			break;
			/*======================= Jump short (0xEB) ====================================*/
		case Instruction::JMP_EB:
			break;
		case Instruction::_FF:
			break;
		default:
			std::cout << "[EMU] Error at 0x" << std::hex << pc << ", unknown opcode 0x" << std::hex << opcode << "\n";
			//std::cout << "[EMU] " << std::dec << debug_instr_count << " instructions executed before crashing.\n\n";
			//CrashDump(*this);
			break;
		}
		/*========================================================================*/
				//Increment the rip to get next instruction
		debug_instr_count++;
		SetReg<u64>(Register::Rip, pc + inst.size());
	}
	return;
}
void Emulator::Reset(const Emulator& other) {
	//Reset memory state
	memory.Reset(other.memory);
	registers = other.registers;
}