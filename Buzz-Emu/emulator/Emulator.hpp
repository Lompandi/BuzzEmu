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

	/*
	Reset all state
	*/
	void Reset(const Emulator& other);

	/*
	Get a register from the guest
	*/
	u64 Reg(Register reg);
	void SetReg(Register reg, u64 val);
};