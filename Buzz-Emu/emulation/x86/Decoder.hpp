#pragma once

#include <vector>

#include "DecodeContext.hpp"
#include "../../core/Memtypes.hpp"
#include "../../memory/VirtualAddr.hpp"

constexpr size_t max_inst_length = 15;

constexpr u8 escape_opcode_2b	= 0x0f;
constexpr u8 escape_opcode_3b_1 = 0x38;
constexpr u8 escape_opcode_3b_2 = 0x3a;

/* ModRM byte operation */
#define MODRM_RM(b)		((b >> 0) & 7)
#define MODRM_REG(b)	((b >> 3) & 7)
#define MODRM_MOD(b)	((b >> 6) & 3)

/* SIB byte operation */
#define SIB_BASE(b)         ((b >> 0) & 7)
#define SIB_INDEX(b)        ((b >> 3) & 7)
#define SIB_SCALE(b)        ((b >> 6) & 3)

/* REX byte determination and operation */
#define REX_PREFIX(b)       (((b >> 4) & 0x0f) == 4)
#define _REX_B(b)           ((b >> 0) & 1)
#define _REX_X(b)           ((b >> 1) & 1)
#define _REX_R(b)           ((b >> 2) & 1)
#define _REX_W(b)           ((b >> 3) & 1)

/*
	VEX 2-byte
*/
#define _VEX_2B_PP(b)       ((b >> 0) & 3)
#define _VEX_2B_L(b)        ((b >> 2) & 1)
#define _VEX_2B_VVVV(b)     ((~(b >> 3)) & 0x0f)
#define _VEX_2B_R(b)        ((~(b >> 7)) & 1)

#define VEX_2B_PM(b)        (MODRM_MOD(b) == 3)

/*
	VEX 3-byte
*/
#define _VEX_3B_MMMMM(b)    ((b >> 0) & 0x1f)
#define _VEX_3B_B(b)        ((~(b >> 5)) & 1)
#define _VEX_3B_X(b)        ((~(b >> 6)) & 1)
#define _VEX_3B_R(b)        ((~(b >> 7)) & 1)

#define _VEX_3B_PP(b)       ((b >> 0) & 3)
#define _VEX_3B_L(b)        ((b >> 2) & 1)
#define _VEX_3B_VVVV(b)     ((~(b >> 3)) & 0x0f)
#define _VEX_3B_W(b)        ((b >> 7) & 1)

#define VEX_3B_PM(b)        (MODRM_MOD(b) == 3)

/*
	XOP
*/
#define _XOP_MMMMM(b)       ((b >> 0) & 0x1f)
#define _XOP_B(b)           ((~(b >> 5)) & 1)
#define _XOP_X(b)           ((~(b >> 6)) & 1)
#define _XOP_R(b)           ((~(b >> 7)) & 1)

#define _XOP_PP(b)          ((b >> 0) & 3)
#define _XOP_L(b)           ((b >> 2) & 1)
#define _XOP_VVVV(b)        ((~(b >> 3)) & 0x0f)
#define _XOP_W(b)           ((b >> 7) & 1)

#define XOP_PM(b)           (MODRM_MOD(b) == 3)
#define XOP_VALID(b)        (_XOP_MMMMM(b) > 7 && _XOP_MMMMM(b) < 11)

/*
	EVEX
*/
#define _EVEX_B(b)          ((~(b >> 5)) & 1)
#define _EVEX_X(b)          ((~(b >> 6)) & 1) 
#define _EVEX_R(b)          ((~(b >> 7)) & 1)
#define _EVEX_RR(b)         ((~(b >> 4)) & 1) /* R' */
#define _EVEX_MM(b)         ((b >> 0) & 3)

#define _EVEX_PP(b)         ((b >> 0) & 3)
#define _EVEX_W(b)          ((b >> 7) & 1)
#define _EVEX_VVVV(b)       ((~(b >> 3)) & 0x0f)

#define _EVEX_Z(b)          ((b >> 7) & 1) 
#define _EVEX_LL(b)         ((b >> 6) & 1) /* L' */
#define _EVEX_L(b)          ((b >> 5) & 1) 
#define _EVEX_BB(b)         ((b >> 4) & 1)
#define _EVEX_VV(b)         ((~(b >> 3)) & 1) /* V' */
#define _EVEX_AAA(b)        ((b >> 0) & 7)

#define EVEX_P0(b)          (((b >> 2) & 3) == 0) /* check on P0, else #UD */
#define EVEX_P1(b)          (((b >> 2) & 1) == 1) /* check on P1, else #UD */
#define EVEX_PM(b)          (MODRM_MOD(b) == 3) /* BOUND must have modrm.mod = !11b */

/* from .cpp originally */
#define REX_B               _REX_B(x86_dctx.pfx_rex)
#define REX_X               _REX_X(x86_dctx.pfx_rex)
#define REX_R               _REX_R(x86_dctx.pfx_rex)
#define REX_W               _REX_W(x86_dctx.pfx_rex)

#define VEX_2B_PP           _VEX_2B_PP(x86_dctx.pfx_vex[0])
#define VEX_2B_L            _VEX_2B_L(x86_dctx.pfx_vex[0])
#define VEX_2B_VVVV         _VEX_2B_VVVV(x86_dctx.pfx_vex[0])
#define VEX_2B_R            _VEX_2B_R(x86_dctx.pfx_vex[0])

#define VEX_3B_MMMMM        _VEX_3B_MMMMM(x86_dctx.pfx_vex[0])
#define VEX_3B_B            _VEX_3B_B(x86_dctx.pfx_vex[0])
#define VEX_3B_X            _VEX_3B_X(x86_dctx.pfx_vex[0])
#define VEX_3B_R            _VEX_3B_R(x86_dctx.pfx_vex[0])
#define VEX_3B_PP           _VEX_3B_PP(x86_dctx.pfx_vex[1])
#define VEX_3B_L            _VEX_3B_L(x86_dctx.pfx_vex[1])
#define VEX_3B_VVVV         _VEX_3B_VVVV(x86_dctx.pfx_vex[1])
#define VEX_3B_W            _VEX_3B_W(x86_dctx.pfx_vex[1])

#define XOP_MMMMM           _XOP_MMMMM(x86_dctx.pfx_xop[0])
#define XOP_B               _XOP_B(x86_dctx.pfx_xop[0])
#define XOP_X               _XOP_X(x86_dctx.pfx_xop[0])
#define XOP_R               _XOP_R(x86_dctx.pfx_xop[0])
#define XOP_PP              _XOP_PP(x86_dctx.pfx_xop[1])
#define XOP_L               _XOP_L(x86_dctx.pfx_xop[1])
#define XOP_VVVV            _XOP_VVVV(x86_dctx.pfx_xop[1])
#define XOP_W               _XOP_W(x86_dctx.pfx_xop[1])

/*
	Prefixes
*/
constexpr u8 PREFIX_OSIZE        = 0x66; /* operand-size override */
constexpr u8 PREFIX_ASIZE        = 0x67; /* address-size override */
constexpr u8 PREFIX_SEGOVRD_CS   = 0x2e; /* segment override, ignored in 64-bit mode */
constexpr u8 PREFIX_SEGOVRD_DS   = 0x3e; /* segment override, ignored in 64-bit mode */
constexpr u8 PREFIX_SEGOVRD_ES   = 0x26; /* segment override, ignored in 64-bit mode */
constexpr u8 PREFIX_SEGOVRD_FS   = 0x64; /* segment override */
constexpr u8 PREFIX_SEGOVRD_GS   = 0x65; /* segment override */
constexpr u8 PREFIX_SEGOVRD_SS   = 0x36; /* segment override, ignored in 64-bit mode */
constexpr u8 PREFIX_LOCK         = 0xf0; /* lock rw atomically */
constexpr u8 PREFIX_REPNE        = 0xf2;
constexpr u8 PREFIX_REP          = 0xf3;
constexpr u8 PREFIX_BRT          = 0x2e; /* branch taken (hint), only with jcc */
constexpr u8 PREFIX_BRNT         = 0x3e; /* branch not taken (hint), only with jcc */
constexpr u8 PREFIX_VEX_3B       = 0xc4;
constexpr u8 PREFIX_VEX_2B       = 0xc5;
constexpr u8 PREFIX_EVEX         = 0x62;
constexpr u8 PREFIX_XOP          = 0x8f;

enum
{
    F_NONE = 0,

    /*
        the operand size is forced to a 64-bit operand size when in 64-bit mode
        (prefixes that change operand size are ignored for this instruction in
        64-bit mode).
    */
    F_F64 = F_NONE, /* disabled, will be handled in code */

    /*
        when in 64-bit mode, instruction defaults to 64-bit operand size and
        cannot encode 32-bit operand size.
    */
    F_D64 = F_NONE, /* disabled, will be handled in code */

    /*
        AM_C: the reg field of the ModR/M byte selects a control register
        AM_D: the reg field of the ModR/M byte selects a debug register
        AM_G: the reg field of the ModR/M byte selects a general register
        AM_P: the reg field of the ModR/M byte selects a packed quadword
        AM_S: the reg field of the ModR/M byte selects a segment register
        AM_V: the reg field of the ModR/M byte selects a 128-bit XMM register or a 256-bit YMM register

        AM_E: the ModR/M byte follows the opcode and specifies the operand (reg or mem)
        AM_M: the ModR/M byte may refer only to memory
        AM_N: the R/M field of the ModR/M byte selects a packed-quadword
        AM_Q: the ModR/M byte follows the opcode and specifies the operand (mmx or mem)
        AM_R: the R/M field of the ModR/M byte may refer only to a general register
        AM_U: the R/M field of the ModR/M byte selects a 128-bit XMM register or a 256-bit YMM register
        AM_W: the ModR/M byte follows the opcode and specifies the operand (xmm, ymm or mem)
    */
    F_MODRM = (1 << 0),

    /*
        the following flags indicate that data is encoded in the instruction

        AM_I: the instruction contains immediate data
        AM_J: the instruction contains a relative offset
        AM_O: the offset of the operand is coded as a word or double word in the instruction
    */
    F_Z = (1 << 1), /* word for 16-bit operand-size or doubleword for 32 or 64-bit operand-size */
    F_B = (1 << 2), /* byte, regardless of operand-size attribute */

    /* disabled flags, will be handled in code with internal flags */

    F_O = F_NONE, /* the offset of the operand is coded as a word or double word (depending on address size) */
    F_V = F_NONE, /* word, doubleword or quadword (in 64-bit mode), depending on operand-size */
    F_W = F_NONE, /* word, regardless of operand-size attribute */
    F_D = F_NONE, /* doubleword, regardless of operand-size attribute */
    F_AP = F_NONE, /* direct address, 32-bit, 48-bit, or 80-bit pointer, depending on operand-size */

    /* internal flags */

    F_INT_O = (1 << 3),
    F_INT_V = (1 << 4),
    F_INT_W = (1 << 5),
    F_INT_D = (1 << 6),
    F_INT_AP = (1 << 7)
};

constexpr int TABLE_INDEX_ONEBYTE = (0 * 256);
constexpr int TABLE_INDEX_0F      = (1 * 256);
constexpr int TABLE_INDEX_0F38    = (2 * 256);
constexpr int TABLE_INDEX_0F3A    = (3 * 256);
constexpr int TABLE_INDEX_XOP_08  = (4 * 256);
constexpr int TABLE_INDEX_XOP_09  = (5 * 256);
constexpr int TABLE_INDEX_XOP_0A  = (6 * 256);

class Ldasm {
    x86Dcctx x86_dctx;
private:
    void    ConsumeBytes(size_t n);
    u8      GetModRM();
    u8      GetSib();
    void    GetOsize(u16 flags);
    void    GetAsize();

    void    DecodeSib(u8* disp_size);
    void    DecodeModRM();
    int     DecodePrefix();
    void    DecodeFlag(const u32 flags);
    void    DecodeInstruction();
    void    DecodeClear();
public:
	u8 DecodeInstructionLength(DasmMode dmode, const std::vector<u8>& memory, VirtualAddr addr);
    x86Dcctx& GetDecoderCtx() { return x86_dctx; }
};