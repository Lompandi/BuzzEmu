
#include "Register.hpp"

u8 FetchByteRegs(u64 data, u64 mask) {
	return static_cast<u8>((data & mask) >> ((mask == mask_regs_high) * 8));
}