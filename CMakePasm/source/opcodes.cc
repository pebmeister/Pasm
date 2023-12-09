// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

#include "opcodes.h"

cpu cpu_mode = cpu_6502;
int allow_illegal_op_codes = 0;

typedef struct instruction_look_up
{   
    int instruction;
    char* name;
} instruction_look_up;

typedef struct mode_look_up
{
    int mode;
    char* name;
} mode_look_up;

instruction_look_up instr_look_up[_maxOpcode] =
{
    {_ora, const_cast<char*>(static_cast<const char*>("ora"))},
    {_and, const_cast<char*>(static_cast<const char*>("and"))},
    {_eor, const_cast<char*>(static_cast<const char*>("eor"))},
    {_adc, const_cast<char*>(static_cast<const char*>("adc"))},
    {_sbc, const_cast<char*>(static_cast<const char*>("sbc"))},
    {_cmp, const_cast<char*>(static_cast<const char*>("cmp"))},
    {_cpx, const_cast<char*>(static_cast<const char*>("cpx"))},
    {_cpy, const_cast<char*>(static_cast<const char*>("cpy"))},
    {_dec, const_cast<char*>(static_cast<const char*>("dec"))},
    {_dex, const_cast<char*>(static_cast<const char*>("dex"))},
    {_dey, const_cast<char*>(static_cast<const char*>("dey"))},
    {_inc, const_cast<char*>(static_cast<const char*>("inc"))},
    {_inx, const_cast<char*>(static_cast<const char*>("inx"))},
    {_iny, const_cast<char*>(static_cast<const char*>("iny"))},
    {_asl, const_cast<char*>(static_cast<const char*>("asl"))},
    {_rol, const_cast<char*>(static_cast<const char*>("rol"))},
    {_lsr, const_cast<char*>(static_cast<const char*>("lsr"))},
    {_ror, const_cast<char*>(static_cast<const char*>("ror"))},
    {_lda, const_cast<char*>(static_cast<const char*>("lda"))},
    {_sta, const_cast<char*>(static_cast<const char*>("sta"))},
    {_ldx, const_cast<char*>(static_cast<const char*>("ldx"))},
    {_stx, const_cast<char*>(static_cast<const char*>("stx"))},
    {_ldy, const_cast<char*>(static_cast<const char*>("ldy"))},
    {_sty, const_cast<char*>(static_cast<const char*>("sty"))},
    {_rmb0, const_cast<char*>(static_cast<const char*>("rmb0"))},
    {_rmb1, const_cast<char*>(static_cast<const char*>("rmb1"))},
    {_rmb2, const_cast<char*>(static_cast<const char*>("rmb2"))},
    {_rmb3, const_cast<char*>(static_cast<const char*>("rmb3"))},
    {_rmb4, const_cast<char*>(static_cast<const char*>("rmb4"))},
    {_rmb5, const_cast<char*>(static_cast<const char*>("rmb5"))},
    {_rmb6, const_cast<char*>(static_cast<const char*>("rmb6"))},
    {_rmb7, const_cast<char*>(static_cast<const char*>("rmb7"))},
    {_smb0, const_cast<char*>(static_cast<const char*>("smb0"))},
    {_smb1, const_cast<char*>(static_cast<const char*>("smb1"))},
    {_smb2, const_cast<char*>(static_cast<const char*>("smb2"))},
    {_smb3, const_cast<char*>(static_cast<const char*>("smb3"))},
    {_smb4, const_cast<char*>(static_cast<const char*>("smb4"))},
    {_smb5, const_cast<char*>(static_cast<const char*>("smb5"))},
    {_smb6, const_cast<char*>(static_cast<const char*>("smb6"))},
    {_smb7, const_cast<char*>(static_cast<const char*>("smb7"))},
    {_stz, const_cast<char*>(static_cast<const char*>("stz"))},
    {_tax, const_cast<char*>(static_cast<const char*>("tax"))},
    {_txa, const_cast<char*>(static_cast<const char*>("txa"))},
    {_tay, const_cast<char*>(static_cast<const char*>("tay"))},
    {_tya, const_cast<char*>(static_cast<const char*>("tya"))},
    {_tsx, const_cast<char*>(static_cast<const char*>("tsx"))},
    {_txs, const_cast<char*>(static_cast<const char*>("txs"))},
    {_pla, const_cast<char*>(static_cast<const char*>("pla"))},
    {_pha, const_cast<char*>(static_cast<const char*>("pha"))},
    {_plp, const_cast<char*>(static_cast<const char*>("plp"))},
    {_php, const_cast<char*>(static_cast<const char*>("php"))},
    {_phx, const_cast<char*>(static_cast<const char*>("phx"))},
    {_phy, const_cast<char*>(static_cast<const char*>("phy"))},
    {_plx, const_cast<char*>(static_cast<const char*>("plx"))},
    {_ply, const_cast<char*>(static_cast<const char*>("ply"))},
    {_bra, const_cast<char*>(static_cast<const char*>("bra"))},
    {_bpl, const_cast<char*>(static_cast<const char*>("bpl"))},
    {_bmi, const_cast<char*>(static_cast<const char*>("bmi"))},
    {_bvc, const_cast<char*>(static_cast<const char*>("bvc"))},
    {_bvs, const_cast<char*>(static_cast<const char*>("bvs"))},
    {_bcc, const_cast<char*>(static_cast<const char*>("bcc"))},
    {_bcs, const_cast<char*>(static_cast<const char*>("bcs"))},
    {_bne, const_cast<char*>(static_cast<const char*>("bne"))},
    {_beq, const_cast<char*>(static_cast<const char*>("beq"))},
    {_bbr0, const_cast<char*>(static_cast<const char*>("bbr0"))},
    {_bbr1, const_cast<char*>(static_cast<const char*>("bbr1"))},
    {_bbr2, const_cast<char*>(static_cast<const char*>("bbr2"))},
    {_bbr3, const_cast<char*>(static_cast<const char*>("bbr3"))},
    {_bbr4, const_cast<char*>(static_cast<const char*>("bbr4"))},
    {_bbr5, const_cast<char*>(static_cast<const char*>("bbr5"))},
    {_bbr6, const_cast<char*>(static_cast<const char*>("bbr6"))},
    {_bbr7, const_cast<char*>(static_cast<const char*>("bbr7"))},
    {_bbs0, const_cast<char*>(static_cast<const char*>("bbs0"))},
    {_bbs1, const_cast<char*>(static_cast<const char*>("bbs1"))},
    {_bbs2, const_cast<char*>(static_cast<const char*>("bbs2"))},
    {_bbs3, const_cast<char*>(static_cast<const char*>("bbs3"))},
    {_bbs4, const_cast<char*>(static_cast<const char*>("bbs4"))},
    {_bbs5, const_cast<char*>(static_cast<const char*>("bbs5"))},
    {_bbs6, const_cast<char*>(static_cast<const char*>("bbs6"))},
    {_bbs7, const_cast<char*>(static_cast<const char*>("bbs7"))},
    {_stp, const_cast<char*>(static_cast<const char*>("stp"))},
    {_wai, const_cast<char*>(static_cast<const char*>("wai"))},
    {_brk, const_cast<char*>(static_cast<const char*>("brk"))},
    {_rti, const_cast<char*>(static_cast<const char*>("rti"))},
    {_jsr, const_cast<char*>(static_cast<const char*>("jsr"))},
    {_rts, const_cast<char*>(static_cast<const char*>("rts"))},
    {_jmp, const_cast<char*>(static_cast<const char*>("jmp"))},
    {_bit, const_cast<char*>(static_cast<const char*>("bit"))},
    {_trb, const_cast<char*>(static_cast<const char*>("trb"))},
    {_tsb, const_cast<char*>(static_cast<const char*>("tsb"))},
    {_clc, const_cast<char*>(static_cast<const char*>("clc"))},
    {_sec, const_cast<char*>(static_cast<const char*>("sec"))},
    {_cld, const_cast<char*>(static_cast<const char*>("cld"))},
    {_sed, const_cast<char*>(static_cast<const char*>("sed"))},
    {_cli, const_cast<char*>(static_cast<const char*>("cli"))},
    {_sei, const_cast<char*>(static_cast<const char*>("sei"))},
    {_clv, const_cast<char*>(static_cast<const char*>("clv"))},
    {_nop, const_cast<char*>(static_cast<const char*>("nop"))},
    {_slo, const_cast<char*>(static_cast<const char*>("slo"))},
    {_rla, const_cast<char*>(static_cast<const char*>("rla"))},
    {_sre, const_cast<char*>(static_cast<const char*>("sre"))},
    {_rra, const_cast<char*>(static_cast<const char*>("rra"))},
    {_sax, const_cast<char*>(static_cast<const char*>("sax"))},
    {_lax, const_cast<char*>(static_cast<const char*>("lax"))},
    {_dcp, const_cast<char*>(static_cast<const char*>("dcp"))},
    {_isc, const_cast<char*>(static_cast<const char*>("isc"))},
    {_anc, const_cast<char*>(static_cast<const char*>("anc"))},
    {_anc2, const_cast<char*>(static_cast<const char*>("anc2"))},
    {_alr, const_cast<char*>(static_cast<const char*>("alr"))},
    {_arr, const_cast<char*>(static_cast<const char*>("arr"))},
    {_xaa, const_cast<char*>(static_cast<const char*>("xaa"))},
    {_axs, const_cast<char*>(static_cast<const char*>("axs"))},
    {_usbc, const_cast<char*>(static_cast<const char*>("usbc"))},
    {_ahx, const_cast<char*>(static_cast<const char*>("ahx"))},
    {_shy, const_cast<char*>(static_cast<const char*>("shy"))},
    {_shx, const_cast<char*>(static_cast<const char*>("shx"))},
    {_tas, const_cast<char*>(static_cast<const char*>("tas"))},
    {_las, const_cast<char*>(static_cast<const char*>("las"))}
};

mode_look_up md_look_up[max_addressing_mode] =
{
    { i, const_cast<char*>(static_cast<const char*>("implied"))},
    { I, const_cast<char*>(static_cast<const char*>("immediate"))},
    { zp, const_cast<char*>(static_cast<const char*>("zero page"))},
    { zpi, const_cast<char*>(static_cast<const char*>("zero page indirect"))},
    { zpx, const_cast<char*>(static_cast<const char*>("zero page x"))},
    { zpy, const_cast<char*>(static_cast<const char*>("zero page y"))},
    { izx, const_cast<char*>(static_cast<const char*>("indirect zero page x"))},
    { izy, const_cast<char*>(static_cast<const char*>("indirect zero page y"))},
    { a, const_cast<char*>(static_cast<const char*>("absolute"))},
    { aix, const_cast<char*>(static_cast<const char*>("absolute i"))},
    { ax, const_cast<char*>(static_cast<const char*>("absolute x"))},
    { ay, const_cast<char*>(static_cast<const char*>("absolute y"))},
    { ind, const_cast<char*>(static_cast<const char*>("absolute i"))},
    { r, const_cast<char*>(static_cast<const char*>("relative"))},
    { A, const_cast<char*>(static_cast<const char*>("Accumulator"))}
};

//
// The table below was made using information from
// http://www.oxyron.de/html/opcodes02.html
//
static int ops6502[_maxOpcode][max_addressing_mode + 1] =
{
    //        i     I    zp   zpi   zpx   zpy    izx  izy     a   aix    ax    ay   ind     r     A
    { _ora,   -1, 0x09, 0x05,   -1, 0x15,   -1, 0x01, 0x11, 0x0D,   -1, 0x1D, 0x19,   -1,   -1,   -1},
    { _and,   -1, 0x29, 0x25,   -1, 0x35,   -1, 0x21, 0x31, 0x2D,   -1, 0x3D, 0x39,   -1,   -1,   -1},
    { _eor,   -1, 0x49, 0x45,   -1, 0x55,   -1, 0x41, 0x51, 0x4D,   -1, 0x5D, 0x59,   -1,   -1,   -1},
    { _adc,   -1, 0x69, 0x65,   -1, 0x75,   -1, 0x61, 0x71, 0x6D,   -1, 0x7D, 0x79,   -1,   -1,   -1},
    { _sbc,   -1, 0xE9, 0xE5,   -1, 0xF5,   -1, 0xE1, 0xF1, 0xED,   -1, 0xFD, 0xF9,   -1,   -1,   -1},
    { _cmp,   -1, 0xC9, 0xC5,   -1, 0xD5,   -1, 0xC1, 0xD1, 0xCD,   -1, 0xDD, 0xD9,   -1,   -1,   -1},
    { _cpx,   -1, 0xE0, 0xE4,   -1,   -1,   -1,   -1,   -1, 0xEC,   -1,   -1,   -1,   -1,   -1,   -1},
    { _cpy,   -1, 0xC0, 0xC4,   -1,   -1,   -1,   -1,   -1, 0xCC,   -1,   -1,   -1,   -1,   -1,   -1},
    { _dec,   -1,   -1, 0xC6,   -1, 0xD6,   -1,   -1,   -1, 0xCE,   -1, 0xDE,   -1,   -1,   -1,   -1},
    { _dex, 0xCA,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _dey, 0x88,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _inc,   -1,   -1, 0xE6,   -1, 0xF6,   -1,   -1,   -1, 0xEE,   -1, 0xFE,   -1,   -1,   -1,   -1},
    { _inx, 0xE8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _iny, 0xC8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _asl, 0x0A,   -1, 0x06,   -1, 0x16,   -1,   -1,   -1, 0x0E,   -1, 0x1E,   -1,   -1,   -1, 0x0A},
    { _rol, 0x2A,   -1, 0x26,   -1, 0x36,   -1,   -1,   -1, 0x2E,   -1, 0x3E,   -1,   -1,   -1, 0x2A},
    { _lsr, 0x4A,   -1, 0x46,   -1, 0x56,   -1,   -1,   -1, 0x4E,   -1, 0x5E,   -1,   -1,   -1, 0x4A},
    { _ror, 0x6A,   -1, 0x66,   -1, 0x76,   -1,   -1,   -1, 0x6E,   -1, 0x7E,   -1,   -1,   -1, 0x6A},

    // Move commands
    //         i     I    zp   zpi   zpx   zpy  izx  izy     a   aix    ax    ay     ind     r    A
    { _lda,   -1, 0xA9, 0xA5,   -1, 0xB5,   -1, 0xA1, 0xB1, 0xAD,   -1, 0xBD, 0xB9,   -1,   -1,   -1},
    { _sta,   -1,   -1, 0x85,   -1, 0x95,   -1, 0x81, 0x91, 0x8D,   -1, 0x9D, 0x99,   -1,   -1,   -1},
    { _ldx,   -1, 0xA2, 0xA6,   -1,   -1, 0xB6,   -1,   -1, 0xAE,   -1,   -1, 0xBE,   -1,   -1,   -1},
    { _stx,   -1,   -1, 0x86,   -1,   -1, 0x96,   -1,   -1, 0x8E,   -1,   -1,   -1,   -1,   -1,   -1},
    { _ldy,   -1, 0xA0, 0xA4,   -1, 0xB4,   -1,   -1,   -1, 0xAC,   -1, 0xBC,   -1,   -1,   -1,   -1},
    { _sty,   -1,   -1, 0x84,   -1, 0x94,   -1,   -1,   -1, 0x8C,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb0,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb1,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb2,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb3,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb4,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb5,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb6,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb7,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb0,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb1,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb2,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb3,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb4,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb5,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb6,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb7,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _stz,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _tax, 0xAA,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _txa, 0x8A,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _tay, 0xA8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _tya, 0x98,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _tsx, 0xBA,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _txs, 0x9A,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _pla, 0x68,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _pha, 0x48,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _plp, 0x28,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _php, 0x08,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _phx,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _phy,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _plx,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _ply,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},

    // Jump/Flag commands
    //    
    //         i     I    zp   zpi   zpx   zpy   izx   izy     a   aix    ax    ay   ind     r     A
    { _bra,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bpl,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x10,   -1},
    { _bmi,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x30,   -1},
    { _bvc,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x50,   -1},
    { _bvs,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x70,   -1},
    { _bcc,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x90,   -1},
    { _bcs,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xB0,   -1},
    { _bne,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xD0,   -1},
    { _beq,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xF0,   -1},
    { _bbr0,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbr1,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbr2,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbr3,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbr4,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbr5,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbr6,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbr7,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbs0,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbs1,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbs2,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbs3,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbs4,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbs5,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbs6,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _bbs7,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _stp,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _wai,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _brk, 0x00,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rti, 0x40,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _jsr,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x20,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rts, 0x60,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _jmp,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x4C,   -1,   -1,   -1, 0x6C,   -1,   -1},
    { _bit,   -1,   -1, 0x24,   -1,   -1,   -1,   -1,   -1, 0x2C,   -1,   -1,   -1,   -1,   -1,   -1},
    { _trb,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _tsb,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _clc, 0x18,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _sec, 0x38,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _cld, 0xD8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _sed, 0xF8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _cli, 0x58,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _sei, 0x78,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _clv, 0xB8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _nop, 0xEA,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},

    // These are the illegal opcodes
    //
    //         i     I    zp   zpi   zpx   zpy  izx  izy     a    aix    ax    ay   ind     r      A
    { _slo,   -1,   -1, 0x07,   -1, 0x17,   -1, 0x03, 0x13, 0x0F,   -1, 0x1F, 0x1B,   -1,   -1,   -1 },
    { _rla,   -1,   -1, 0x27,   -1, 0x37,   -1, 0x23, 0x33, 0x2F,   -1, 0x3F, 0x3B,   -1,   -1,   -1 },
    { _sre,   -1,   -1, 0x47,   -1, 0x57,   -1, 0x43, 0x53, 0x4F,   -1, 0x5F, 0x5B,   -1,   -1,   -1 },
    { _rra,   -1,   -1, 0x67,   -1, 0x77,   -1, 0x63, 0x73, 0x6F,   -1, 0x7F, 0x7B,   -1,   -1,   -1 },
    { _sax,   -1,   -1, 0x87,   -1,   -1, 0x97, 0x83,   -1, 0x8F,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _lax,   -1, 0xAB, 0xA7,   -1,   -1, 0xB7, 0xA3, 0xB3, 0xAF,   -1,   -1, 0xBF,   -1,   -1,   -1 },
    { _dcp,   -1,   -1, 0xC7,   -1, 0xD7,   -1, 0xC3, 0xD3, 0xCF,   -1, 0xDF, 0xDB,   -1,   -1,   -1 },
    { _isc,   -1,   -1, 0xE7,   -1, 0xF7,   -1, 0xE3, 0xF3, 0xEF,   -1, 0xFF, 0xFB,   -1,   -1,   -1 },
    { _anc,   -1, 0x0B,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _anc2,  -1, 0x2B,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _alr,   -1, 0x4B,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _arr,   -1, 0x6B,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _xaa,   -1, 0x8B,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _axs,   -1, 0xCB,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _usbc,  -1, 0xEB,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _ahx,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x93,   -1,   -1,   -1, 0x9F,   -1,   -1,   -1 },
    { _shy,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x9C,   -1,   -1,   -1,   -1 },
    { _shx,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x9E,   -1,   -1,   -1 },
    { _tas,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x9B,   -1,   -1,   -1 },
    { _las,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xBB,   -1,   -1,   -1 }
};

//
// 65C02 cpu
//
// ReSharper disable once CppInconsistentNaming
static int ops65c02[_maxOpcode][max_addressing_mode + 1] =
{
    // Logical and arithmetic commands
    //         i     I    zp   zpi   zpx   zpy  izx  izy     a   aix    ax    ay   ind      r     A
    { _ora,   -1, 0x09, 0x05,   -1, 0x15,   -1, 0x01, 0x11, 0x0D,   -1, 0x1D, 0x19, 0x12,   -1,   -1},
    { _and,   -1, 0x29, 0x25,   -1, 0x35,   -1, 0x21, 0x31, 0x2D,   -1, 0x3D, 0x39, 0x32,   -1,   -1},
    { _eor,   -1, 0x49, 0x45,   -1, 0x55,   -1, 0x41, 0x51, 0x4D,   -1, 0x5D, 0x59, 0x52,   -1,   -1},
    { _adc,   -1, 0x69, 0x65,   -1, 0x75,   -1, 0x61, 0x71, 0x6D,   -1, 0x7D, 0x79, 0x72,   -1,   -1},
    { _sbc,   -1, 0xE9, 0xE5,   -1, 0xF5,   -1, 0xE1, 0xF1, 0xED,   -1, 0xFD, 0xF9, 0xF2,   -1,   -1},
    { _cmp,   -1, 0xC9, 0xC5,   -1, 0xD5,   -1, 0xC1, 0xD1, 0xCD,   -1, 0xDD, 0xD9, 0xD2,   -1,   -1},
    { _cpx,   -1, 0xE0, 0xE4,   -1,   -1,   -1,   -1,   -1, 0xEC,   -1,   -1,   -1,   -1,   -1,   -1},
    { _cpy,   -1, 0xC0, 0xC4,   -1,   -1,   -1,   -1,   -1, 0xCC,   -1,   -1,   -1,   -1,   -1,   -1},
    { _dec, 0x3A,   -1, 0xC6,   -1, 0xD6,   -1,   -1,   -1, 0xCE,   -1, 0xDE,   -1,   -1,   -1, 0x3A},
    { _dex, 0xCA,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _dey, 0x88,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _inc, 0x1A,   -1, 0xE6,   -1, 0xF6,   -1,   -1,   -1, 0xEE,   -1, 0xFE,   -1,   -1,   -1, 0x1A},
    { _inx, 0xE8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _iny, 0xC8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _asl, 0x0A,   -1, 0x06,   -1, 0x16,   -1,   -1,   -1, 0x0E,   -1, 0x1E,   -1,   -1,   -1, 0x0A},
    { _rol, 0x2A,   -1, 0x26,   -1, 0x36,   -1,   -1,   -1, 0x2E,   -1, 0x3E,   -1,   -1,   -1, 0x2A},
    { _lsr, 0x4A,   -1, 0x46,   -1, 0x56,   -1,   -1,   -1, 0x4E,   -1, 0x5E,   -1,   -1,   -1, 0x4A},
    { _ror, 0x6A,   -1, 0x66,   -1, 0x76,   -1,   -1,   -1, 0x6E,   -1, 0x7E,   -1,   -1,   -1, 0x6A},

    // Move commands
    //         i     I    zp   zpi   zpx   zpy  izx   izy     a    aix    ax    ay   ind     r     A
    { _lda,   -1, 0xA9, 0xA5,   -1, 0xB5,   -1, 0xA1, 0xB1, 0xAD,   -1, 0xBD, 0xB9, 0xB2,   -1,   -1},
    { _sta,   -1,   -1, 0x85,   -1, 0x95,   -1, 0x81, 0x91, 0x8D,   -1, 0x9D, 0x99, 0x92,   -1,   -1},
    { _ldx,   -1, 0xA2, 0xA6,   -1,   -1, 0xB6,   -1,   -1, 0xAE,   -1,   -1, 0xBE,   -1,   -1,   -1},
    { _stx,   -1,   -1, 0x86,   -1,   -1, 0x96,   -1,   -1, 0x8E,   -1,   -1,   -1,   -1,   -1,   -1},
    { _ldy,   -1, 0xA0, 0xA4,   -1, 0xB4,   -1,   -1,   -1, 0xAC,   -1, 0xBC,   -1,   -1,   -1,   -1},
    { _sty,   -1,   -1, 0x84,   -1, 0x94,   -1,   -1,   -1, 0x8C,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb0,  -1,   -1, 0x07,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb1,  -1,   -1, 0x17,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb2,  -1,   -1, 0x27,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb3,  -1,   -1, 0x37,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb4,  -1,   -1, 0x47,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb5,  -1,   -1, 0x57,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb6,  -1,   -1, 0x67,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rmb7,  -1,   -1, 0x77,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb0,  -1,   -1, 0x87,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb1,  -1,   -1, 0x97,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb2,  -1,   -1, 0xA7,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb3,  -1,   -1, 0xB7,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb4,  -1,   -1, 0xC7,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb5,  -1,   -1, 0xD7,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb6,  -1,   -1, 0xE7,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _smb7,  -1,   -1, 0xF7,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _stz,   -1,   -1, 0x64,   -1, 0x74,   -1,   -1,   -1, 0x9C,   -1, 0x9E,   -1,   -1,   -1,   -1},
    { _tax, 0xAA,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _txa, 0x8A,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _tay, 0xA8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _tya, 0x98,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _tsx, 0xBA,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _txs, 0x9A,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _pla, 0x68,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _pha, 0x48,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _plp, 0x28,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _php, 0x08,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _phx, 0xDA,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _phy, 0x5A,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _plx, 0xFA,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _ply, 0x7A,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},

    // Jump/Flag commands
    //    
    //         i     I    zp   zpi   zpx   zpy  izx    izy     a   aix    ax    ay   ind     r     A
    { _bra,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x80,   -1},
    { _bpl,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x10,   -1},
    { _bmi,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x30,   -1},
    { _bvc,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x50,   -1},
    { _bvs,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x70,   -1},
    { _bcc,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x90,   -1},
    { _bcs,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xB0,   -1},
    { _bne,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xD0,   -1},
    { _beq,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xF0,   -1},
    { _bbr0,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x0F,   -1},
    { _bbr1,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x1F,   -1},
    { _bbr2,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x2F,   -1},
    { _bbr3,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x3F,   -1},
    { _bbr4,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x4F,   -1},
    { _bbr5,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x5F,   -1},
    { _bbr6,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x6F,   -1},
    { _bbr7,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x7F,   -1},
    { _bbs0,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x8F,   -1},
    { _bbs1,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x9F,   -1},
    { _bbs2,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xAF,   -1},
    { _bbs3,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xBF,   -1},
    { _bbs4,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xCF,   -1},
    { _bbs5,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xDF,   -1},
    { _bbs6,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xEF,   -1},
    { _bbs7,  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xFF,   -1},
    { _stp, 0xDB,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _wai, 0xCB,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _brk, 0x00,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rti, 0x40,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _jsr,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x20,   -1,   -1,   -1,   -1,   -1,   -1},
    { _rts, 0x60,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _jmp,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x4C, 0x7C,   -1,   -1, 0x6C,   -1,   -1},
    { _bit,   -1, 0x89, 0x24,   -1, 0x34,   -1,   -1,   -1, 0x2C,   -1, 0x3C,   -1,   -1,   -1,   -1},
    { _trb,   -1,   -1, 0x14,   -1,   -1,   -1,   -1,   -1, 0x1C,   -1,   -1,   -1,   -1,   -1,   -1},
    { _tsb,   -1,   -1, 0x04,   -1,   -1,   -1,   -1,   -1, 0x0C,   -1,   -1,   -1,   -1,   -1,   -1},
    { _clc, 0x18,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _sec, 0x38,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _cld, 0xD8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _sed, 0xF8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _cli, 0x58,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _sei, 0x78,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _clv, 0xB8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
    { _nop, 0xEA,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},

    // These are the illegal opcodes
    //
    //         i     I    zp   zpi   zpx   zpy  izx   izy     a    aix   ax    ay    ind     r     A
    { _slo,   -1,   -1, 0x07,   -1, 0x17,   -1, 0x03, 0x13, 0x0F,   -1, 0x1F, 0x1B,   -1,   -1,   -1 },
    { _rla,   -1,   -1, 0x27,   -1, 0x37,   -1, 0x23, 0x33, 0x2F,   -1, 0x3F, 0x3B,   -1,   -1,   -1 },
    { _sre,   -1,   -1, 0x47,   -1, 0x57,   -1, 0x43, 0x53, 0x4F,   -1, 0x5F, 0x5B,   -1,   -1,   -1 },
    { _rra,   -1,   -1, 0x67,   -1, 0x77,   -1, 0x63, 0x73, 0x6F,   -1, 0x7F, 0x7B,   -1,   -1,   -1 },
    { _sax,   -1,   -1, 0x87,   -1,   -1, 0x97, 0x83,   -1, 0x8F,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _lax,   -1,   -1, 0xA7,   -1,   -1, 0xB7, 0xA3, 0xB3, 0xAF,   -1,   -1, 0xBF,   -1,   -1,   -1 },
    { _dcp,   -1,   -1, 0xC7,   -1, 0xD7,   -1, 0xC3, 0xD3, 0xCF,   -1, 0xDF, 0xDB,   -1,   -1,   -1 },
    { _isc,   -1,   -1, 0xE7,   -1, 0xF7,   -1, 0xE3, 0xF3, 0xEF,   -1, 0xFF, 0xFB,   -1,   -1,   -1 },
    { _anc,   -1, 0x0B,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _anc2,  -1, 0x2B,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _alr,   -1, 0x4B,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _arr,   -1, 0x6B,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _xaa,   -1, 0x8B,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _axs,   -1, 0xCB,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _usbc,  -1, 0xEB,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
    { _ahx,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x93,   -1,   -1,   -1, 0x9F,   -1,   -1,   -1 },
    { _shy,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x9C,   -1,   -1,   -1,   -1 },
    { _shx,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x9E,   -1,   -1,   -1 },
    { _tas,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x9B,   -1,   -1,   -1 },
    { _las,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 0xBB,   -1,   -1,   -1 }
};


/**
 * \brief get the opcode for an instruction and addressing mode
 * \param instruction instruction to get the opcode for
 * \param addressing_mode addressing mode of instruction
 * \return opcode or -1 if not found
 */
int get_op_code(const int instruction, const int addressing_mode)
{

    if ((instruction >= 0 && instruction < _maxOpcode) &&
        (addressing_mode >= 0 && addressing_mode < max_addressing_mode))
    {
        if (!allow_illegal_op_codes && instruction >= _illegalStart && cpu_mode == cpu_6502)
            return -1;

        switch (cpu_mode)  // NOLINT(hicpp-multiway-paths-covered)
        {
        case cpu_6502:
            return ops6502[instruction][addressing_mode + 1];

        case cpu_65C02:
            return ops65c02[instruction][addressing_mode + 1];
        }
    }
    return -1;
}


/**
 * \brief Get string representation of instruction
 * \param instruction instruction to get the string for
 * \return string or "Unknown" if not found
 */
char* instruction_to_string(const int instruction)
{
    if (instruction < _maxOpcode)
        return instr_look_up[instruction].name;
    return (char*)"Unknown";
}

/// <summary>
/// Mode to string.
/// </summary>
/// <param name="mode">The addressing mode.</param>
/// <returns>char *.</returns>
char* mode_to_string(const int mode)
{
    if (mode < max_addressing_mode)
        return md_look_up[mode].name;
    return (char*)"Unknown";
}
