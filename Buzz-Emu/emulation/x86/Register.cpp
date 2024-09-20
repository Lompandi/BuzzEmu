
#include "Register.hpp"

u8 get_byte_register(u64 data, u64 mask) {
	return static_cast<u8>((data & mask) >> ((mask == mask_regs_high) * 8));
}