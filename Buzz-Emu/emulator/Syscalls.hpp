#pragma once

#include "VmExit.hpp"
#include "Emulator.hpp"
#include "../include/buzzemu/Results.hpp"

bzmu::result<u8, VmExitResult> handle_syscall(Emulator& emu);