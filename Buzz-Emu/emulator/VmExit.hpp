#pragma once

//reasons why vm exit
enum VmExit {
	//exit due to syscall instruction
	Syscall,

	//Read failed
	ReadFault,
};