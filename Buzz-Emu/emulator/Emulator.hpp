#pragma once

#include <array>
#include <string>

#include "../memory/Mmu.hpp"
#include "../memory/Section.hpp"
#include "../emulation/x86/Flags.hpp"
#include "../emulation/x86/Register.hpp"

#define DEBUG_FUNC 

//All the state of the emulated system
class Emulator {
public:
	//Memory for the emulator
	Mmu memory;

	//All x86 64-bit register
	std::array<u64, 17> registers;

	//All x86 64 Cpu flags
	FlagsRegister64 flags;

	/*
	Create a new emulator with `size` bytes of memory
	*/
	Emulator(size_t size) : memory(size), registers{} {};
	Emulator(Emulator& other) : memory(other.memory), registers(other.registers) {};

	/*
	Load a file into th eemulators address space using the sections as described
	*/
	bool LoadExecutable(const std::string& filename, const std::vector<Section>& sections);

	/*
	Run the machine
	*/
	void Run();
	void TestRun();

	/*
	Reset all state
	*/
	void Reset(const Emulator& other);

	/*
	Get a register from the guest
	*/
	u64 Reg(Register reg);

	void SetReg64(Register reg, u64 val);

	template<typename TReg = unsigned long long>
	void SetReg(Register reg, TReg val, RegisterMask extra_mask = RegisterMask::LowByte) {
		u64 qval = Reg(reg);
		//std::cout << "[SETREG] Original value: 0x" << std::hex << qval << " := 0x" << val << "\n";

		unsigned long long mask = 0xFFFFFFFFFFFFFFFFUL << (int)(sizeof(TReg) * 8 + (extra_mask / 0xFF) * 8);;
		mask |= extra_mask;
		mask = (sizeof(TReg) == 8 ? 0 : mask); //64 bits will be inaccurate

		//std::cout << "[SETREG] Mask: 0x" << std::hex << mask << "\n";

		qval &= mask;
		qval |= ((val << (mask / 0xFF) * 8) & ~mask);

		//std::cout << "[SETREG] After value: 0x" << std::hex << qval << "\n";
		SetReg64(reg, qval);
	}
};