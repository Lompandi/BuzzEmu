#pragma once

#include <variant>

//An access of Virtaddr with usize failed
struct ReadFault {
	VirtualAddr addr;
	size_t		size;
};

using VmExitTypes = std::variant<
	int,
	ReadFault
>;

//reasons why vm exit
enum class VmExit {
	//exit due to syscall instruction
	Syscall,
	//integer overflow in syscall due to bad-supplied arguments by the program
	SyscallIntegerOverflow,
};