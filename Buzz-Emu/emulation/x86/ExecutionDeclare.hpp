#pragma once

#include "../../emulator/Emulator.hpp"

#include "operand_types/Immediate.hpp"
#include "operand_types/AddressType.hpp"
#include "operand_types/RegisterType.hpp"

template <typename Func>
struct exec_declare {
public:
	exec_declare(Emulator& emu)
		: emu_(&emu) {};

	void operator() (const ImmValue& imm) {

	}

	void operator() (const RegisterValue& reg) {

	}

	void operator() (const AddressValue& mem) {

	}
	
private:
	Emulator* emu_;
};