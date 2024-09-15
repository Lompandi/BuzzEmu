#pragma once

#include <array>
#include <string>

#include "VmExit.hpp"
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
	VmExit Run();
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

	template <typename TReg = unsigned long long>
	void SetReg(Register reg, TReg val, ByteRegister extra_mask = ByteRegister::LowByte) {
		// Define masks and shifts for each register portion
		constexpr uint64_t masks[] = {
			0xFFFFFFFFFFFF00FFULL, // Mask for AH (clear high 8 bits of AX)
			0xFFFFFFFFFFFFFF00ULL,  // Mask for AL (clear low 8 bits of AX)
			0xFFFFFFFFFFFF0000ULL, // Mask for AX (clear lower 16 bits)
			0xFFFFFFFF00000000ULL,
			0xFFFFFFFF00000000ULL, // Mask for EAX (clear lower 32 bits)
			0x0,
			0x0,
			0x0000000000000000ULL,
			0x0000000000000000ULL, // Mask for RAX (no change)
		};

		constexpr int shifts[] = {
			8,        // AH (shift by 8 bits)
			0,        // RAX (no shift)
			0,        // EAX (no shift, value is 32-bit)
			0,        // AX (no shift, value is 16-bit)
			0,         // AL (no shift, value is 8-bit)
			0,
			0,
			0,
			0,
		};
		//std::cout << "Before: 0x" << std::hex << Reg(reg) << "\n";

		uint64_t mask = masks[sizeof(TReg) - (extra_mask & 1)];
		uint64_t shift = shifts[sizeof(TReg) - (extra_mask & 1)];
		uint64_t shifted_val = (static_cast<uint64_t>(val) << shift) & ~mask;

		//std::cout << "Mask: \t0x" << std::hex << mask << "\n";

		//std::cout << "After:  0x" << std::hex << (u64)((Reg(reg)) & mask | shifted_val) << "\n";

		SetReg64(reg, (Reg(reg) & mask) | shifted_val);
	}
};