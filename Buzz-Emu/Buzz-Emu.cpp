
#include <iostream>

#include <chrono>

#include "emulator/Emulator.hpp"
#include "emulation/x86/InstructionHandler.hpp"

int main()
{
    //NOT FINISHED¡@YET: https://youtu.be/iM3s8-umRO0?t=29219
    Emulator emu(32 * 1024 * 1024);


    //Load the application into the emulator
    Section text_section = {
        .file_off = 0x00000400,
        .virt_addr = 0x00001000,
        .file_size = 0x00001200,
        .mem_size = 0x00001200,
        .permission = (PERM_READ | PERM_EXEC)
    };

    Section rdata = {
        .file_off = 0x00001600,
        .virt_addr = 0x00003000,
        .file_size = 0x00001400,
        .mem_size = 0x00001400,
        .permission = (PERM_READ)
    };

    Section data = {
        .file_off = 0x00002a00,
        .virt_addr = 0x00005000,
        .file_size = 0x00000200,
        .mem_size = 0x00000200,
        .permission = (PERM_READ | PERM_WRITE)
    };

    Section tail_data = {
        .file_off = 0x00002c00,
        .virt_addr = 0x00006000,
        .file_size = 0x00000600,
        .mem_size = 0x00000600,
        .permission = PERM_READ
    };

    // also: load "C:\Windows\System32\msvcp140.dll"


    std::vector<Section> load_segment = { text_section, rdata, data, tail_data };
    emu.LoadExecutable("C:\\Users\\USER\\source\\repos\\ConsoleApplication1\\x64\\Release\\ConsoleApplication1.exe", load_segment);

    Emulator forked(emu);

    //Set the program entry points
    emu.SetReg<u64>(Register::Rip, 0x00001000);

    //set up a stack
    auto base = emu.memory.mem_alloc(3 * 1024).value(); 
    std::cout << "Stack: 0x" << std::hex << base << " - " << base + 0x2200 << "\n";
    emu.SetReg<u64>(Register::Rsp, base + 3 * 1024);

    // Example memory allocation and initialization
    auto argv = emu.memory.mem_alloc(8).value(); // Allocate memory for argv
    emu.memory.Write(argv, "ConsoleApplication1.exe\0");        // Write a null-terminated string to memory

    // Push arguments onto the stack in the order used by the main function
    push(0ull);   // Push auxv (for example purposes; typically this is not used directly in the main function)
    push(0ull);   // Push envp (pointer to environment variables; here set to 0 for example)
    push(argv);   // Push argv (pointer to the argument vector)
    push(1ull);   // Push argc (number of arguments; in this case, 1 argument)

    /*while (true) {
        auto num = emu.Run();
        switch (num) {
            //Syscall processing, which is not going to be emulated in guest 
        case VmExit::Syscall:
            switch (emu.Reg(Register::Rax)) {
            case 0x2C: //NtTerminateProcess
                //exit
                break;
            }
        }
    }*/

    emu.TestRun();
}

