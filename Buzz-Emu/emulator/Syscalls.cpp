

#include "Syscalls.hpp"
#include "../core/Fs.hpp"
#include "../emulation/x86/InstructionHandler.hpp"

bzmu::result<u8, VmExitResult> handle_syscall(Emulator& emu) {
    switch (emu.Reg(Register::Rax)) {
    case 0x07: //NtDeviceIoControlFile
        emu.SetReg<u64>(Register::Rax, !0); //error for now
        break;
    case 0x08: { //NtWriteFile
        auto handle_ptr = emu.Reg(Register::Rcx);
        auto event_ptr = emu.Reg(Register::Rdi);
        auto apc_runtime_ref = emu.Reg(Register::R8);
        auto apc_ctx_ptr = emu.Reg(Register::R9);
        auto io_status_block_ref = emu.Reg(Register::R10);
        p0 buf_ptr = reinterpret_cast<p0>(pop(emu));
        u64  len = pop(emu);
        auto byte_offset = pop(emu);
        p64 key = reinterpret_cast<p64>(pop(emu));

        //we will handle only for writing memory case now
        /* Compute the write offset and wrap around to it 
        to validate that it will not overflow */
        auto ptr = wrap_around<p8, u64>(checked_add(reinterpret_cast<u64>(buf_ptr)
            , byte_offset));

        if (!ptr.has_value())
            return bzmu::result_error{ VmExitResult(VmExit::SyscallIntegerOverflow) };
        auto _ptr = ptr.value();

        auto data = emu.memory.peek_perms(VirtualAddr(buf_ptr), len,
            PERM_READ);
        if (!data.has_value())
            return bzmu::result_error{ data.error() };
        break;
    }
    case 0x2C: //NtTerminateProcess
        return bzmu::result_error{ VmExitResult(VmExit::Exit) }; //exited cleanly
    default:
        break;
    }

    return true;
}