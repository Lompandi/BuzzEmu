
#include <iostream>

#include "emulator/Emulator.hpp"

int main()
{
    //NOT FINISHED¡@YET: 4:35:19 https://youtu.be/iM3s8-umRO0?t=16519
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

    

    std::vector<Section> load_segment = { text_section, rdata, data, tail_data };

    emu.LoadExecutable("C:\\Users\\USER\\source\\repos\\ConsoleApplication1\\x64\\Release\\ConsoleApplication1.exe", load_segment);

    Emulator forked(emu);

    //Set the program entry points
    emu.SetReg(Register::Rip, 0x00001000);
    emu.Run();
}

