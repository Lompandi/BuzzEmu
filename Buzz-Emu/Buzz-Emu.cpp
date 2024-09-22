
#include <iostream>

#include <chrono>

#include "emulator/Emulator.hpp"
#include "emulation/x86/InstructionHandler.hpp"

#include "pe/ImportHandler.hpp"
#include "pe/ExportHandler.hpp"

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

    std::vector<u8> content = read_file("C:\\Users\\USER\\source\\repos\\ConsoleApplication1\\x64\\Release\\ConsoleApplication1.exe");
    PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)(content.data());

    bzmu::pe::import_container container;
    container.set_import_table(dos_hdr);
    auto result = container.get_dll_by_function("RtlVirtualUnwind");
    std::wcout << L"Dll for function RtlVirtualUnwind: " << result.value() << "\n";
    auto result2 = container.get_functions_by_dll(result.value());
    std::cout << "Imported from kernel32.dll: \n";
    std::cout << result2.size() << "\n";
    for (const auto& it : result2) {
        std::cout << it << "\n";
    }

    std::cout << "\nExported function from msvcp140.dll:\n";
    std::vector<u8> libcpp_content = read_file("C:\\Windows\\System32\\msvcp140.dll");
    PIMAGE_DOS_HEADER dos_hdr2 = (PIMAGE_DOS_HEADER)(libcpp_content.data());
    bzmu::pe::export_container container2;
    container2.set_export_container(dos_hdr2);
    std::cout << "Address for std::cout: " << std::hex << container2.get_function_address("?cout@std@@3V?$basic_ostream@DU?$char_traits@D@std@@@1@A") << '\n';

    //==================Load===================
    std::vector<Section> load_segment = {text_section, rdata, data, tail_data};
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

