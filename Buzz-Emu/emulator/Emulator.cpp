
#include <bit>
#include <iostream>

#include "Emulator.hpp"

#include "VmExit.hpp"
#include "../core/Fs.hpp"
#include "../emulation/x86/ModRM.hpp"
#include "../emulation/x86/InstructionHandler.hpp"

#define instruction_param *this, &lendec.GetDecoderCtx(), inst

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

void CrashDump(Emulator& emu) {
	//dump general pourpose registers
	std::cout << "rax: 0x" << std::hex << emu.Reg(Register::Rax);
	std::cout << " rcx: 0x" << std::hex << emu.Reg(Register::Rcx) << "\n";
	std::cout << "rdx: 0x" << std::hex << emu.Reg(Register::Rdx);
	std::cout << " rbx: 0x" << std::hex << emu.Reg(Register::Rbx) << "\n";
	std::cout << "rsp: 0x" << std::hex << emu.Reg(Register::Rsp);
	std::cout << " rbp: 0x" << std::hex << emu.Reg(Register::Rbp) << "\n";
	std::cout << "rsi: 0x" << std::hex << emu.Reg(Register::Rsi);
	std::cout << " rdi: 0x" << std::hex << emu.Reg(Register::Rdi) << "\n\n";

	//dump Cpu flags
	std::cout << "cf: " << std::hex <<emu.flags.CF; // Carry Flag
	std::cout << " pf: " << std::hex <<emu.flags.PF; // Parity Flag
	std::cout << " af: " << std::hex <<emu.flags.AF; // Auxiliary Carry Flag
	std::cout << " zf: " << std::hex <<emu.flags.ZF << "\n"; // Zero Flag
	std::cout << "sf: " << std::hex <<emu.flags.SF; // Sign Flag
	std::cout << " tf: " << std::hex <<emu.flags.TF; // Trap Flag
	std::cout << " if: " << std::hex <<emu.flags.IF; // Interrupt Enable Flag
	std::cout << " df: " << std::hex <<emu.flags.DF << "\n"; // Direction Flag
	std::cout << "of: " << std::hex <<emu.flags.OF << "\n\n"; // Overflow Flag
}

void opcode_cpy(uint8_t* opcode, const uint8_t* src, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		opcode[i] = src[size - i - 1];
	}
}
//TODO: currently using lineaer emulating , will change it to table if so
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
		std::cout << "opcode size: " << lendec.GetDecoderCtx().opcode_size << "\n";
		
		u32 opcode = 0;
		u8 modrm;
		if (lendec.GetDecoderCtx().pfx_p_rex)
			std::cout << " prefixed REX";
		
		if (lendec.GetDecoderCtx().p_sib)
			std::cout << " have SIB";

		/*std::copy(inst.begin() + lendec.GetDecoderCtx().pos_opcode,
			inst.begin() + lendec.GetDecoderCtx().pos_opcode + lendec.GetDecoderCtx().opcode_size, 
			&opcode); //copy the opcode */ 

		opcode_cpy((u8*)&opcode,
			inst.data() + lendec.GetDecoderCtx().pos_opcode,
			lendec.GetDecoderCtx().opcode_size);

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
			SetReg<u8>(Register::Rax, GET_L_REG(Reg(Register::Rax)) | read_from_vec<u8>(inst, 1));
			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) | read_from_vec<u8>(inst, 1));
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
			SetReg<u8>(Register::Rax, GET_L_REG(Reg(Register::Rax)) & read_from_vec<u8>(inst, 1));
			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) & read_from_vec<u8>(inst, 1));
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
			SetReg<u8>(Register::Rax, SubAndSetFlags(GET_L_REG(Reg(Register::Rax)), read_from_vec<u8>(inst, 1), flags));
			break;
/*======================= Subtract(0x2D) =====================================*/
		case Instruction::SUB_2D:
			if (lendec.GetDecoderCtx().osize == X86_Osize_16bit && inst.size() == 3)
				SetReg<u16>(Register::Rax, SubAndSetFlags(GET_X_REG(Reg(Register::Rax)), read_from_vec<u16>(inst, 1), flags));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_32bit && inst.size() == 5)
				SetReg<u32>(Register::Rax, SubAndSetFlags(GET_EXT_REG(Reg(Register::Rax)), read_from_vec<u32>(inst, 1), flags));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_64bit && inst.size() == 5)
				SetReg<u64>(Register::Rax, SubAndSetFlags(Reg(Register::Rax), static_cast<u64>(read_from_vec<u32>(inst, 1)), flags));
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
			SetReg<u8>(Register::Rax, GET_L_REG(Reg(Register::Rax)) ^ read_from_vec<u8>(inst, 1));
			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) ^ read_from_vec<u8>(inst, 1));
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
		case Instruction::POP_58:
			Pop_58_5F(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::POP_59:
			Pop_58_5F(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::POP_5A:
			Pop_58_5F(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::POP_5B:
			Pop_58_5F(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::POP_5C:
			Pop_58_5F(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::POP_5D:
			Pop_58_5F(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::POP_5E:
			Pop_58_5F(*this, &lendec.GetDecoderCtx(), inst);
			break;
		case Instruction::POP_5F:
			Pop_58_5F(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Move with sign-extended (0x63) =====================*/
		case Instruction::MOVSXD_63:
			Movsxd_63(*this, &lendec.GetDecoderCtx(), inst);
			break;
/*======================= Jump if zero (0x74) ================================*/
		case Instruction::JZ_74:
			//Signed-extended offset
			pc = Reg(Register::Rip) + (this->flags.ZF ? read_from_vec<s8>(inst, 1) : 0);
			break;
		case Instruction::JNZ_75:
			pc = Reg(Register::Rip) + (this->flags.ZF ? 0 : read_from_vec<s8>(inst, 1));
			break;
/*======================= Jump if less (0x7C) ================================*/
		case Instruction::JL_7C:
			//Signed-extended offset
			pc = Reg(Register::Rip) + (this->flags.SF != this->flags.OF ? read_from_vec<s8>(inst, 1) : 0);
			std::cout << "JL Rel8: 0x" << std::hex << (int)read_from_vec<s8>(inst, 1) << "\n";
			std::cout << "Pc after JL: 0x" << std::hex << pc << "\n";
			break;
		case Instruction::JLE_7E:
			pc = Reg(Register::Rip) + 
				(this->flags.ZF == 1 || this->flags.SF != this->flags.OF ? read_from_vec<s8>(inst, 1) : 0);
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
			Mov_8B(instruction_param);
			break;
/*======================= No operation instruction =======================*/
		case Instruction::NOP:
			break;
/*======================= Test(0xA8) ===========================================*/
		case Instruction::TEST_A8:
			SetLogicOpFlags(flags, GET_L_REG(Reg(Register::Rax)) & read_from_vec<u8>(inst, 1));
			break;
/*======================= Test(0xA9) ===========================================*/
		case Instruction::TEST_A9:
			if (lendec.GetDecoderCtx().osize == X86_Osize_16bit && inst.size() == 3)
				SetLogicOpFlags(flags, GET_X_REG(Reg(Register::Rax)) & read_from_vec<u16>(inst, 1));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_32bit && inst.size() == 5)
				SetLogicOpFlags(flags, GET_EXT_REG(Reg(Register::Rax)) & read_from_vec<u32>(inst, 1));
			else if (lendec.GetDecoderCtx().osize == X86_Osize_64bit && inst.size() == 5)
				SetLogicOpFlags(flags, Reg(Register::Rax) & static_cast<u64>(read_from_vec<u32>(inst, 1)));
			break;
		case Instruction::MOV_B8:
			Mov_B8_BF(instruction_param);
			break;
		case Instruction::MOV_B9:
			Mov_B8_BF(instruction_param);
			break;
		case Instruction::MOV_BA:
			Mov_B8_BF(instruction_param);
			break;
		case Instruction::MOV_BB:
			Mov_B8_BF(instruction_param);
			break;
		case Instruction::MOV_BC:
			Mov_B8_BF(instruction_param);
			break;
		case Instruction::MOV_BD:
			Mov_B8_BF(instruction_param);
			break;
		case Instruction::MOV_BE:
			Mov_B8_BF(instruction_param);
			break;
		case Instruction::MOV_BF:
			Mov_B8_BF(instruction_param);
			break;
		case Instruction::RET_C3:
			std::cout << "Returning from procedure...\n";
			Ret_C3(*this, &lendec.GetDecoderCtx(), inst, pc);
			break;
		case Instruction::MOV_C7:
			Mov_C7(*this, &lendec.GetDecoderCtx(), inst);
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
			pc = Reg(Register::Rip) + static_cast<s64>(read_from_vec<s8>(inst, 1));
			break;
		case Instruction::_FF:
			switch (MODRM_REG(lendec.GetDecoderCtx().modrm))
			{
			case 0x01:
				Dec_FF(instruction_param);
				break;
			case 0x02:
				Call_FF_reg2(*this, &lendec.GetDecoderCtx(), inst, pc);
				break;
			default:
				break;
			}
			break;
		case Instruction::SYSCALL_0F05:
			return VmExit::Syscall;
		case Instruction::MOVZX_0FB6:
			Movzx_0FB6(instruction_param);
			break;
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

	while (true) {
		if (pc > 0x1243) break;

		pc = Reg(Register::Rip);

		u32 opcode = 0;

		std::vector<u8> inst;
		// Fetch the current instructions
		memory.ReadInstruction(lendec, pc, inst, (PERM_READ | PERM_EXEC));
		//std::cout << "opcode size: " << (int)(lendec.GetDecoderCtx().opcode_size) << "\n";


		// Extract context information
		auto pos_opcode = lendec.GetDecoderCtx().pos_opcode;
		auto opcode_size = lendec.GetDecoderCtx().opcode_size;

		// Copy the opcode from the instruction vector
		opcode_cpy((u8*)&opcode, inst.data() + pos_opcode, opcode_size);

		std::cout << "0x" <<std::hex << pc << " with opcode 0x" << std::hex << opcode << "\n";

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
		case Instruction::POP_58:
			break;
		case Instruction::POP_59:
			break;
		case Instruction::POP_5A:
			break;
		case Instruction::POP_5B:
			break;
		case Instruction::POP_5C:
			break;
		case Instruction::POP_5D:
			break;
		case Instruction::POP_5E:
			break;
		case Instruction::POP_5F:
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
		case Instruction::JLE_7E:
			break;
		case Instruction::_81:
			break;
		case Instruction::_83:
			switch (MODRM_REG(lendec.GetDecoderCtx().modrm)) {
			case 0b000:
				CrashDump(*this);
				std::cout << "calulating add...\n";
				Add_83(instruction_param);
				CrashDump(*this);
				break;
			case 0x01:
				break;
			case 0x04:
				break;
			case 0x05:
				//Sub_83(instruction_param);
				break;
			case 0x06:
				break;
			case 0x07:
				break;
			default:
				std::cout << "[EMU] Error at 0x" << std::hex << pc << ", unknown opcode 0x" << std::hex << opcode << "\n";
				break;
			}
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
		case Instruction::LEA_8D:
			//Lea_8D(*this, &lendec.GetDecoderCtx(), inst, pc);
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
		case Instruction::MOV_B9:
			break;
		case Instruction::MOV_BA:
			break;
		case Instruction::MOV_BB:
			break;
		case Instruction::MOV_BC:
			break;
		case Instruction::MOV_BD:
			break;
		case Instruction::MOV_BE:
			break;
		case Instruction::MOV_BF:
			break;
		case Instruction::RET_C3:
			break;
		case Instruction::MOV_C7:
			//Mov_C7(instruction_param);
			break;
		case 0xCC:
			break;
			/*============================ Call procedure (0xE8) ===========================*/
		case Instruction::CALL_E8:
			break;
		case Instruction::JMP_E9:
			break;
			/*======================= Jump short (0xEB) ====================================*/
		case Instruction::JMP_EB:
			break;
		case Instruction::_FF:
			switch (MODRM_REG(lendec.GetDecoderCtx().modrm))
			{
			case 0b001:
				break;
			case 0b010:
				break;
			default:
				std::cout << "[EMU] Error at 0x" << std::hex << pc << ", unknown opcode 0x" << std::hex << opcode << "\n";
				break;
			}
			break;
		case Instruction::MOVZX_0FB6:
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