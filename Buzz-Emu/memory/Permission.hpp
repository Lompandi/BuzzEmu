#pragma once

#include "../core/Memtypes.hpp"

constexpr u8 PERM_READ = 1 << 0;
constexpr u8 PERM_WRITE = 1 << 1;
constexpr u8 PERM_EXEC = 1 << 2;
constexpr u8 PERM_RAW = 1 << 3;

using Permission = u8;
