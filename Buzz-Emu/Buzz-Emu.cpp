
#include <iostream>
#include <chrono>
#include <variant>

#include "emulator/Emulator.hpp"
#include "emulation/x86/InstructionHandler.hpp"
#include "emulator/Syscalls.hpp"

#include "pe/ImportHandler.hpp"
#include "pe/ExportHandler.hpp"

void vmexit_to_string(VmExitResult result) {
    if (std::holds_alternative<VmExit>(result)) {
        auto exit_code = std::get<VmExit>(result);
        switch (exit_code) {
        case VmExit::Exit:
            std::cout << "Vm exited with exit()-like syscall\n"; 
            break;
        case VmExit::SyscallIntegerOverflow:
            std::cout << "Vm exited with syscall integer overflow\n";
            break;
        case VmExit::AddressIntegerOverflow:
            std::cout << "Vm exited with address integer overflow\n";
            break;
        default:
            std::cout << "Vm exited with unknown reason\n";
            break;
        }
    }
    else if (std::holds_alternative<ReadFault>(result)) {
        ReadFault exit_code = std::get<ReadFault>(result);
        std::cout << "Vm exited with ReadFault(0x" << std::hex << exit_code.addr << ")\n";
    }
    else if (std::holds_alternative<WriteFault>(result)) {
        WriteFault exit_code = std::get<WriteFault>(result);
        std::cout << "Vm exited with ReadFault(0x" << std::hex << exit_code.addr << ")\n";
    }
    else if (std::holds_alternative<AddressMissed>(result)) {
        AddressMissed exit_code = std::get<AddressMissed>(result);
        std::cout << "Vm exited with ReadFault(0x" << std::hex << exit_code.addr << ", " << std::dec << exit_code.size << ")\n";
    }
}

int main()
{
    //NOT FINISHED¡@YET: https://youtu.be/iM3s8-umRO0?t=30843
    Emulator emu(32 * 1024 * 1024);
    
    emu.LoadExecutable("C:\\Users\\USER\\source\\repos\\ConsoleApplication1\\x64\\Release\\ConsoleApplication1.exe");

    Emulator forked(emu);

    //Set the program entry points
    emu.SetReg<u64>(Register::Rip, 0x00001000);

    //set up a stack
    auto base = emu.memory.mem_alloc(3 * 1024).value(); 
    emu.memory.SetPermission(base, 3 * 1024, PERM_READ | PERM_WRITE);
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

    VmExitResult num;
    while (true) {
        num = emu.Run();
        if (std::holds_alternative<VmExit>(num)) {
            auto vmexit_enum = std::get<VmExit>(num);
            switch (vmexit_enum) {
            case VmExit::Syscall: {
                auto result = handle_syscall(emu);
                if (!result.has_value()) {
                    num = result.error();
                    goto exit_loop; // Jump to exit the loop
                }

                // Advance PC
                auto pc = emu.Reg(Register::Rip);
                emu.SetReg<u64>(Register::Rip, pc + /* Sizeof syscall */2);
                break;
            }
            default:
                goto exit_loop;
            }
        }
    }

exit_loop:
    vmexit_to_string(num);
    //emu.Run();
}

