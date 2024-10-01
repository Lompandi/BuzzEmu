#pragma once

#include <variant>
#include "../memory/VirtualAddr.hpp"

//An access of Virtaddr with usize failed
struct ReadFault {
	VirtualAddr addr;
};

struct WriteFault {
	VirtualAddr addr;
};

struct AddressMissed {
	VirtualAddr addr;
	size_t		size;
};

//reasons why vm exit
enum class VmExit {
	//exit due to syscall instruction
	Syscall,
	// the VM¡@exited cleanly as requested by the VM
	Exit,
	//integer overflow in syscall due to bad-supplied arguments by the program
	SyscallIntegerOverflow,
	// A read or write memory request overflowed the address size
	AddressIntegerOverflow,
	//Read fault
	ReadFault,
};

using VmExitResult = std::variant<
	VmExit,
	ReadFault,
	WriteFault,
	AddressMissed
>;