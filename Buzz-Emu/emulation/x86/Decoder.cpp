
#include <array>

#include "Decoder.hpp"

void Ldasm::ConsumeBytes(size_t n) {
	x86_dctx.pos += n;
}

u8 Ldasm::GetModRM() {
	if (!x86_dctx.p_modrm) {
		x86_dctx.p_modrm = true;
		x86_dctx.pos_modrm = x86_dctx.pos;  /*if you want to get the opcode that contains ModR/M byte, then just subtract this with 0*/
		x86_dctx.modrm = x86_dctx.ip[x86_dctx.pos++];
	}

	return x86_dctx.modrm;
}

u8 Ldasm::GetSib() {
	if (!x86_dctx.p_sib)
	{
		x86_dctx.p_sib = true;
		x86_dctx.pos_sib = x86_dctx.pos;
		x86_dctx.sib = x86_dctx.ip[x86_dctx.pos++];
	}

	return x86_dctx.sib;
}

void Ldasm::GetOsize(u16 flags) {
   
    u8 p = x86_dctx.pfx_p_osize;

    if (x86_dctx.dmode == X86_Dmode_16bit)
        x86_dctx.osize = (p ? X86_Osize_32bit : X86_Osize_16bit);
    else if (x86_dctx.dmode == X86_Dmode_32bit)
        x86_dctx.osize = (p ? X86_Osize_16bit : X86_Osize_32bit);

    else // x86_dctx.dmode == X86_Dmode_64bit
    {
        if (flags & x86_Flag_F64)
            x86_dctx.osize = X86_Osize_64bit;
        else
        {
            if ((REX_W) ||
                (x86_dctx.pfx_p_vex3b && VEX_3B_W) ||
                (x86_dctx.pfx_p_xop && XOP_W))
            {
               
                x86_dctx.osize = X86_Osize_64bit;
            }
            else
            {
                if (flags & x86_Flag_D64)
                    x86_dctx.osize = (p ? X86_Osize_16bit : X86_Osize_64bit);
                else
                    x86_dctx.osize = (p ? X86_Osize_16bit : X86_Osize_32bit);
            }
        }
    }
}

void Ldasm::GetAsize() {
    /* address-size override prefix present */
    u8 p = x86_dctx.pfx_p_asize;

    if (x86_dctx.dmode == X86_Dmode_16bit)
    {
        /* assumed default address size: 16 bit */
        x86_dctx.asize = (p ? X86_Asize_32bit : X86_Asize_16bit);
    }
    else if (x86_dctx.dmode == X86_Dmode_32bit)
    {
        /* assumed default address size: 32 bit */
        x86_dctx.asize = (p ? X86_Asize_16bit : X86_Asize_32bit);
    }
    else // x86_dctx.dmode == X86_Dmode_64bit
    {
        /* assumed default address size: 64 bit */
        x86_dctx.asize = (p ? X86_Asize_32bit : X86_Asize_64bit);
    }
}

void Ldasm::DecodeSib(u8* disp_size){
    u8 base = SIB_BASE(GetSib());
    u8 mod = MODRM_MOD(GetModRM());

    if (x86_dctx.dmode == X86_Dmode_64bit)
    {
        if (x86_dctx.pfx_p_rex)
        {
            base |= (REX_B << 3);
        }
        else if (x86_dctx.pfx_p_vex3b)
        {
            base |= (VEX_3B_B << 3);
        }
        else if (x86_dctx.pfx_p_xop)
        {
            base |= (XOP_B << 3);
        }
    }

    /* effective address depends on addressing size */
    if (x86_dctx.asize == X86_Asize_32bit ||
        x86_dctx.asize == X86_Asize_64bit)
    {
        /* rbp, r13 */
        if (base == 5 || base == 13)
        {
            if (mod == 1)
                *disp_size = X86_Disp_8;
            else
                *disp_size = X86_Disp_32;
        }
    }
}

void Ldasm::DecodeModRM()
{
    u8 mod = MODRM_MOD(GetModRM());
    u8 rm = MODRM_RM(GetModRM());

    /* register-direct addressing mode */
    if (mod == 3)
    {
        return;
    }

    if (x86_dctx.dmode == X86_Dmode_64bit)
    {
        /*
            in long mode the REX.B/VEX.B/XOP.B fields extend the MODRM.RM
            field, and currently there can be no REX prefix if a VEX prefix
            is present (result is #UD)
        */
        if (x86_dctx.pfx_p_rex)
        {
            rm |= (REX_B << 3);
        }
        else
            if (x86_dctx.pfx_p_vex3b)
            {
                rm |= (VEX_3B_B << 3);
            }
            else
                if (x86_dctx.pfx_p_xop)
                {
                    rm |= (XOP_B << 3);
                }
    }

    /* mod = !11b, register-indirect addressing */
    u8 disp_size = X86_Disp_None;

    /* effective address depends on addressing size */
    if (x86_dctx.asize == X86_Asize_16bit)
    {
        /* REX.B/VEX.B/XOP.B is ignored */
        rm &= 7;

        if (mod == 0 && rm == 6)
        {
            disp_size = X86_Disp_16;
        }
        else
        {
            disp_size = (mod == 1 ? X86_Disp_8 : disp_size);
            disp_size = (mod == 2 ? X86_Disp_16 : disp_size);
        }
    }
    else /* X86_Asize_32bit, X86_ASIZE_64BIT */
    {
        /* REX.B/VEX.B/XOP.B is ignored */
        if (mod == 0 && (rm & 7) == 5)
        {
            disp_size = X86_Disp_32;
        }

        /* if EVEX: X86_Disp_8 * N, else... */
        disp_size = (mod == 1 ? X86_Disp_8 : disp_size);
        disp_size = (mod == 2 ? X86_Disp_32 : disp_size);

        /* REX.B/VEX.B/XOP.B is ignored */
        if ((rm & 7) == 4)
        {
            /* SIB byte follows the ModR/M byte */
            DecodeSib(&disp_size);
        }
    }

    /* consume the displacement */
    if (disp_size != X86_Disp_None)
    {
        /*
            X86_Disp_8  = 1
            X86_Disp_16 = 2
            X86_Disp_32 = 3
        */
        x86_dctx.pos_disp = x86_dctx.pos;
        x86_dctx.disp_size = 1 << (disp_size - 1);

        ConsumeBytes(x86_dctx.disp_size);
    }
}

int Ldasm::DecodePrefix() {
    size_t i;

    u8* p = x86_dctx.ip;

    /* loop only until max instruction length */
    for (i = 0; i < max_inst_length; i++)
    {
        /* check for REX prefix separately */
        if (REX_PREFIX(p[i]))
        {
            if (x86_dctx.dmode == X86_Dmode_64bit)
            {
                x86_dctx.pfx_last = p[i];
                x86_dctx.pfx_rex = p[i];
                x86_dctx.pos_rex = i;
                x86_dctx.pfx_p_rex = true;
            }
            else
            {
                /*
                    not in long mode, so the opcode must be an inc/dec
                    instruction and we are done decoding prefixes
                */
                break;
            }
        }
        else
        {
            /* check for all the other prefixes */
            if (p[i] == PREFIX_OSIZE)
            {
                x86_dctx.pfx_last = p[i];
                x86_dctx.pfx_p_osize = true;

                /*
                    if the prefixes F2h, F3h are present, then it seems
                    that the prefix 66h is ignored and between F2h and
                    F3h the one that comes later has precedence
                */
                if (x86_dctx.pfx_mandatory != PREFIX_REPNE &&
                    x86_dctx.pfx_mandatory != PREFIX_REP)
                {
                    x86_dctx.pfx_mandatory = p[i];
                }
            }
            else if (p[i] == PREFIX_ASIZE)
            {
                x86_dctx.pfx_last = p[i];
                x86_dctx.pfx_p_asize = true;
            }
            else if (p[i] == PREFIX_SEGOVRD_CS ||
                p[i] == PREFIX_SEGOVRD_DS ||
                p[i] == PREFIX_SEGOVRD_ES ||
                p[i] == PREFIX_SEGOVRD_SS)
            {
                x86_dctx.pfx_seg = p[i];

                /* these segment override pfxs are ignored in long mode */
                if (x86_dctx.dmode != X86_Dmode_64bit)
                {
                    /* last segment override prefix wins */
                    x86_dctx.pfx_last = p[i];
                    x86_dctx.pfx_p_seg = true;
                }
            }
            else if (p[i] == PREFIX_SEGOVRD_FS ||
                p[i] == PREFIX_SEGOVRD_GS)
            {
                x86_dctx.pfx_seg = p[i];

                /* last segment override prefix wins */
                x86_dctx.pfx_last = p[i];
                x86_dctx.pfx_p_seg = true;
            }
            else if (p[i] == PREFIX_LOCK)
            {
                x86_dctx.pfx_last = p[i];
                x86_dctx.pfx_p_lock = true;
            }
            else if (p[i] == PREFIX_REPNE)
            {
                x86_dctx.pfx_last = p[i];
                x86_dctx.pfx_mandatory = p[i];
                x86_dctx.pfx_p_repne = true;
            }
            else if (p[i] == PREFIX_REP)
            {
                x86_dctx.pfx_last = p[i];
                x86_dctx.pfx_mandatory = p[i];
                x86_dctx.pfx_p_rep = true;
            }
            else
            {
                break;
            }
        }
    }

    /* instruction's size exceeded the limit! */
    if (i == max_inst_length)
        return -1;

    /*
        REX prefix
    */
    if (x86_dctx.dmode == X86_Dmode_64bit)
    {
        /*
            REX prefix is only valid in long mode (64-bit mode) and if used,
            the REX prefix byte must immediately precede the opcode byte or
            the escape opcode byte (0FH). When a REX prefix is used in
            conjunction with an instruction containing a mandatory prefix,
            the mandatory prefix must come before the REX so the REX prefix
            can be immediately preceding the opcode or the escape byte. Other
            placements are ignored. The instruction-size limit of 15 bytes
            still applies to instructions with a REX prefix.
        */
        if (!REX_PREFIX(x86_dctx.pfx_last))
        {
            /* ignore it */
            x86_dctx.pfx_rex = 0;
            x86_dctx.pos_rex = 0;
            x86_dctx.pfx_p_rex = false;
        }
    }

    /*
        VEX, EVEX, XOP prefixes
    */
    const unsigned int vex_xop_tbl_selectors[] =
    {
        0,
        PREFIX_OSIZE,
        PREFIX_REP,
        PREFIX_REPNE
    };

    if (p[i] == PREFIX_VEX_3B && (x86_dctx.dmode == X86_Dmode_64bit ||
        VEX_3B_PM(p[i + 1])))
    {
        if (i > (max_inst_length - 5) ||
            x86_dctx.pfx_p_osize || x86_dctx.pfx_p_rex ||
            x86_dctx.pfx_p_lock || x86_dctx.pfx_p_rep ||
            x86_dctx.pfx_p_repne)
        {
            /*
                any VEX-encoded instruction with a REX, LOCK, 66H, F2H,
                or F3H prefix preceding VEX will #UD
            */
            return -1;
        }
        else
        {
            x86_dctx.pfx_vex[0] = p[i + 1]; /* P0 */
            x86_dctx.pfx_vex[1] = p[i + 2]; /* P1 */
            x86_dctx.pfx_p_vex3b = true;

            /* check if the m-mmmm value is invalid */
            if (VEX_3B_MMMMM == 0 || VEX_3B_MMMMM > 3)
            {
                return -1;
            }

            x86_dctx.table_index = (
                (VEX_3B_MMMMM) == 0 ? TABLE_INDEX_0F : (
                    (VEX_3B_MMMMM) == 1 ? TABLE_INDEX_0F : (
                        (VEX_3B_MMMMM) == 2 ? TABLE_INDEX_0F38 : TABLE_INDEX_0F3A)));

            x86_dctx.pfx_mandatory = vex_xop_tbl_selectors[VEX_3B_PP];

            /* update the position */
            i += 3;
        }
    }
    else if (p[i] == PREFIX_VEX_2B && (x86_dctx.dmode == X86_Dmode_64bit || VEX_2B_PM(p[i + 1])))
    {
        if (i > (max_inst_length - 4) ||
            x86_dctx.pfx_p_osize || x86_dctx.pfx_p_rex ||
            x86_dctx.pfx_p_lock || x86_dctx.pfx_p_rep ||
            x86_dctx.pfx_p_repne)
        {
            /*
                any VEX-encoded instruction with a REX, LOCK, 66H, F2H,
                or F3H prefix preceding VEX will #UD
            */
            return -1;
        }
        else
        {
            x86_dctx.pfx_vex[0] = p[i + 1]; /* P0 */
            x86_dctx.pfx_p_vex2b = true;

            x86_dctx.table_index = TABLE_INDEX_0F;
            x86_dctx.pfx_mandatory = vex_xop_tbl_selectors[VEX_2B_PP];

            /* update the position */
            i += 2;
        }
    }
    else if (p[i] == PREFIX_EVEX && (x86_dctx.dmode == X86_Dmode_64bit || EVEX_PM(p[i + 1])))
    {
        /*
            currently dont support EVEX prefix until better
            documentation is released...and someone cares enough to add it
        */
        return -1;
    }
    else if (p[i] == PREFIX_XOP && (x86_dctx.dmode == X86_Dmode_64bit || XOP_PM(p[i + 1])) && XOP_VALID(p[i + 1]))
    {
        if (i > (max_inst_length - 5) ||
            x86_dctx.pfx_p_osize || x86_dctx.pfx_p_rex ||
            x86_dctx.pfx_p_lock || x86_dctx.pfx_p_rep ||
            x86_dctx.pfx_p_repne)
        {
            /*
                any XOP-encoded instruction with a REX, LOCK, 66H, F2H,
                or F3H prefix preceding VEX will #UD
            */
            return -1;
        }
        else
        {
            x86_dctx.pfx_xop[0] = p[i + 1]; /* P0 */
            x86_dctx.pfx_xop[1] = p[i + 2]; /* P1 */
            x86_dctx.pfx_p_xop = true;

            /* check if the pp value is invalid */
            if (XOP_PP != 0 || (XOP_MMMMM - 8) > 2)
            {
                return -1;
            }

            x86_dctx.table_index = (
                (XOP_MMMMM - 8) == 0 ? TABLE_INDEX_XOP_08 : (
                    (XOP_MMMMM - 8) == 1 ? TABLE_INDEX_XOP_09 : TABLE_INDEX_XOP_0A));

            x86_dctx.pfx_mandatory = vex_xop_tbl_selectors[XOP_PP];

            /* update the position */
            i += 3;
        }
    }
    else if (p[i] == escape_opcode_2b) {
        if (p[i + 1] == escape_opcode_3b_1)
        {
            x86_dctx.table_index = TABLE_INDEX_0F38;

            /* update the position */
            i += 2;
        }
        else if (p[i + 1] == escape_opcode_3b_2)
        {
            x86_dctx.table_index = TABLE_INDEX_0F3A;

            /* update the position */
            i += 2;
        }
        else
        {
            x86_dctx.table_index = TABLE_INDEX_0F;

            /* update the position */
            i++;
        }
    }
    else {
        /* one-byte table */
        x86_dctx.pfx_mandatory = 0;
        x86_dctx.table_index = TABLE_INDEX_ONEBYTE;
    }

    /* update the position in the buffer */
    x86_dctx.pos = i;
    x86_dctx.pos_opcode = x86_dctx.pos;

    return i;
}

void Ldasm::DecodeFlag(const u32 flags) {
    /* decoding has to be sequential, as there may be multiple flags set */
    if (flags == F_NONE)
    {
        return;
    }

    if (flags & F_MODRM)
    {
        DecodeModRM();
    }

    if (flags & F_Z)
    {
        /* data encoded as word for 16-bit osize or dword for 32 or 64-bit osize */
        ConsumeBytes((x86_dctx.osize == X86_Osize_16bit ? 2 : 4));
    }

    if (flags & F_B)
    {
        /* byte, regardless of operand-size attribute */
        ConsumeBytes(1);
    }

    if (flags & F_INT_O)
    {
        /* the offset of the operand is coded as a word or double word (depending on address size) */
        ConsumeBytes((
            x86_dctx.asize == X86_Asize_16bit ? 2 : (
                x86_dctx.asize == X86_Asize_32bit ? 4 : 8)));
    }

    if (flags & F_INT_V)
    {
        /* word, doubleword or quadword (in 64-bit mode), depending on operand-size */
        ConsumeBytes((
            x86_dctx.osize == X86_Osize_16bit ? 2 : (
                x86_dctx.osize == X86_Osize_32bit ? 4 : 8)));
    }

    if (flags & F_INT_W)
    {
        /* word, regardless of operand-size attribute */
        ConsumeBytes(2);
    }

    if (flags & F_INT_D)
    {
        /* doubleword, regardless of operand-size attribute */
        ConsumeBytes(4);
    }

    if (flags & F_INT_AP)
    {
        /* direct address, 32-bit, 48-bit, or 80-bit pointer, depending on operand-size */
        ConsumeBytes((
            x86_dctx.osize == X86_Osize_16bit ? 2 : (
                x86_dctx.osize == X86_Osize_32bit ? 4 : 8)));

        ConsumeBytes(2);
    }
}

void Ldasm::DecodeInstruction() {
    volatile u8 table_compressed[] =
    {
        0x41, 0x14, 0x12, 0x20, 0x41, 0x14, 0x12, 0x20, 0x41, 0x14, 0x12, 0x20, 0x41, 0x14, 0x12, 0x20,
        0x41, 0x14, 0x12, 0x20, 0x41, 0x14, 0x12, 0x20, 0x41, 0x14, 0x12, 0x20, 0x41, 0x14, 0x12, 0xA4,
        0x00, 0x21, 0x40, 0x12, 0x13, 0x14, 0x15, 0x40, 0x90, 0x04, 0x15, 0x13, 0x25, 0x8C, 0x01, 0x98,
        0x00, 0x14, 0x12, 0x60, 0x88, 0x04, 0x88, 0x00, 0x25, 0x20, 0x21, 0x15, 0x13, 0x14, 0x40, 0x14,
        0x20, 0x41, 0x24, 0x20, 0x88, 0x01, 0x88, 0x04, 0x22, 0x10, 0x14, 0x8A, 0x00, 0x21, 0x60, 0x61,
        0x89, 0x00, 0x11, 0x10, 0x15, 0x94, 0x01, 0x40, 0x88, 0x01, 0x90, 0x00, 0xAC, 0x01, 0x20, 0x21,
        0x45, 0x31, 0x10, 0x21, 0x20, 0x41, 0x90, 0x02, 0x90, 0x01, 0x30, 0x11, 0x15, 0x11, 0x50, 0x11,
        0x15, 0x8D, 0x01, 0x15, 0x71, 0x15, 0x11, 0x35, 0x11, 0x88, 0x00, 0xC1, 0x01, 0x20, 0x15, 0x71,
        0x10, 0x31, 0x10, 0x61, 0x20, 0x9A, 0x01, 0x30, 0x31, 0x90, 0x00, 0x31, 0x9D, 0x00, 0x21, 0x60,
        0x31, 0x89, 0x00, 0x11, 0x10, 0x11, 0x10, 0x41, 0x20, 0x8A, 0x01, 0x60, 0x8A, 0x01, 0x60, 0x8A,
        0x01, 0x9B, 0x00, 0x51, 0x90, 0x00, 0x41, 0x10, 0x31, 0x88, 0x00, 0x35, 0x10, 0x35, 0x10, 0x88,
        0x05, 0x40, 0x65, 0x30, 0x15, 0x20, 0x35, 0x95, 0x00, 0x25, 0x60, 0x35, 0x10, 0x15, 0x10, 0x15,
        0x30, 0x31, 0x8F, 0x00, 0x41, 0x45, 0x40, 0x88, 0x01, 0x88, 0x00, 0x88, 0x01, 0xDF, 0x00, 0x15,
        0x90, 0x00, 0x15, 0xFF, 0x00, 0x95, 0x00, 0x31, 0x60, 0x21, 0x50, 0x31, 0x60, 0x21, 0x20, 0x21,
        0x20, 0x11, 0x8F, 0x00, 0x11, 0x89, 0x00, 0x45, 0x88, 0x00, 0x45, 0x9C, 0x00, 0x45, 0x91, 0x00,
        0x21, 0x8F, 0x00, 0x11, 0xED, 0x00, 0x41, 0x8C, 0x00, 0x8C, 0x01, 0xA5, 0x00, 0x31, 0x20, 0x21,
        0x30, 0x11, 0x50, 0x31, 0x20, 0x21, 0x30, 0x11, 0x50, 0x31, 0xAC, 0x00, 0x11, 0x10, 0x11, 0xFF,
        0x00, 0xEE, 0x00
    };

    size_t table_size = sizeof(table_compressed);

    /* fetch the opcode and update the position */
    uint8_t opcode = x86_dctx.ip[x86_dctx.pos++];

    uint32_t flags = 0;
    size_t table_pos = x86_dctx.table_index + opcode;

    for (size_t i = 0, j = 0; i < table_size; i++)
    {
        uint8_t d = table_compressed[i];
        uint8_t s = ((d & 0x80) == 0 ? (d >> 4) : (d & ~0x80));

        flags = ((d & 0x80) == 0 ? (d & 0x0F) : table_compressed[++i]);

        /* check if the opcode's position falls between the boundary */
        if (table_pos < j + s)
        {
            break;
        }

        j += s;
    }

    /* handle the special cases for which the flags were disabled in favor of a smaller table */
    if (x86_dctx.table_index == TABLE_INDEX_ONEBYTE)
    {
        /* call, jmp */
        if (opcode == 0xE8 || opcode == 0xE9)
        {
            flags = F_Z;

            /* update the operand size */
            GetOsize(x86_Flag_F64);
        }
        /* mov */
        else if (opcode >= 0xB8 && opcode <= 0xBF)
        {
            flags = F_INT_V;
        }
        /* mov */
        else if (opcode >= 0xA0 && opcode <= 0xA3)
        {
            flags = F_INT_O;
        }
        /* retn, retf */
        else if (opcode == 0xC2 || opcode == 0xCA)
        {
            flags = F_INT_W;
        }
        /* enter */
        else if (opcode == 0xC8)
        {
            flags = F_INT_W | F_B;
        }
        /* push */
        else if (opcode == 0x68)
        {
            flags = F_Z;

            /* update the operand size */
            GetOsize(x86_Flag_D64);
        }
        /* jmp far, call far */
        else if (opcode == 0xEA || opcode == 0x9A)
        {
            flags = F_INT_AP;
        }
        else if (opcode == 0xF6)
        {
            uint8_t reg = MODRM_REG(GetModRM());

            if (reg == 0 || reg == 1)
            {
                flags = F_MODRM | F_B;
            }
        }
        else if (opcode == 0xF7)
        {
            uint8_t reg = MODRM_REG(GetModRM());

            if (reg == 0 || reg == 1)
            {
                flags = F_MODRM | F_Z;
            }
        }
    }
    else if (x86_dctx.table_index == TABLE_INDEX_0F)
    {
        /* jcc */
        if (opcode >= 0x80 && opcode <= 0x8F)
        {
            flags = F_Z;

            /* update the operand size */
            GetOsize(x86_Flag_F64);
        }
        else if (opcode == 0x78)
        {
            /* insertq, extrq */
            if (x86_dctx.pfx_mandatory == PREFIX_REPNE ||
                x86_dctx.pfx_mandatory == PREFIX_OSIZE)
            {
                /* Vss, Uss, Ib, Ib */
                flags = F_MODRM | F_INT_W;
            }
        }
    }
    else if (x86_dctx.table_index == TABLE_INDEX_XOP_0A)
    {
        /* bextr, lwpins, lwpval */
        if (opcode == 0x10 || opcode == 0x12)
        {
            flags = F_MODRM | F_INT_D;
        }
    }

    DecodeFlag(flags);
}

void Ldasm::DecodeClear() {
    x86_dctx.pos = 0;
    x86_dctx.pos_modrm = 0;
    x86_dctx.pos_sib = 0;
    x86_dctx.pos_rex = 0;
    x86_dctx.pos_opcode = 0;
    x86_dctx.pos_disp = 0;

    x86_dctx.pfx_rex = 0;
    x86_dctx.pfx_last = 0;
    x86_dctx.pfx_seg = 0;
    x86_dctx.pfx_mandatory = 0;

    x86_dctx.pfx_p_rex = false;
    x86_dctx.pfx_p_seg = false;
    x86_dctx.pfx_p_osize = false;
    x86_dctx.pfx_p_asize = false;
    x86_dctx.pfx_p_lock = false;
    x86_dctx.pfx_p_rep = false;
    x86_dctx.pfx_p_repne = false;
    x86_dctx.pfx_p_vex2b = false;
    x86_dctx.pfx_p_vex3b = false;
    x86_dctx.pfx_p_xop = false;

    x86_dctx.p_vsib = false;
    x86_dctx.p_modrm = false;
    x86_dctx.p_sib = false;

    x86_dctx.table_index = ~0;
    x86_dctx.disp_size = 0;
}

u8 Ldasm::DecodeInstructionLength(DasmMode dmode, const std::vector<u8>& memory, VirtualAddr pc) {
    DecodeClear();

    x86_dctx.ip = (u8*)(memory.data() + pc);
    if (dmode > X86_Dmode_64bit) {
        x86_dctx.dmode = (
            dmode == 16 ? X86_Dmode_16bit : (
            dmode == 32 ? X86_Dmode_32bit : X86_Dmode_64bit
        ));
    }
    else {
        x86_dctx.dmode = dmode;
    }

    if (DecodePrefix() < 0)
        return -1;

    GetAsize();
    GetOsize(0);
    DecodeInstruction();
    return x86_dctx.len; 
}