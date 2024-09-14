#pragma once

#include "SizeEnum.hpp"

struct x86Dcctx {
	u8* ip;		/*instruction pointer*/

	union {
		u8 pos;	/*position in buffer*/
		u8 len;	/*instruction length*/
	};

	DasmMode		dmode : 2;
	OperandSize		osize : 2;
	AddressingSize	asize : 2;

	u8 modrm;
	u8 sib;
	u8 vsib_base;

	u8 pfx_rex;
	u8 pfx_last;
	u8 pfx_seg;
	u8 pfx_mandatory;

	union {
		u8 pfx_vex[3];
		u8 pfx_xop[3];
	};

	u8 pos_modrm : 4;
	u8 pos_sib : 4;
	u8 pos_rex : 4;
	u8 pos_opcode : 4;
	u8 pos_disp : 4;

	u8 disp_size : 4;

	/* prefixes present */
	u8 pfx_p_rex : 1;
	u8 pfx_p_seg : 1;
	u8 pfx_p_osize : 1;
	u8 pfx_p_asize : 1;
	u8 pfx_p_lock : 1;
	u8 pfx_p_rep : 1;
	u8 pfx_p_repne : 1;
	u8 pfx_p_vex2b : 1;
	u8 pfx_p_vex3b : 1;
	u8 pfx_p_xop : 1;

	u8 p_modrm : 1;
	u8 p_sib : 1;
	u8 p_vsib : 1;

	u8 opcode_size;

	size_t table_index;

	u8* table_compressed;
	size_t table_size;
};