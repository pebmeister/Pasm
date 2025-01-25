#include <gtest/gtest.h>
#include "unit_test.h"
#include "parseargs.h"
#include "pass.h"

ops_ptr cpu_6502_illegal_ops = new ops[21]
{
    {
        _slo,
        7,
        new int[7] { zp, zpx, izx, izy, a, ax, ay },
        new int [7] { 0x07, 0x17, 0x03, 0x13, 0x0F, 0x1F, 0x1B }
    },
    {
        _rla,
        7,
        new int[7] { zp, zpx, izx, izy, a, ax, ay },
        new int [7] { 0x27, 0x37, 0x23, 0x33, 0x2F, 0x3F, 0x3B }
    },
    {
        _sre,
        7,
        new int[7] { zp, zpx, izx, izy, a, ax, ay },
        new int [7] { 0x47, 0x57, 0x43, 0x53, 0x4F, 0x5F, 0x5B }
    },
    {
        _rra,
        7,
        new int[7] { zp, zpx, izx, izy, a, ax, ay },
        new int [7] { 0x67, 0x77, 0x63, 0x73, 0x6F, 0x7F, 0x7B }
    },
    {
        _sax,
        4,
        new int[4] { zp, zpy, izx, a },
        new int [4] { 0x87, 0x97, 0x83, 0x8F }
    },
    {
        _lax,
        7,
        new int[7] { zp, I, zpy, izx, izy, a, ay },
        new int[7] { 0xA7, 0xAB, 0xB7, 0xA3, 0xB3, 0xAF, 0xBF}
    },
    {
        _dcp,
        7,
        new int[7] { zp, zpx, izx, izy, a, ax, ay },
        new int[7] { 0xC7, 0xD7, 0xC3, 0xD3, 0xCF, 0xDF, 0xDB }
    },
    {
        _isc,
        7,
        new int[7] { zp, zpx, izx, izy, a, ax, ay },
        new int[7] { 0xE7, 0xF7, 0xE3, 0xF3, 0xEf, 0xFF, 0xFB }
    },
    {
        _anc,
        1,
        new int[1] { I },
        new int[1] { 0x0B }
    },
    {
        _anc2,
        1,
        new int[1] { I },
        new int[1] { 0x2B }
    },
    {
        _alr,
        1,
        new int[1] { I },
        new int[1] { 0x4B }
    },
    {
        _arr,
        1,
        new int[1] { I },
        new int[1] { 0x6B }
    },
    {
        _xaa,
        1,
        new int[1] { I },
        new int[1] { 0x8B }
    },
    {
        _usbc,
        1,
        new int[1] { I },
        new int[1] { 0xEB }
    },
    {
        _ahx,
        2,
        new int[2] { izy, ay },
        new int[2] { 0x93, 0x9F }
    },
    {
        _shy,
        1,
        new int[2] { ax },
        new int[2] { 0x9C }
    },
    {
        _shx,
        1,
        new int[2] { ay },
        new int[2] { 0x9E }
    },
    {
        _tas,
        1,
        new int[2] { ay },
        new int[2] { 0x9B }
    },
    {
        _las,
        1,
        new int[2] { ay },
        new int[2] { 0xBB }
    },

    /////////////////////////////////////////////

    {
    _maxOpcode,
    0,
           nullptr,
           nullptr
    }
};

// ReSharper disable once CppInconsistentNaming
ops_ptr cpu_65c02_ops = new ops[55]
{
    {
        _bra,
        1,
        new int[1] { r },
        new int[1] { 0x80 }
    },
    {
        _phx,
        1,
        new int[1] { i },
        new int[1] { 0xDA },
    },
    {
        _phy,
        1,
        new int[1] { i },
        new int[1] { 0x5A },
    },
    {
        _plx,
        1,
        new int[1] { i },
        new int[1] { 0xFA },
    },
    {
        _ply,
        1,
        new int[1] { i },
        new int[1] { 0x7A },
    },
    {
        _stz,
        4,
        new int[4] { zp, zpx, a, ax },
        new int[4] { 0x64, 0x74, 0x9C, 0x9E },
    },
    {
        _trb,
        2,
        new int[2] { zp, a },
        new int [2] { 0x14, 0x1C }
    },
    {
        _tsb,
        2,
        new int[2] { zp, a },
        new int [2] { 0x04, 0x0C }
    },
    {
        _bbr0,
        1,
        new int[1] { r },
        new int [1] { 0x0F }
    },
    {
        _bbr1,
        1,
        new int[1] { r },
        new int [1] { 0x1F }
    },
    {
        _bbr2,
        1,
        new int[1] { r },
        new int [1] { 0x2F }
    },
    {
        _bbr3,
        1,
        new int[1] { r },
        new int [1] { 0x3F }
    },
    {
        _bbr4,
        1,
        new int[1] { r },
        new int [1] { 0x4F }
    },
    {
        _bbr5,
        1,
        new int[1] { r },
        new int [1] { 0x5F }
    },
    {
        _bbr6,
        1,
        new int[1] { r },
        new int [1] { 0x6F }
    },
    {
        _bbr7,
        1,
        new int[1] { r },
        new int [1] { 0x7F }
    },
    {
        _bbs0,
        1,
        new int[1] { r },
        new int [1] { 0x8F }
    },
    {
        _bbs1,
        1,
        new int[1] { r },
        new int [1] { 0x9F }
    },
    {
        _bbs2,
        1,
        new int[1] { r },
        new int [1] { 0xAF }
    },
    {
        _bbs3,
        1,
        new int[1] { r },
        new int [1] { 0xBF }
    },
    {
        _bbs4,
        1,
        new int[1] { r },
        new int [1] { 0xCF }
    },
    {
        _bbs5,
        1,
        new int[1] { r },
        new int [1] { 0xDF }
    },
    {
        _bbs6,
        1,
        new int[1] { r },
        new int [1] { 0xEF }
    },
    {
        _bbs7,
        1,
        new int[1] { r },
        new int [1] { 0xFF }
    },
    {
        _rmb0,
        1,
        new int[1] { zp },
        new int [1] { 0x07 }
    },
    {
        _rmb1,
        1,
        new int[1] { zp },
        new int [1] { 0x17 }
    },
    {
        _rmb2,
        1,
        new int[1] { zp },
        new int [1] { 0x27 }
    },
    {
        _rmb3,
        1,
        new int[1] { zp },
        new int [1] { 0x37 }
    },
    {
        _rmb4,
        1,
        new int[1] { zp },
        new int [1] { 0x47 }
    },
    {
        _rmb5,
        1,
        new int[1] { zp },
        new int [1] { 0x57 }
    },
    {
        _rmb6,
        1,
        new int[1] { zp },
        new int [1] { 0x67 }
    },
    {
        _rmb7,
        1,
        new int[1] { zp },
        new int [1] { 0x77 }
    },
    {
        _smb0,
        1,
        new int[1] { zp },
        new int [1] { 0x87 }
    },
    {
        _smb1,
        1,
        new int[1] { zp },
        new int [1] { 0x97 }
    },
    {
        _smb2,
        1,
        new int[1] { zp },
        new int [1] { 0xA7 }
    },
    {
        _smb3,
        1,
        new int[1] { zp },
        new int [1] { 0xB7 }
    },
    {
        _smb4,
        1,
        new int[1] { zp },
        new int [1] { 0xC7 }
    },
    {
        _smb5,
        1,
        new int[1] { zp },
        new int [1] { 0xD7 }
    },
    {
        _smb6,
        1,
        new int[1] { zp },
        new int [1] { 0xE7 }
    },
    {
        _smb7,
        1,
        new int[1] { zp },
        new int [1] { 0xF7 }
    },
    {
        _wai,
        1,
        new int[1] { i },
        new int[1] { 0xCB }
    },
     {
         _stp,
         1,
         new int[1] { i },
         new int[1] { 0xDB }
    },
    {
        _adc,
        9,
        new int[9] { I, zp, zpx, a, ax, ay, izx, izy, ind },
        new int[9] { 0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71, 0x72}
    },
    {
        _and,
        9,
        new int[9] { I, zp, zpx, a, ax, ay, izx, izy, ind },
            new int[9] { 0x29, 0x25, 0x35, 0x2D, 0x3D, 0x39, 0x21, 0x31, 0x32 }
    },
    {
        _bit,
        5,
        new int[5] { I, zp, zpx, a, ax },
        new int[5] { 0x89, 0x24, 0x34, 0x2C, 0x3C }
    },
    {
        _cmp,
        9,
        new int[9] { I, zp, zpx, a, ax, ay, izx, izy, ind },
        new int[9] { 0xC9, 0xC5, 0xD5, 0xCD, 0xDD, 0xD9, 0xC1, 0xD1, 0xD2 }
    },
    {
        _dec,
        6,
        new int[6] { A, i, zp, zpx, a, ax },
        new int[6] { 0x3A, 0x3A, 0xC6, 0xD6, 0xCE, 0xDE }
    },
    {
        _eor,
        9,
        new int[9] { I, zp, zpx, a, ax, ay, izx, izy, ind },
        new int[9] { 0x49, 0x45, 0x55, 0x4D, 0x5D, 0x59, 0x41, 0x51, 0x52 }
    },
    {
        _inc,
        6,
        new int[6] { A, i, zp, zpx, a, ax },
        new int[6] { 0x1A, 0x1A, 0xE6, 0xF6, 0xEE, 0xFE }
    },
    {
        _jmp,
        3,
        new int[3] { a, ind, aix },
        new int[3] { 0x4C, 0x6C, 0x7C }
    },
    {
        _lda,
        9,
        new int[9] { I, zp, zpx, a, ax, ay, izx, izy, ind },
        new int[9] { 0xA9, 0xA5, 0xB5, 0xAD, 0xBD, 0xB9, 0xA1, 0xB1, 0xB2 }
    },
    {
        _ora,
        9,
        new int[9] { I, zp, zpx, a, ax, ay, izx, izy, ind },
        new int[9] { 0x09, 0x05, 0x15, 0x0D, 0x1D, 0x19, 0x01, 0x11, 0x12 }
    },
    {
        _sbc,
        9,
        new int[9] { I, zp, zpx, a, ax, ay, izx, izy, ind },
        new int[9] { 0xE9, 0xE5, 0xF5, 0xED, 0xFD, 0xF9 , 0xE1, 0xF1, 0xF2  }
    },
    {
        _sta,
        8,
        new int[8] { zp, zpx, a, ax, ay, izx, izy, ind },
        new int[8] { 0x85, 0x95, 0x8D, 0x9D, 0x99, 0x81, 0x91, 0x92 }
    },

    /////////////////////////////////////////////

    {
    _maxOpcode,
    0,
           nullptr,
           nullptr
    }
};

ops_ptr cpu_6502_ops = new ops[56]
{
    {
        _adc,
        8,
        new int[8] { I, zp, zpx, a, ax, ay, izx, izy },
        new int[8] { 0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71}
    },
    {
        _and, 
        8,
        new int[8] { I, zp, zpx, a, ax, ay, izx, izy },
        new int[8] { 0x29, 0x25, 0x35, 0x2D, 0x3D, 0x39, 0x21, 0x31 }
    },
    {
        _asl,
        6,
        new int[6] { i, A, zp, zpx, a, ax },
        new int [6] { 0x0A, 0x0A, 0x06, 0x16, 0x0E, 0x1E }
    },
    {
        _bcc,
        1,
        new int[1] { r },
        new int[1] { 0x90 },
    },
    {
        _bcs,
        1,
        new int[1] { r },
        new int[1] { 0xB0 }
    },
    {
        _beq,
        1,
        new int[1] { r },
        new int[1] { 0xF0 }
    },
    {
        _bit,
        2,
        new int[2] { zp, a },
        new int[2] { 0x24, 0x2C }
    },
    {
        _bmi,
        1,
        new int[1] { r },
        new int[1] { 0x30 }
    },
    {
        _bne,
        1,
        new int[1] { r },
        new int[1] { 0xD0 }
    },
    {
        _bpl,
        1,
        new int[1] { r },
        new int[1] { 0x10 }
    },
    {
        _brk,
        1,
        new int[1] { i },
        new int[1] { 0x00 }
    },
    {
        _bvc,
        1,
        new int[1] { r },
        new int[1] { 0x50 }
    },
    {
        _bvs,
        1,
        new int[1] { r },
        new int[1] { 0x70 }
    },
    {
        _clc,
        1,
        new int[1] { i },
        new int[1] { 0x18 }
    },
    {
        _cld,
        1,
        new int[1] { i },
        new int[1] { 0xD8 }
    },
    {
        _cli,
        1,
        new int[1] { i },
        new int[1] { 0x58 }
    },
    {
        _clv,
        1,
        new int[1] { i },
        new int[1] { 0xB8 }
    },
    {
        _cmp,
        8,
        new int[8] { I, zp, zpx, a, ax, ay, izx, izy },
        new int[8] { 0xC9, 0xC5, 0xD5, 0xCD, 0xDD, 0xD9, 0xC1, 0xD1 }
    },
    {
        _cpx,
        3,
        new int[3] { I, zp, a },
        new int[3] { 0xE0, 0xE4, 0xEC }
    },

    {
        _cpy,
        3,
        new int[3] { I, zp, a },
        new int[3] { 0xC0, 0xC4, 0xCC }
    },
    {
        _dec,
        4,
        new int[4] { zp, zpx, a, ax },
        new int[4] { 0xC6, 0xD6, 0xCE, 0xDE }
    },
    {
        _dex,
        1,
        new int[1] { i },
        new int[1] { 0xCA }
    },
    {
        _dey,
        1,
        new int[1] { i },
        new int[1] { 0x88 }
    },
    {
        _eor,
        8,
        new int[8] { I, zp, zpx, a, ax, ay, izx, izy },
        new int[8] { 0x49, 0x45, 0x55, 0x4D, 0x5D, 0x59, 0x41, 0x51 }
    },
    {
        _inc,
        4,
        new int[4] { zp, zpx, a, ax },
        new int[4] { 0xE6, 0xF6, 0xEE, 0xFE }
    },
    {
        _inx,
        1,
        new int[1] { i },
        new int[1] { 0xE8}
    },
    {
        _iny,
        1,
        new int[1] { i },
        new int[1] { 0xC8}
    },
    {
        _jmp,
        2,
        new int[2] { a, ind },
        new int[2] { 0x4C, 0x6C }
    },
    {
        _jsr,
        1,
        new int[1] { a  },
        new int[1] { 0x20 }
    },
    {
        _lda,
        8,
        new int[8] { I, zp, zpx, a, ax, ay, izx, izy },
        new int[8] { 0xA9, 0xA5, 0xB5, 0xAD, 0xBD, 0xB9, 0xA1, 0xB1 }
    },
    {
        _ldx,
        5,
        new int[5] { I, zp, zpy, a, ay },
        new int[5] { 0xA2, 0xA6, 0xB6, 0xAE, 0xBE }
    },
    {
        _ldy,
        5,
        new int[5] { I, zp, zpx, a, ax },
        new int[5] { 0xA0, 0xA4, 0xB4, 0xAC, 0xBC }
    },
    {
        _lsr,
        6,
        new int[6] { A, i, zp, zpx, a, ax },
        new int[6] { 0x4A, 0x4A, 0x46, 0x56, 0x4E, 0x5E }
    },
    {
        _nop,
        1,
        new int[1] { i },
        new int[1] { 0xEA }
    },
    {
        _ora,
        8,
        new int[8] { I, zp, zpx, a, ax, ay, izx, izy },
        new int[8] { 0x09, 0x05, 0x15, 0x0D, 0x1D, 0x19, 0x01, 0x11 }
    },
    {
        _pha,
        1,
        new int[1] { i },
        new int[1] { 0x48 }
    },
    {
        _php,
        1,
        new int[1] { i },
        new int[1] { 0x08 }
    },
    {
        _pla,
        1,
        new int[1] { i },
        new int[1] { 0x68 }
    },
    {
        _plp,
        1,
        new int[1] { i },
        new int[1] { 0x28 }
    },
    {
        _rol,
        6,
        new int[6] { A, i, zp, zpx, a, ax },
        new int[6] { 0x2A, 0x2A, 0x26, 0x36, 0x2E, 0x3E }
    },
    {
        _ror,
        6,
        new int[6] { A, i, zp, zpx, a, ax },
        new int[6] { 0x6A, 0x6A, 0x66, 0x76, 0x6E, 0x7E }
    },
    {
        _rti,
        1,
        new int[1] { i },
        new int[1] { 0x40 }
    },
    {
        _rts,
        1,
        new int[1] { i },
        new int[1] { 0x60 }
    },
    {
        _sbc,
        8,
        new int[8] { I, zp, zpx, a, ax, ay, izx, izy },
        new int[8] { 0xE9, 0xE5, 0xF5, 0xED, 0xFD, 0xF9, 0xE1, 0xF1 }
    },
    {
        _sec,
        1,
        new int[1] { i },
        new int[1] { 0x38 }
    },
    {
        _sed,
        1,
        new int[1] { i },
        new int[1] { 0xF8 }
    },
    {
        _sei,
        1,
        new int[1] { i },
        new int[1] { 0x78 }
    },
    {
        _sta,
        7,
        new int[7] { zp, zpx, a, ax, ay, izx, izy },
        new int[7] { 0x85, 0x95, 0x8D, 0x9D, 0x99, 0x81, 0x91 }
    },
    {
        _stx,
        3,
        new int[3] { zp, zpy, a },
        new int[3] { 0x86, 0x96, 0x8E }
    },
    {
        _sty,
        3,
        new int[3] { zp, zpx, a },
        new int[3] { 0x84, 0x94, 0x8C }
    },
    {
        _tax,
        1,
        new int[3] { i },
        new int[3] { 0xAA }
    },
    {
        _tya,
        1,
        new int[3] { i },
        new int[3] { 0x98 }
    },
    {
        _tsx,
        1,
        new int[3] { i },
        new int[3] { 0xBA }
    },
    {
        _txa,
        1,
        new int[3] { i },
        new int[3] { 0x8A }
    },
    {
        _txs,
        1,
        new int[3] { i },
        new int[3] { 0x9A }
    },
    /////////////////////////////////////////////


    {
        _maxOpcode,
        0,
        nullptr,
        nullptr
    }
};

char* escape_string(const char* str)
{
    int len = static_cast<int>(strlen(str)) + 1;
    const int original_len = len;
    for (auto i = 0; i < len; ++i)
    {
        if (str[i] == '\\')
        {
            ++len;
        }
    }

    auto out_index = 0;
    const auto out_str = static_cast<char*>(malloc(len + 1));
    if (out_str == nullptr) return nullptr;
    for (auto original_index = 0; original_index < original_len; ++original_index)
    {
        out_str[out_index++] = str[original_index];
        if (str[original_index] == '\\')
        {
            out_str[out_index++] = '\\';
        }
    }
    return  out_str;
}

void initialize()
{
    destroy();

    init_globals();

    console = stdout;
    console_error = stdout;
    yyout = stdout;

    if (internal_buffer == nullptr)
    {
        internal_buffer = static_cast<char*>(malloc(max_line_len));
    }
    if (file_stack == nullptr)
    {
        file_stack = create_stack(sizeof(file_line_stack_entry));
    }
    if (ifdef_stack == nullptr)
    {
        ifdef_stack = create_stack(sizeof(int));
    }

    final_pass = false;
    ignore_warnings = false;
    output_file_format = bin;
    pass = 0;
    current_file_name = nullptr;

    if (log_file.is_open())
        log_file.close();

    yyin = nullptr;
    yylineno = 0;
    allow_illegal_op_codes = 1;
    program_counter = 0;
    max_macro_param = 0;
}

void destroy()
{
    if (internal_buffer != nullptr) {
        free(internal_buffer);
        internal_buffer = nullptr;
    }
    if (file_stack != nullptr) {
        free_stack(file_stack);
        file_stack = nullptr;
    }
    if (ifdef_stack != nullptr) {
        free_stack(ifdef_stack);
        ifdef_stack = nullptr;
    }

    macro_dict.clear();
    symbol_dictionary.clear();

    if (macro_params_stack != nullptr) {
        free_stack(macro_params_stack);
        macro_params_stack = nullptr;
    }
}

size_t char_to_w_string(std::string s, std::wstring& ws)
{
    const std::wstring ws_tmp(s.begin(), s.end());
    ws = ws_tmp;

    return ws.length();
}

void execute_text(const char* text, const unsigned char* expected, const size_t count, const char* expected_text)
{
    const char* in_file_name =  (char*)"execute_text.a";
    const char* out_file_name = (char*)"execute_text.bin";
    const char* console_name =  (char*)"console.txt";
    
    FILE* temp_file = open_file((char*)in_file_name, "w");
    EXPECT_NOT_NULL(temp_file);

    fwrite(text, 1,  strlen(text), temp_file);
    fclose(temp_file);

    console = open_file("console.txt", "w");
    EXPECT_NOT_NULL(console);
    console_error = console;
    yyout = console;
    
    input_file_count = 0;
    char* argv[] =
    {
        (char*)"pasm.exe",
        (char*)"-v",
        (char*)in_file_name,
        (char*)"-o",
        (char*)out_file_name
    };
    int argc = _countof(argv);
    EXPECT_EQ(5, argc);
    
    int result = parse_arguments(argc, argv);
    EXPECT_EQ(1, result);

    result = assemble();

    fflush(console);
    fclose(console);
    console = stdout;

    if (result != 0)
        EXPECT_EQ(0, result);

    if (result == 0)
    {
        auto* buffer = static_cast<unsigned char*>(malloc(count));
        EXPECT_NOT_NULL(buffer);
        output_file = open_file(out_file_name, "rb");
        EXPECT_NOT_NULL(output_file);

        fseek(output_file, 0, SEEK_END);
        size_t pos = ftell(output_file);
        if (count != pos)
            EXPECT_EQ(count, pos);

        fseek(output_file, 0, SEEK_SET);
        fread(buffer, 1, count, output_file);
        fclose(output_file);

        for (size_t i = 0; i < count; ++ i)
        {
            const auto e = expected[i];
            const auto a = buffer[i];
            if (e != a)
            {
                EXPECT_EQ(e, a);
            }
        }

        free(buffer);

        if (expected_text != nullptr)
        {
            console = open_file("console.txt", "r");
            const auto len = strlen(expected_text);

            fseek(console, 0, SEEK_END);
            pos = ftell(console);
            fseek(console, 0, SEEK_SET);
            buffer = static_cast<unsigned char*>(malloc(pos + 1));
            EXPECT_NOT_NULL(buffer);
            if (buffer) {
                memset(buffer, 0, pos + 1);
                fread(buffer, 1, pos, console);
                fclose(console);

                EXPECT_TRUE(pos >= len);
                result = strncmp(expected_text, reinterpret_cast<char*>(buffer), len);
                if (result != 0) {
                    EXPECT_STREQ(expected_text, reinterpret_cast<char*>(buffer));

                    //for (auto i = 0; i < len; ++i)
                    //{
                    //    auto e = expected_text[i];
                    //    auto a = static_cast<char>(buffer[i]);
                    //    if (e != a)
                    //    {
                    //        EXPECT_EQ(e, a);
                    //    }
                    //}
                }
                EXPECT_EQ(0, result);

                free(buffer);
            }
        }
    }

    if (expected_text != nullptr)
    {
        console = stdout;
    }
    remove(in_file_name);
    remove(out_file_name);
    input_file_count = 0;
    reset_lex();
}
