#include <gtest/gtest.h>
#include "unit_test.h"

namespace assemble_test
{
    static void assemble_unit_test_method_initialize();
    static void assemble_unit_test_method_cleanup();

    static void assemble_unit_test_method_initialize()
    {
        initialize();
        free_parse_tree();
        program_counter = 0;
        generate_output_reset();
        symbol_dictionary.clear();
        symbol_dictionary.clear();
        reset_lex();
    }

    static void assemble_unit_test_method_cleanup()
    {
        destroy();
    }

    TEST(assemble_unit_test, program_statement_unit_test)
    {
        assemble_unit_test_method_initialize();
        std::string code =
            "   .org $1000\n"
            "   nop\n";

        constexpr unsigned char expected[] =
        {
            0xea
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $EA               nop                    nop\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, program_symbol_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   start\n"
            "       nop\n";

        constexpr unsigned char expected[] =
        {
            0xea
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                start\n"
            "$1000: $EA               nop                        nop\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_opcode_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "       clc\n";

        constexpr unsigned char expected[] =
        {
            0x18
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $18               clc                        clc\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, branch_list_unit_text)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $C63A\n"
            "    BCDSTR = $003B\n"
            "    BCDLEN = $61\n"
            ";********************************************\n"
            ";*                                          *\n"
            ";*  BCDLENGTH                               *\n"
            ";*                                          *\n"
            ";*  get length of BCDSTR                    *\n"
            ";*  store result in BCDLEN                  *\n"
            ";*                                          *\n"
            ";*  destroys    a,y                         *\n"
            ";*                                          *\n"
            ";********************************************\n"
            "BCDLENGTH\n"
            "        ;\n"
            "        ;   loop through 5 chars\n"
            "        ;\n"
            "        ldy #0\n"
            "        sty BCDLEN\n"
            "-\n"
            "        lda BCDSTR,y\n"
            "        cmp #'0'\n"
            "        bne +\n"
            "\n"
            "        cpy #4\n"
            "        beq +\n"
            "\n"
            "        ldx BCDLEN\n"
            "        beq ++\n"
            "+\n"
            "        sty BCDLEN\n"
            "        lda #5\n"
            "        sec\n"
            "        sbc BCDLEN\n"
            "        sta BCDLEN\n"
            "        rts\n"
            "+\n"
            "        iny\n"
            "        bne -\n";

        constexpr unsigned char expected[] =
        {
            0xA0, 0x00,
            0x84, 0x61,
            0xB9, 0x3B, 0x00,
            0xC9, 0x30,
            0xD0, 0x08,
            0xC0, 0x04,
            0xF0, 0x04,
            0xA6, 0x61,
            0xF0, 0x0A,
            0x84, 0x61,
            0xA9, 0x05,
            0x38,
            0xE5, 0x61,
            0x85, 0x61,
            0x60,
            0xC8,
            0xD0, 0xE4
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "32 bytes written to $$BIN_FILE$$\n"
            "\n"
            "         BCDSTR $003B           BCDLEN $0061  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $C63A\n"
            "                                                 BCDSTR = $003B\n"
            "                                                 BCDLEN = $61\n"
            "                                             ;********************************************\n"
            "                                             ;*                                          *\n"
            "                                             ;*  BCDLENGTH                               *\n"
            "                                             ;*                                          *\n"
            "                                             ;*  get length of BCDSTR                    *\n"
            "                                             ;*  store result in BCDLEN                  *\n"
            "                                             ;*                                          *\n"
            "                                             ;*  destroys    a,y                         *\n"
            "                                             ;*                                          *\n"
            "                                             ;********************************************\n"
            "                                             BCDLENGTH\n"
            "                                                     ;\n"
            "                                                     ;   loop through 5 chars\n"
            "                                                     ;\n"
            "$C63A: $A0 $00           ldy #$00                    ldy #0\n"
            "$C63C: $84 $61           sty $61                     sty BCDLEN\n"
            "                                             -\n"
            "$C63E: $B9 $3B $00       lda $003B,y                 lda BCDSTR,y\n"
            "$C641: $C9 $30           cmp #$30                    cmp #'0'\n"
            "$C643: $D0 $08           bne $C64D                   bne +\n"
            "                                             \n"
            "$C645: $C0 $04           cpy #$04                    cpy #4\n"
            "$C647: $F0 $04           beq $C64D                   beq +\n"
            "                                             \n"
            "$C649: $A6 $61           ldx $61                     ldx BCDLEN\n"
            "$C64B: $F0 $0A           beq $C657                   beq ++\n"
            "                                             +\n"
            "$C64D: $84 $61           sty $61                     sty BCDLEN\n"
            "$C64F: $A9 $05           lda #$05                    lda #5\n"
            "$C651: $38               sec                         sec\n"
            "$C652: $E5 $61           sbc $61                     sbc BCDLEN\n"
            "$C654: $85 $61           sta $61                     sta BCDLEN\n"
            "$C656: $60               rts                         rts\n"
            "                                             +\n"
            "$C657: $C8               iny                         iny\n"
            "$C658: $D0 $E4           bne $C63E                   bne -\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_directive_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   brk\n";

        constexpr unsigned char expected[] =
        {
            0x00
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $00               brk                    brk\n"
            ;
        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_symbol_value_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "symbol\n"
            "   .word symbol\n";

        constexpr unsigned char expected[] =
        {
            WORD(0x1000)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "         symbol $1000  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                             symbol\n"
            "$1000: $00 $10           .db $00, $10           .word symbol\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_symbol_assign_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   symbol = $1234\n"
            "   .word symbol\n";

        constexpr unsigned char expected[] =
        {
            WORD(0x1234)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "         symbol $1234  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                symbol = $1234\n"
            "$1000: $34 $12           .db $34, $12           .word symbol\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_pc_assign_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   * = $300\n"
            "   here\n"
            "   * = $500\n"
            "   there\n"
            "   .org $1000\n"
            "   .word *\n"
            "   .word here\n"
            "   .word there\n";

        constexpr unsigned char expected[] =
        {
            WORD(0x1000),
            WORD(0x0300),
            WORD(0x0500)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "6 bytes written to $$BIN_FILE$$\n"
            "\n"
            "           here $0300            there $0500  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                * = $300\n"
            "                                                here\n"
            "                                                * = $500\n"
            "                                                there\n"
            "                                                .org $1000\n"
            "$1000: $00 $10           .db $00, $10           .word *\n"
            "$1002: $00 $03           .db $00, $03           .word here\n"
            "$1004: $00 $05           .db $00, $05           .word there\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_ifexpr_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var sym = 23\n"
            "   .if sym > 20\n"
            "       .text \"sym > 20\"\n"
            "   .else\n"
            "       .text \"sym <= 20\"\n"
            "   .endif\n"
            "   .if * < $1000\n"
            "       .text \"* < $1000\"\n"
            "   .else\n"
            "       .text \"* >= $1000\"\n"
            "   .endif\n"
            "\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x73, 0x79, 0x6D,
            0x20, 0x3E, 0x20,
            0x32, 0x30,
            0x2A, 0x20, 0x3E,
            0x3D, 0x20, 0x24,
            0x31, 0x30, 0x30,
            0x30
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "18 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var sym = 23\n"
            "                                                .if sym > 20\n"
            "                                                    .text \"sym > 20\"\n"
            "                                                .else\n"
            "                                                    .text \"sym <= 20\"\n"
            "$1000: $73 $79 $6D       .db 's', 'y', 'm'      .endif\n"
            "$1003: $20 $3E $20       .db ' ', '>', ' '   \n"
            "$1006: $32 $30           .db '2', '0'        \n"
            "                                                .if * < $1000\n"
            "                                                    .text \"* < $1000\"\n"
            "                                                .else\n"
            "                                                    .text \"* >= $1000\"\n"
            "$1008: $2A $20 $3E       .db '*', ' ', '>'      .endif\n"
            "$100B: $3D $20 $24       .db '=', ' ', '$'   \n"
            "$100E: $31 $30 $30       .db '1', '0', '0'   \n"
            "$1011: $30               .db '0'             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_loopexpr_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var i = 23\n"
            "   .repeat\n"
            "       .word i\n"
            "       i = i -1\n"
            "   .until i < 20\n";

        constexpr unsigned char expected[] =
        {
            WORD(23),
            WORD(22),
            WORD(21),
            WORD(20)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "8 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var i = 23\n"
            "                                                .repeat\n"
            "                                                    .word i\n"
            "                                                    i = i -1\n"
            "$1000: $17 $00           .db $17, $00           .until i < 20\n"
            "$1002: $16 $00           .db $16, $00        \n"
            "$1004: $15 $00           .db $15, $00        \n"
            "$1006: $14 $00           .db $14, $00        \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_regloopexpr_unit_test)
    {
        assemble_unit_test_method_initialize();
        std::string code =
            "    .org $1000\n"
            "\n"
            "    lda #$FF\n"
            "    .for .regx = 0 .to 10\n"
            "        sta data, x\n"
            "    .next X\n"
            "    stx cow\n"
            "\n"
            "    data .ds 11\n"
            "    cow  .ds 12\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa9, 0xff,
            0xa2, 00,
            0x9d, 0x0f, 0x10,
            0xe8,
            0xe0, 0x0b,
            0x90, 0xf8,
            0x8e, 0x1a, 0x10,
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "15 bytes written to $$BIN_FILE$$\n"
            "\n"
            "           data $100F              cow $101A  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "                                             \n"
            "$1000: $A9 $FF           lda #$FF                lda #$FF\n"
            "                                                 .for .regx = 0 .to 10\n"
            "                                                     sta data, x\n"
            "$1002: $A2 $00           ldx #$00                .next X\n"
            "$1004: $9D $0F $10       sta $100F,x         \n"
            "$1007: $E8               inx                 \n"
            "$1008: $E0 $0B           cpx #$0B            \n"
            "$100A: $90 $F8           bcc $1004           \n"
            "$100C: $8E $1A $10       stx $101A               stx cow\n"
            "                                             \n"
            "                                                 data .ds 11\n"
            "                                                 cow  .ds 12\n";

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_macrodef_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "        .macro ADD16\n"
            "        clc\n"
            "        lda \\1\n"
            "        adc \\2\n"
            "        sta \\3\n"
            "        lda \\1 + 1\n"
            "        adc \\2 + 1\n"
            "        sta \\3 + 1\n"
            "        .endm\n"
            "\n"
            "        .org $1000\n"
            "        ADD16 AAA, BBB, RESULT\n"
            "\n"
            "AAA     .word $1234\n"
            "BBB     .word $4567\n"
            "RESULT  .word 0\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x18,
            0xad, 0x13, 0x10,
            0x6d, 0x15, 0x10,
            0x8d, 0x17, 0x10,
            0xad, 0x14, 0x10,
            0x6d, 0x16, 0x10,
            0x8d, 0x18, 0x10,
            0x34, 0x12,
            0x67, 0x45,
            0x00, 0x00
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 3\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "25 bytes written to $$BIN_FILE$$\n"
            "\n"
            "            AAA $1013              BBB $1015           RESULT $1017  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                     .macro ADD16\n"
            "                                                     clc\n"
            "                                                     lda \\1\n"
            "                                                     adc \\2\n"
            "                                                     sta \\3\n"
            "                                                     lda \\1 + 1\n"
            "                                                     adc \\2 + 1\n"
            "                                                     sta \\3 + 1\n"
            "                                                     .endm\n"
            "                                             \n"
            "                                                     .org $1000\n"
            "$1000: $18               clc                         ADD16 AAA, BBB, RESULT\n"
            "$1001: $AD $13 $10       lda $1013           \n"
            "$1004: $6D $15 $10       adc $1015           \n"
            "$1007: $8D $17 $10       sta $1017           \n"
            "$100A: $AD $14 $10       lda $1014           \n"
            "$100D: $6D $16 $10       adc $1016           \n"
            "$1010: $8D $18 $10       sta $1018           \n"
            "                                             \n"
            "$1013: $34 $12           .db $34, $12        AAA     .word $1234\n"
            "$1015: $67 $45           .db $67, $45        BBB     .word $4567\n"
            "$1017: $00 $00           .db $00, $00        RESULT  .word 0\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_macrocall_unit_test)
    {
        assemble_unit_test_method_initialize();
        std::string code =
            "        .macro ADD16\n"
            "        clc\n"
            "        lda \\1\n"
            "        adc \\2\n"
            "        sta \\3\n"
            "        lda \\1 + 1\n"
            "        adc \\2 + 1\n"
            "        sta \\3 + 1\n"
            "        .endm\n"
            "\n"
            "        .org $1000\n"
            "        ADD16 AAA, BBB, RESULT\n"
            "\n"
            "AAA     .word $1234\n"
            "BBB     .word $4567\n"
            "RESULT  .word 0\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x18,
            0xad, 0x13, 0x10,
            0x6d, 0x15, 0x10,
            0x8d, 0x17, 0x10,
            0xad, 0x14, 0x10,
            0x6d, 0x16, 0x10,
            0x8d, 0x18, 0x10,
            0x34, 0x12,
            0x67, 0x45,
            0x00, 0x00
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 3\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "25 bytes written to $$BIN_FILE$$\n"
            "\n"
            "            AAA $1013              BBB $1015           RESULT $1017  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                     .macro ADD16\n"
            "                                                     clc\n"
            "                                                     lda \\1\n"
            "                                                     adc \\2\n"
            "                                                     sta \\3\n"
            "                                                     lda \\1 + 1\n"
            "                                                     adc \\2 + 1\n"
            "                                                     sta \\3 + 1\n"
            "                                                     .endm\n"
            "                                             \n"
            "                                                     .org $1000\n"
            "$1000: $18               clc                         ADD16 AAA, BBB, RESULT\n"
            "$1001: $AD $13 $10       lda $1013           \n"
            "$1004: $6D $15 $10       adc $1015           \n"
            "$1007: $8D $17 $10       sta $1017           \n"
            "$100A: $AD $14 $10       lda $1014           \n"
            "$100D: $6D $16 $10       adc $1016           \n"
            "$1010: $8D $18 $10       sta $1018           \n"
            "                                             \n"
            "$1013: $34 $12           .db $34, $12        AAA     .word $1234\n"
            "$1015: $67 $45           .db $67, $45        BBB     .word $4567\n"
            "$1017: $00 $00           .db $00, $00        RESULT  .word 0\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_vardef_unit_test)
    {
        assemble_unit_test_method_initialize();

        // Fibonacci sequence using repeat
        std::string code =
            "   .org $c000\n"
            "   .var aa = 1, bb = 2, t\n"
            "   .while aa < 256\n"
            "       .byte aa\n"
            "       t = aa\n"
            "       aa = bb\n"
            "       bb = t + bb\n"
            "   .wend\n";

        const unsigned char expected[] =
        {
            1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $c000\n"
            "                                                .var aa = 1, bb = 2, t\n"
            "                                                .while aa < 256\n"
            "                                                    .byte aa\n"
            "                                                    t = aa\n"
            "                                                    aa = bb\n"
            "                                                    bb = t + bb\n"
            "$C000: $01               .db $01                .wend\n"
            "$C001: $02               .db $02             \n"
            "$C002: $03               .db $03             \n"
            "$C003: $05               .db $05             \n"
            "$C004: $08               .db $08             \n"
            "$C005: $0D               .db $0D             \n"
            "$C006: $15               .db $15             \n"
            "$C007: $22               .db $22             \n"
            "$C008: $37               .db $37             \n"
            "$C009: $59               .db $59             \n"
            "$C00A: $90               .db $90             \n"
            "$C00B: $E9               .db $E9             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_EOL_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $c000\n"
            "\n"
            "\n"
            "   .byte 0\n";

        constexpr unsigned char expected[] =
        {
           0
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $c000\n"
            "                                             \n"
            "                                             \n"
            "$C000: $00               .db $00                .byte 0\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);

        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // stmt_list
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, stmt_list_stmt_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $c000\n"
            "\n"
            "   .byte 0\n";

        constexpr unsigned char expected[] =
        {
           0
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $c000\n"
            "                                             \n"
            "$C000: $00               .db $00                .byte 0\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, stmt_list_stmt_list_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $c000\n"
            "\n"
            "\n .do"
            "       .byte 0\n"
            "       .byte 1\n"
            "\n .while 0\n";

        constexpr unsigned char expected[] =
        {
           0, 1
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $c000\n"
            "                                             \n"
            "                                             \n"
            "                                              .do       .byte 0\n"
            "                                                    .byte 1\n"
            "                                             \n"
            "$C000: $00               .db $00              .while 0\n"
            "$C001: $01               .db $01             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // ifexpr
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, ifexpr_if_else_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var aa = 100, bb = 200\n"
            "   .if aa > 50\n"
            "       .text \"aa > 50\"\n"
            "   .else\n"
            "       .text \"FAIL\"\n"
            "   .endif\n"
            "   .if bb > 200\n"
            "       .text \"FAIL\"\n"
            "   .else\n"
            "       .text \"bb <= 200\"\n"
            "   .endif\n"
            ;

        constexpr unsigned char expected[] =
        {
            "aa > 50"
            "bb <= 200"
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "16 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var aa = 100, bb = 200\n"
            "                                                .if aa > 50\n"
            "                                                    .text \"aa > 50\"\n"
            "                                                .else\n"
            "                                                    .text \"FAIL\"\n"
            "$1000: $61 $61 $20       .db 'a', 'a', ' '      .endif\n"
            "$1003: $3E $20 $35       .db '>', ' ', '5'   \n"
            "$1006: $30               .db '0'             \n"
            "                                                .if bb > 200\n"
            "                                                    .text \"FAIL\"\n"
            "                                                .else\n"
            "                                                    .text \"bb <= 200\"\n"
            "$1007: $62 $62 $20       .db 'b', 'b', ' '      .endif\n"
            "$100A: $3C $3D $20       .db '<', '=', ' '   \n"
            "$100D: $32 $30 $30       .db '2', '0', '0'   \n"
            ;

        execute_text(code, expected, _countof(expected) - 1, expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, ifexpr_if_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var aa = 889\n"
            "   .if aa > 88\n"
            "       .text \"aa > 88\"\n"
            "   .endif\n"
            ;

        constexpr unsigned char expected[] =
        {
            "aa > 88"
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "7 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var aa = 889\n"
            "                                                .if aa > 88\n"
            "                                                    .text \"aa > 88\"\n"
            "$1000: $61 $61 $20       .db 'a', 'a', ' '      .endif\n"
            "$1003: $3E $20 $38       .db '>', ' ', '8'   \n"
            "$1006: $38               .db '8'             \n"
            ;

        execute_text(code, expected, _countof(expected) - 1, expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // loopexpr
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, loopexpr_repeat_unil_unit_test)
    {
        assemble_unit_test_method_initialize();

        // Fibonacci sequence using repeat
        std::string code =
            "   .org $c000\n"
            "   .var aa = 1, bb = 2, t\n"
            "   .repeat\n"
            "       .byte aa\n"
            "       t = aa\n"
            "       aa = bb\n"
            "       bb = t + bb\n"
            "   .until aa >= 256\n";

        const unsigned char expected[] =
        {
            1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $c000\n"
            "                                                .var aa = 1, bb = 2, t\n"
            "                                                .repeat\n"
            "                                                    .byte aa\n"
            "                                                    t = aa\n"
            "                                                    aa = bb\n"
            "                                                    bb = t + bb\n"
            "$C000: $01               .db $01                .until aa >= 256\n"
            "$C001: $02               .db $02             \n"
            "$C002: $03               .db $03             \n"
            "$C003: $05               .db $05             \n"
            "$C004: $08               .db $08             \n"
            "$C005: $0D               .db $0D             \n"
            "$C006: $15               .db $15             \n"
            "$C007: $22               .db $22             \n"
            "$C008: $37               .db $37             \n"
            "$C009: $59               .db $59             \n"
            "$C00A: $90               .db $90             \n"
            "$C00B: $E9               .db $E9             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, loopexpr_do_while_unit_test)
    {
        assemble_unit_test_method_initialize();

        // Fibonacci sequence using do while
        std::string code =
            "   .org $c000\n"
            "   .var aa = 1, bb = 2, t\n"
            "   .do\n"
            "       .byte aa\n"
            "       t = aa\n"
            "       aa = bb\n"
            "       bb = t + bb\n"
            "   .while aa < 256\n";

        const unsigned char expected[] =
        {
            1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $c000\n"
            "                                                .var aa = 1, bb = 2, t\n"
            "                                                .do\n"
            "                                                    .byte aa\n"
            "                                                    t = aa\n"
            "                                                    aa = bb\n"
            "                                                    bb = t + bb\n"
            "$C000: $01               .db $01                .while aa < 256\n"
            "$C001: $02               .db $02             \n"
            "$C002: $03               .db $03             \n"
            "$C003: $05               .db $05             \n"
            "$C004: $08               .db $08             \n"
            "$C005: $0D               .db $0D             \n"
            "$C006: $15               .db $15             \n"
            "$C007: $22               .db $22             \n"
            "$C008: $37               .db $37             \n"
            "$C009: $59               .db $59             \n"
            "$C00A: $90               .db $90             \n"
            "$C00B: $E9               .db $E9             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, loopexpr_while_wend_unit_test)
    {
        assemble_unit_test_method_initialize();

        // Fibonacci sequence using while wend
        std::string code =
            "   .org $c000\n"
            "   .var aa = 1, bb = 2, t\n"
            "   .while aa < 256\n"
            "       .byte aa\n"
            "       t = aa\n"
            "       aa = bb\n"
            "       bb = t + bb\n"
            "   .wend\n";

        const unsigned char expected[] =
        {
            1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $c000\n"
            "                                                .var aa = 1, bb = 2, t\n"
            "                                                .while aa < 256\n"
            "                                                    .byte aa\n"
            "                                                    t = aa\n"
            "                                                    aa = bb\n"
            "                                                    bb = t + bb\n"
            "$C000: $01               .db $01                .wend\n"
            "$C001: $02               .db $02             \n"
            "$C002: $03               .db $03             \n"
            "$C003: $05               .db $05             \n"
            "$C004: $08               .db $08             \n"
            "$C005: $0D               .db $0D             \n"
            "$C006: $15               .db $15             \n"
            "$C007: $22               .db $22             \n"
            "$C008: $37               .db $37             \n"
            "$C009: $59               .db $59             \n"
            "$C00A: $90               .db $90             \n"
            "$C00B: $E9               .db $E9             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, loopexpr_for_next_step_unit_test)
    {
        assemble_unit_test_method_initialize();

        // Fibonacci sequence using for
        std::string code =
            "   .org $c000\n"
            "   .var aa = 1, bb = 2, t, i\n"
            "   .for i = 12 .to 1 .step -1\n"
            "       .byte aa\n"
            "       t = aa\n"
            "       aa = bb\n"
            "       bb = t + bb\n"
            "   .next i\n";

        const unsigned char expected[] =
        {
            1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $c000\n"
            "                                                .var aa = 1, bb = 2, t, i\n"
            "                                                .for i = 12 .to 1 .step -1\n"
            "                                                    .byte aa\n"
            "                                                    t = aa\n"
            "                                                    aa = bb\n"
            "                                                    bb = t + bb\n"
            "$C000: $01               .db $01                .next i\n"
            "$C001: $02               .db $02             \n"
            "$C002: $03               .db $03             \n"
            "$C003: $05               .db $05             \n"
            "$C004: $08               .db $08             \n"
            "$C005: $0D               .db $0D             \n"
            "$C006: $15               .db $15             \n"
            "$C007: $22               .db $22             \n"
            "$C008: $37               .db $37             \n"
            "$C009: $59               .db $59             \n"
            "$C00A: $90               .db $90             \n"
            "$C00B: $E9               .db $E9             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, loopexpr_for_next_unit_test)
    {
        assemble_unit_test_method_initialize();

        // Fibonacci sequence using for
        std::string code =
            "   .org $c000\n"
            "   .var aa = 1, bb = 2, t, i\n"
            "   .for i = 1 .to 12\n"
            "       .byte aa\n"
            "       t = aa\n"
            "       aa = bb\n"
            "       bb = t + bb\n"
            "   .next i\n";

        const unsigned char expected[] =
        {
            1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $c000\n"
            "                                                .var aa = 1, bb = 2, t, i\n"
            "                                                .for i = 1 .to 12\n"
            "                                                    .byte aa\n"
            "                                                    t = aa\n"
            "                                                    aa = bb\n"
            "                                                    bb = t + bb\n"
            "$C000: $01               .db $01                .next i\n"
            "$C001: $02               .db $02             \n"
            "$C002: $03               .db $03             \n"
            "$C003: $05               .db $05             \n"
            "$C004: $08               .db $08             \n"
            "$C005: $0D               .db $0D             \n"
            "$C006: $15               .db $15             \n"
            "$C007: $22               .db $22             \n"
            "$C008: $37               .db $37             \n"
            "$C009: $59               .db $59             \n"
            "$C00A: $90               .db $90             \n"
            "$C00B: $E9               .db $E9             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // regloopexpr
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, regloopexpr_for_regx_to_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "\n"
            "    lda #$FF\n"
            "    .for .regx = 0 .to 10\n"
            "        sta data, x\n"
            "    .next X\n"
            "\n"
            "    data .ds 11\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa9, 0xff,
            0xa2, 0x00,
            0x9d, 0x0c, 0x10,
            0xe8,
            0xe0, 0x0b,
            0x90, 0xf8,
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "           data $100C  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "                                             \n"
            "$1000: $A9 $FF           lda #$FF                lda #$FF\n"
            "                                                 .for .regx = 0 .to 10\n"
            "                                                     sta data, x\n"
            "$1002: $A2 $00           ldx #$00                .next X\n"
            "$1004: $9D $0C $10       sta $100C,x         \n"
            "$1007: $E8               inx                 \n"
            "$1008: $E0 $0B           cpx #$0B            \n"
            "$100A: $90 $F8           bcc $1004           \n"
            "                                             \n"
            "                                                 data .ds 11\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, regloopexpr_for_regx_downto_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "\n"
            "    lda #$FF\n"
            "    .for .regx = 10 .downto 0\n"
            "        sta data, x\n"
            "    .next X\n"
            "\n"
            "    data\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa9, 0xff,
            0xa2, 0x0a,
            0x9d, 0x0a, 0x10,
            0xca,
            0xb0, 0xfa
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "10 bytes written to $$BIN_FILE$$\n"
            "\n"
            "           data $100A  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "                                             \n"
            "$1000: $A9 $FF           lda #$FF                lda #$FF\n"
            "                                                 .for .regx = 10 .downto 0\n"
            "                                                     sta data, x\n"
            "$1002: $A2 $0A           ldx #$0A                .next X\n"
            "$1004: $9D $0A $10       sta $100A,x         \n"
            "$1007: $CA               dex                 \n"
            "$1008: $B0 $FA           bcs $1004           \n"
            "                                             \n"
            "                                                 data\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, regloopexpr_for_regy_to_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "\n"
            "    lda #$FF\n"
            "    .for .regy = 0 .to 10\n"
            "        sta data, y\n"
            "    .next Y\n"
            "\n"
            "    data\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa9, 0xff,
            0xa0, 0x00,
            0x99, 0x0c, 0x10,
            0xc8,
            0xc0, 0x0b,
            0x90, 0xf8
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "           data $100C  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "                                             \n"
            "$1000: $A9 $FF           lda #$FF                lda #$FF\n"
            "                                                 .for .regy = 0 .to 10\n"
            "                                                     sta data, y\n"
            "$1002: $A0 $00           ldy #$00                .next Y\n"
            "$1004: $99 $0C $10       sta $100C,y         \n"
            "$1007: $C8               iny                 \n"
            "$1008: $C0 $0B           cpy #$0B            \n"
            "$100A: $90 $F8           bcc $1004           \n"
            "                                             \n"
            "                                                 data\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, regloopexpr_for_regy_downto_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "\n"
            "    lda #$FF\n"
            "    .for .regy = 10 .downto 0\n"
            "        sta data, y\n"
            "    .next Y\n"
            "\n"
            "    data .ds 11\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa9, 0xff,
            0xa0, 0x0a,
            0x99, 0x0a, 0x10,
            0x88,
            0xb0, 0xfa
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "10 bytes written to $$BIN_FILE$$\n"
            "\n"
            "           data $100A  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "                                             \n"
            "$1000: $A9 $FF           lda #$FF                lda #$FF\n"
            "                                                 .for .regy = 10 .downto 0\n"
            "                                                     sta data, y\n"
            "$1002: $A0 $0A           ldy #$0A                .next Y\n"
            "$1004: $99 $0A $10       sta $100A,y         \n"
            "$1007: $88               dey                 \n"
            "$1008: $B0 $FA           bcs $1004           \n"
            "                                             \n"
            "                                                 data .ds 11\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // expr_list
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, expr_list_subexpr_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .byte $56\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x56
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $56               .db $56                .byte $56\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_list_string_literal_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .byte \"abcdef\"\n"
            ;

        constexpr unsigned char expected[] =
        {
            "abcdef"
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "6 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $61 $62 $63       .db 'a', 'b', 'c'      .byte \"abcdef\"\n"
            "$1003: $64 $65 $66       .db 'd', 'e', 'f'   \n"
            ;

        execute_text(code, expected, _countof(expected) - 1, expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_list_expr_list_subexpr_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .byte $56, $55, $54\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x56, 0x55, 0x54
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "3 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $56               .db $56                .byte $56, $55, $54\n"
            "$1001: $55               .db $55             \n"
            "$1002: $54               .db $54             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_list_expr_list_string_literal_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .byte \"cat\",\"dog\",\"cow\"\n"
            ;

        constexpr unsigned char expected[] =
        {
            "cat"
            "dog"
            "cow"
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "9 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $63 $61 $74       .db 'c', 'a', 't'      .byte \"cat\",\"dog\",\"cow\"\n"
            "$1003: $64 $6F $67       .db 'd', 'o', 'g'   \n"
            "$1006: $63 $6F $77       .db 'c', 'o', 'w'   \n"
            ;

        execute_text(code, expected, _countof(expected) - 1, expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // macrodef
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, macro_def_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "        .macro ADD16\n"
            "        clc\n"
            "        lda \\1\n"
            "        adc \\2\n"
            "        sta \\3\n"
            "        lda \\1 + 1\n"
            "        adc \\2 + 1\n"
            "        sta \\3 + 1\n"
            "        .endm\n"
            "\n"
            "        .org $1000\n"
            "        ADD16 AAA, BBB, RESULT\n"
            "\n"
            "AAA     .word $1234\n"
            "BBB     .word $4567\n"
            "RESULT  .ds 2\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x18,
            0xad, 0x13, 0x10,
            0x6d, 0x15, 0x10,
            0x8d, 0x17, 0x10,
            0xad, 0x14, 0x10,
            0x6d, 0x16, 0x10,
            0x8d, 0x18, 0x10,
            0x34, 0x12,
            0x67, 0x45
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 3\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "23 bytes written to $$BIN_FILE$$\n"
            "\n"
            "            AAA $1013              BBB $1015           RESULT $1017  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                     .macro ADD16\n"
            "                                                     clc\n"
            "                                                     lda \\1\n"
            "                                                     adc \\2\n"
            "                                                     sta \\3\n"
            "                                                     lda \\1 + 1\n"
            "                                                     adc \\2 + 1\n"
            "                                                     sta \\3 + 1\n"
            "                                                     .endm\n"
            "                                             \n"
            "                                                     .org $1000\n"
            "$1000: $18               clc                         ADD16 AAA, BBB, RESULT\n"
            "$1001: $AD $13 $10       lda $1013           \n"
            "$1004: $6D $15 $10       adc $1015           \n"
            "$1007: $8D $17 $10       sta $1017           \n"
            "$100A: $AD $14 $10       lda $1014           \n"
            "$100D: $6D $16 $10       adc $1016           \n"
            "$1010: $8D $18 $10       sta $1018           \n"
            "                                             \n"
            "$1013: $34 $12           .db $34, $12        AAA     .word $1234\n"
            "$1015: $67 $45           .db $67, $45        BBB     .word $4567\n"
            "                                             RESULT  .ds 2\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // macrocall
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, macrocall_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "        .macro ADD16\n"
            "        clc\n"
            "        lda \\1\n"
            "        adc \\2\n"
            "        sta \\3\n"
            "        lda \\1 + 1\n"
            "        adc \\2 + 1\n"
            "        sta \\3 + 1\n"
            "        .endm\n"
            "\n"
            "        .org $1000\n"
            "        ADD16 AAA, BBB, RESULT\n"
            "\n"
            "AAA     .word $1234\n"
            "BBB     .word $4567\n"
            "RESULT  .ds 2\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x18,
            0xad, 0x13, 0x10,
            0x6d, 0x15, 0x10,
            0x8d, 0x17, 0x10,
            0xad, 0x14, 0x10,
            0x6d, 0x16, 0x10,
            0x8d, 0x18, 0x10,
            0x34, 0x12,
            0x67, 0x45
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 3\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "23 bytes written to $$BIN_FILE$$\n"
            "\n"
            "            AAA $1013              BBB $1015           RESULT $1017  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                     .macro ADD16\n"
            "                                                     clc\n"
            "                                                     lda \\1\n"
            "                                                     adc \\2\n"
            "                                                     sta \\3\n"
            "                                                     lda \\1 + 1\n"
            "                                                     adc \\2 + 1\n"
            "                                                     sta \\3 + 1\n"
            "                                                     .endm\n"
            "                                             \n"
            "                                                     .org $1000\n"
            "$1000: $18               clc                         ADD16 AAA, BBB, RESULT\n"
            "$1001: $AD $13 $10       lda $1013           \n"
            "$1004: $6D $15 $10       adc $1015           \n"
            "$1007: $8D $17 $10       sta $1017           \n"
            "$100A: $AD $14 $10       lda $1014           \n"
            "$100D: $6D $16 $10       adc $1016           \n"
            "$1010: $8D $18 $10       sta $1018           \n"
            "                                             \n"
            "$1013: $34 $12           .db $34, $12        AAA     .word $1234\n"
            "$1015: $67 $45           .db $67, $45        BBB     .word $4567\n"
            "                                             RESULT  .ds 2\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // symbol_list
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, symbol_list_symbol_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var sym\n"
            "   sym = $5043\n"
            "   .word sym\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x5043)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var sym\n"
            "                                                sym = $5043\n"
            "$1000: $43 $50           .db $43, $50           .word sym\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, symbol_list_symbol_list_symbol_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var sym1, sym2\n"
            "   sym1 = $5043\n"
            "   sym2 = $5289\n"
            "   .word sym1, sym2\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x5043),
            WORD(0x5289)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var sym1, sym2\n"
            "                                                sym1 = $5043\n"
            "                                                sym2 = $5289\n"
            "$1000: $43 $50           .db $43, $50           .word sym1, sym2\n"
            "$1002: $89 $52           .db $89, $52        \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, symbol_list_symbol_equals_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var sym = $3244\n"
            "   .word sym\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x3244)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var sym = $3244\n"
            "$1000: $44 $32           .db $44, $32           .word sym\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, symbol_list_symbol_list_symbol_equals_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var sym1 = $3244, sym2 = $7896\n"
            "   .word sym1,sym2\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x3244),
            WORD(0x7896)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var sym1 = $3244, sym2 = $7896\n"
            "$1000: $44 $32           .db $44, $32           .word sym1,sym2\n"
            "$1002: $96 $78           .db $96, $78        \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // var_def
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, var_def_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var sym1 = $3244, sym2, sym3 = $7896\n"
            "   .word sym1, sym2, sym3\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x3244),
            WORD(0x0000),
            WORD(0x7896),
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "6 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var sym1 = $3244, sym2, sym3 = $7896\n"
            "$1000: $44 $32           .db $44, $32           .word sym1, sym2, sym3\n"
            "$1002: $00 $00           .db $00, $00        \n"
            "$1004: $96 $78           .db $96, $78        \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // symbol_assign
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, symbol_assign_equals_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   sym1 = $3244\n"
            "   .word sym1\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x3244)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "           sym1 $3244  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                sym1 = $3244\n"
            "$1000: $44 $32           .db $44, $32           .word sym1\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, symbol_assign_equ_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   sym1 .EQU $3244\n"
            "   .word sym1\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x3244)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "           sym1 $3244  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                sym1 .EQU $3244\n"
            "$1000: $44 $32           .db $44, $32           .word sym1\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // pc_assign
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, pc_assign_equals_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   * = $300\n"
            "here\n"
            "   * = * + 2\n"
            "there\n"
            "   .org $1000\n"
            "   .word there\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0302)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "          there $0302  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                * = $300\n"
            "                                             here\n"
            "                                                * = * + 2\n"
            "                                             there\n"
            "                                                .org $1000\n"
            "$1000: $02 $03           .db $02, $03           .word there\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, pc_assign_equ_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   * .equ $0300\n"
            "   here\n"
            "   .org $1000\n"
            "   .word here\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0300)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "           here $0300  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"

            "                                                * .equ $0300\n"
            "                                                here\n"
            "                                                .org $1000\n"
            "$1000: $00 $03           .db $00, $03           .word here\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // symbol_value
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, symbol_value_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   SYM"
            "   .word SYM\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x1000)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "            SYM $1000  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $00 $10           .db $00, $10           SYM   .word SYM\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    /////////////////////////////////////////////////////
    // opcode
    /////////////////////////////////////////////////////
    TEST(assemble_unit_test, opcode_opcode_i_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   nop\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xea
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $EA               nop                    nop\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, opcode_opcode_I_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   ldx #$05\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa2, 0x05
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $A2 $05           ldx #$05               ldx #$05\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, opcode_opcode_a_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   asl a\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x0a
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $0A               asl                    asl a\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, opcode_opcode_ax_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   sta $2000,x\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x9d, WORD(0x2000)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "3 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $9D $00 $20       sta $2000,x            sta $2000,x\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, opcode_opcode_ay_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   sta $2000,y\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x99, WORD(0x2000)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "3 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $99 $00 $20       sta $2000,y            sta $2000,y\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, opcode_opcode_ind_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   jmp ($2000)\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x6c, WORD(0x2000)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "3 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $6C $00 $20       jmp ($2000)            jmp ($2000)\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, opcode_opcode_aix_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   lda ($44,x)\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa1, 0x44
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $A1 $44           lda ($44,x)            lda ($44,x)\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, opcode_opcode_izy_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   lda ($44),y\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xb1, 0x44
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $B1 $44           lda ($44),y            lda ($44),y\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    //////////////////////////////////////////////////////////////
    /// directive
    //////////////////////////////////////////////////////////////
    TEST(assemble_unit_test, directive_org_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .dw *\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x1000)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $00 $10           .db $00, $10           .dw *\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_ds_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   * = $100\n"
            "here .ds 2\n"
            "   .org $1000\n"
            "   .dw *\n"
            "   .dw here\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x1000),
            WORD(0x0100)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "           here $0100  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                * = $100\n"
            "                                             here .ds 2\n"
            "                                                .org $1000\n"
            "$1000: $00 $10           .db $00, $10           .dw *\n"
            "$1002: $00 $01           .db $00, $01           .dw here\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_byte_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .byte $02, $04, $06, $08, $0A\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x02, 0x04, 0x06, 0x08, 0x0a
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "5 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $02               .db $02                .byte $02, $04, $06, $08, $0A\n"
            "$1001: $04               .db $04             \n"
            "$1002: $06               .db $06             \n"
            "$1003: $08               .db $08             \n"
            "$1004: $0A               .db $0A             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_word_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $02, $04, $06, $08, $0A, $1234\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x02),
            WORD(0x04),
            WORD(0x06),
            WORD(0x08),
            WORD(0x0a),
            WORD(0x1234)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $02 $00           .db $02, $00           .word $02, $04, $06, $08, $0A, $1234\n"
            "$1002: $04 $00           .db $04, $00        \n"
            "$1004: $06 $00           .db $06, $00        \n"
            "$1006: $08 $00           .db $08, $00        \n"
            "$1008: $0A $00           .db $0A, $00        \n"
            "$100A: $34 $12           .db $34, $12        \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_str_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .text \"One Plus One = Two\", $00\n"
            ;

        constexpr unsigned char expected[] =
        {
            "One Plus One = Two"
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "19 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $4F $6E $65       .db 'O', 'n', 'e'      .text \"One Plus One = Two\", $00\n"
            "$1003: $20 $50 $6C       .db ' ', 'P', 'l'   \n"
            "$1006: $75 $73 $20       .db 'u', 's', ' '   \n"
            "$1009: $4F $6E $65       .db 'O', 'n', 'e'   \n"
            "$100C: $20 $3D $20       .db ' ', '=', ' '   \n"
            "$100F: $54 $77 $6F       .db 'T', 'w', 'o'   \n"
            "$1012: $00               .db $00             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_fill_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .fill $01, $0a\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "10 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $01 $01           .db $01, $01           .fill $01, $0a\n"
            "$1002: $01 $01           .db $01, $01        \n"
            "$1004: $01 $01           .db $01, $01        \n"
            "$1006: $01 $01           .db $01, $01        \n"
            "$1008: $01 $01           .db $01, $01        \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_print_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .print\n"
            "   nop\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xea
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .print\n"
            "$1000: $EA               nop                    nop\n";

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_print_expession_list_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .print \"123\",65\n"
            "   nop\n"
            "\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xea
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            " 123 $41\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .print \"123\",65\n"
            "$1000: $EA               nop                    nop\n"
            ;
        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_printall_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .printall\n"
            "   nop\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xea
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .printall\n"
            "$1000: $EA               nop                    nop\n";

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_printall_expession_list_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .printall \"123\",65\n"
            "   nop\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xea
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            " 123 $41\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            " 123 $41\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .printall \"123\",65\n"
            "$1000: $EA               nop                    nop\n";

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_printon_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "    ldx #9\n"
            "    nop\n"
            "    .print off\n"
            "    asl\n"
            "    ldy #2\n"
            "    .print on\n"
            "    lda #39\n"
            "    ldy $4\n"
            "    sta $30\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa2, 0x09,
            0xea,
            0x0a,
            0xa0, 0x02,
            0xa9, 0x27,
            0xa4, 0x04,
            0x85, 0x30
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "$1000: $A2 $09           ldx #$09                ldx #9\n"
            "$1002: $EA               nop                     nop\n"
            "$1006: $A9 $27           lda #$27                lda #39\n"
            "$1008: $A4 $04           ldy $04                 ldy $4\n"
            "$100A: $85 $30           sta $30                 sta $30\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_printoff_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "    ldx #9\n"
            "    nop\n"
            "    .print off\n"
            "    asl\n"
            "    ldy #2\n"
            "    .print on\n"
            "    lda #39\n"
            "    ldy $4\n"
            "    sta $30\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa2, 0x09,
            0xea,
            0x0a,
            0xa0, 0x02,
            0xa9, 0x27,
            0xa4, 0x04,
            0x85, 0x30
        };


        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "12 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "$1000: $A2 $09           ldx #$09                ldx #9\n"
            "$1002: $EA               nop                     nop\n"
            "$1006: $A9 $27           lda #$27                lda #39\n"
            "$1008: $A4 $04           ldy $04                 ldy $4\n"
            "$100A: $85 $30           sta $30                 sta $30\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_section_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "    .SECTION TOP\n"
            "    ldx #$02\n"
            "\n"
            "here\n"
            "    dex\n"
            "    bne here\n"
            "    .ENDSECTION\n"
            "\n"
            "here\n"
            "    nop\n"
            "    bne here\n"
            "    bne TOP.HERE\n"
            "\n"
            "    rts\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa2, 0x02,
            0xca,
            0xd0, 0xfd,
            0xea,
            0xd0, 0xfd,
            0xd0, 0xf8,
            0x60
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "11 bytes written to $$BIN_FILE$$\n"
            "\n"
            "       TOP.here $1002             here $1005  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "                                                 .SECTION TOP\n"
            "$1000: $A2 $02           ldx #$02                ldx #$02\n"
            "                                             \n"
            "                                             here\n"
            "$1002: $CA               dex                     dex\n"
            "$1003: $D0 $FD           bne $1002               bne here\n"
            "                                                 .ENDSECTION\n"
            "                                             \n"
            "                                             here\n"
            "$1005: $EA               nop                     nop\n"
            "$1006: $D0 $FD           bne $1005               bne here\n"
            "$1008: $D0 $F8           bne $1002               bne TOP.HERE\n"
            "                                             \n"
            "$100A: $60               rts                     rts\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_endsection_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "    .SECTION TOP\n"
            "    ldx #$02\n"
            "\n"
            "here\n"
            "    dex\n"
            "    bne here\n"
            "    .ENDSECTION\n"
            "\n"
            "here\n"
            "    nop\n"
            "    bne here\n"
            "    bne TOP.HERE\n"
            "\n"
            "    rts\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xa2, 0x02,
            0xca,
            0xd0, 0xfd,
            0xea,
            0xd0, 0xfd,
            0xd0, 0xf8,
            0x60
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "11 bytes written to $$BIN_FILE$$\n"
            "\n"
            "       TOP.here $1002             here $1005  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "                                                 .SECTION TOP\n"
            "$1000: $A2 $02           ldx #$02                ldx #$02\n"
            "                                             \n"
            "                                             here\n"
            "$1002: $CA               dex                     dex\n"
            "$1003: $D0 $FD           bne $1002               bne here\n"
            "                                                 .ENDSECTION\n"
            "                                             \n"
            "                                             here\n"
            "$1005: $EA               nop                     nop\n"
            "$1006: $D0 $FD           bne $1005               bne here\n"
            "$1008: $D0 $F8           bne $1002               bne TOP.HERE\n"
            "                                             \n"
            "$100A: $60               rts                     rts\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_end_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "    nop\n"
            "   .END\n"
            "\n"
            "    nop\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xea
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "$1000: $EA               nop                     nop\n"
            "                                                .END\n"
            "                                             \n"
            "                                                 nop\n"
            ;
        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, directive_inc_unit_test)
    {
        assemble_unit_test_method_initialize();

        const auto inc_text =
            "; include file\n"
            "   nop\n"
            "   brk\n"
            "\n"
            ;

        std::string code =
            "    .org $1000\n"
            "    .inc \"test_include.a\"\n"
            "    jmp $1234\n"
            "    nop\n"
            ;

        constexpr unsigned char expected[] =
        {
            0xea,
            0X00,
            0X4c, WORD(0x1234),
            0xea
        };

        remove("test_include.a");
        const auto inc_file = open_file("test_include.a", "w");
        EXPECT_NOT_NULL(inc_file);
        fwrite(inc_text, 1, strlen(inc_text), inc_file);
        fclose(inc_file);

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "6 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "                                                 .inc \"test_include.a\"\n"
            "; Processing test_include.a\n"
            "                                             ; include file\n"
            "$1000: $EA               nop                    nop\n"
            "$1001: $00               brk                    brk\n"
            "                                             \n"
            "; Processing $$FILE_NAME$$\n"
            "                                             \n"
            "$1002: $4C $34 $12       jmp $1234               jmp $1234\n"
            "$1005: $EA               nop                     nop\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();

        remove("test_include.a");
    }

    TEST(assemble_unit_test, directive_load_unit_test)
    {
        assemble_unit_test_method_initialize();

        constexpr unsigned char bin[] =
        {
            0x01, 0x02, 0x03
        };

        std::string code =
            "    .org $1000\n"
            "    .load \"test_load.bin\"\n"
            "    jmp $1234\n"
            "    nop\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x01, 0X02, 0x03,
            0X4c, WORD(0x1234),
            0xea
        };

        const auto load_file = open_file("test_load.bin", "wb");
        EXPECT_NOT_NULL(load_file);
        fwrite(bin, 1, _countof(bin), load_file);
        fclose(load_file);

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "7 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "$1000: $01 $02           .db $01, $02            .load \"test_load.bin\"\n"
            "$1002: $03               .db $03             \n"
            "$1003: $4C $34 $12       jmp $1234               jmp $1234\n"
            "$1006: $EA               nop                     nop\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();

        remove("test_load.bin");
    }

    //////////////////////////////////////////////////////////////
    /// expr
    //////////////////////////////////////////////////////////////
    TEST(assemble_unit_test, expr_integer_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var expression\n"
            "   expression = 12345\n"
            "   .word expression\n"
            "   .word 567, 89\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(12345),
            WORD(567),
            WORD(89)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "6 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var expression\n"
            "                                                expression = 12345\n"
            "$1000: $39 $30           .db $39, $30           .word expression\n"
            "$1002: $37 $02           .db $37, $02           .word 567, 89\n"
            "$1004: $59 $00           .db $59, $00        \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_symbol_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "expression1\n"
            "   .word expression1\n"
            "expression2"
            "   .word expression2\n"
            "   .word expression1 + expression2\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x1000),
            WORD(0x1002),
            WORD(0x2002)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "6 bytes written to $$BIN_FILE$$\n"
            "\n"
            "    expression1 $1000      expression2 $1002  \n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                             expression1\n"
            "$1000: $00 $10           .db $00, $10           .word expression1\n"
            "$1002: $02 $10           .db $02, $10        expression2   .word expression2\n"
            "$1004: $02 $20           .db $02, $20           .word expression1 + expression2\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_uminus_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word -($1359)\n"
            "   .byte -2\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0xECA7),
            0xfe
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "3 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $A7 $EC           .db $A7, $EC           .word -($1359)\n"
            "$1002: $FE               .db $FE                .byte -2\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_tilda_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word ~($1359)\n"
            "   .byte ~2\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0xeca6),
            0xfd
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "3 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $A6 $EC           .db $A6, $EC           .word ~($1359)\n"
            "$1002: $FD               .db $FD                .byte ~2\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_lobyte_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .byte <$1234\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x34
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $34               .db $34                .byte <$1234\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_hibyte_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .byte >$1234\n"
            ;

        constexpr unsigned char expected[] =
        {
            0x12
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "1 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $12               .db $12                .byte >$1234\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_not_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word NOT 0\n"
            "   .byte NOT 1 < 2, NOT 5\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0001),
            0x00, 0x00
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $01 $00           .db $01, $00           .word NOT 0\n"
            "$1002: $00               .db $00                .byte NOT 1 < 2, NOT 5\n"
            "$1003: $00               .db $00             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_or_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word 0 || 0\n"
            "   .byte 1 || 2, 0 || 1\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0000),
            0x01, 0x01
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $00 $00           .db $00, $00           .word 0 || 0\n"
            "$1002: $01               .db $01                .byte 1 || 2, 0 || 1\n"
            "$1003: $01               .db $01             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_and_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word 0 && 0\n"
            "   .byte 1 && 2, 0 && 1\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0000),
            0x01, 0x00
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $00 $00           .db $00, $00           .word 0 && 0\n"
            "$1002: $01               .db $01                .byte 1 && 2, 0 && 1\n"
            "$1003: $00               .db $00             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_shift_left_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word 1 << 8\n"
            "   .byte 1 << 2, 3 << 1 << 2\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0100),
            0x04, 0x18
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $00 $01           .db $00, $01           .word 1 << 8\n"
            "$1002: $04               .db $04                .byte 1 << 2, 3 << 1 << 2\n"
            "$1003: $18               .db $18             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_shift_right_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $0100 >> 8\n"
            "   .byte 4 >> 2, $18 >> 1 >>2\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0001),
            0x01, 0x03
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $01 $00           .db $01, $00           .word $0100 >> 8\n"
            "$1002: $01               .db $01                .byte 4 >> 2, $18 >> 1 >>2\n"
            "$1003: $03               .db $03             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_less_than_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $01 < $08\n"
            "   .byte 4 < 2, $18 < $55\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0001),
            0x00, 0x01
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $01 $00           .db $01, $00           .word $01 < $08\n"
            "$1002: $00               .db $00                .byte 4 < 2, $18 < $55\n"
            "$1003: $01               .db $01             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_greater_than_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $08 > $01\n"
            "   .byte 2 > 4, $55 > $18\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0001),
            0x00, 0x01
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $01 $00           .db $01, $00           .word $08 > $01\n"
            "$1002: $00               .db $00                .byte 2 > 4, $55 > $18\n"
            "$1003: $01               .db $01             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_greater_than_equal_to_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $08 >= $08\n"
            "   .byte 2 >= 4, $55 >= $18\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0001),
            0x00, 0x01
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $01 $00           .db $01, $00           .word $08 >= $08\n"
            "$1002: $00               .db $00                .byte 2 >= 4, $55 >= $18\n"
            "$1003: $01               .db $01             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_less_than_equal_to_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $08 <= $08\n"
            "   .byte 2 <= 4, $55 <= $18\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0001),
            0x01, 0x00
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $01 $00           .db $01, $00           .word $08 <= $08\n"
            "$1002: $01               .db $01                .byte 2 <= 4, $55 <= $18\n"
            "$1003: $00               .db $00             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_equal_to_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $08 == $08\n"
            "   .byte 2 == 4, $55 == $18\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0001),
            0x00, 0x00
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $01 $00           .db $01, $00           .word $08 == $08\n"
            "$1002: $00               .db $00                .byte 2 == 4, $55 == $18\n"
            "$1003: $00               .db $00             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_not_equal_to_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $08 != $08\n"
            "   .byte 2 != 4, $55 != $18\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0000),
            0x01, 0x01
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $00 $00           .db $00, $00           .word $08 != $08\n"
            "$1002: $01               .db $01                .byte 2 != 4, $55 != $18\n"
            "$1003: $01               .db $01             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_bit_and_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $78 & $08\n"
            "   .byte 2 & 4, $55 & $18\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0008),
            0x00, 0x10
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $08 $00           .db $08, $00           .word $78 & $08\n"
            "$1002: $00               .db $00                .byte 2 & 4, $55 & $18\n"
            "$1003: $10               .db $10             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_bit_or_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $78 | $84\n"
            "   .byte 2 | 4, $55 | $18\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x00fc),
            0x06, 0x5d
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $FC $00           .db $FC, $00           .word $78 | $84\n"
            "$1002: $06               .db $06                .byte 2 | 4, $55 | $18\n"
            "$1003: $5D               .db $5D             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_bit_xor_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $78 ^ $06\n"
            "   .byte 2 ^ 4, $55 ^ $18\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x007E),
            0x06, 0x4D
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $7E $00           .db $7E, $00           .word $78 ^ $06\n"
            "$1002: $06               .db $06                .byte 2 ^ 4, $55 ^ $18\n"
            "$1003: $4D               .db $4D             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_plus_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $78 + $06\n"
            "   .byte 2 + 4, $55 + $18\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x007E),
            0x06, 0x6D
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $7E $00           .db $7E, $00           .word $78 + $06\n"
            "$1002: $06               .db $06                .byte 2 + 4, $55 + $18\n"
            "$1003: $6D               .db $6D             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_minus_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $78 - $06\n"
            "   .byte 2 - 4, $55 - $18\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0072),
            0xfe, 0x3d
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $72 $00           .db $72, $00           .word $78 - $06\n"
            "$1002: $FE               .db $FE                .byte 2 - 4, $55 - $18\n"
            "$1003: $3D               .db $3D             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_multiply_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $78 * $06\n"
            "   .byte 2 * 4, $15 * $08\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x02d0),
            0x08, 0xa8
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $D0 $02           .db $D0, $02           .word $78 * $06\n"
            "$1002: $08               .db $08                .byte 2 * 4, $15 * $08\n"
            "$1003: $A8               .db $A8             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_divide_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .word $78 / $06\n"
            "   .byte 4 / 2, $15 / $08\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0014),
            0x02, 0x02
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "4 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "$1000: $14 $00           .db $14, $00           .word $78 / $06\n"
            "$1002: $02               .db $02                .byte 4 / 2, $15 / $08\n"
            "$1003: $02               .db $02             \n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_back_branch_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "    Start\n"
            "    nop\n"
            "\n"
            "    beq +\n"
            "-    \n"
            "    lda ++,y\n"
            "    bne -\n"
            "    nop\n"
            "-\n"
            "    bpl -\n"
            "    jmp (--)\n"
            "    rts\n"
            "    \n"
            "+\n"
            "    rts\n"
            "    jsr ++\n"
            "+\n"
            "    nop\n"
            "    nop\n"
            "    rts\n"
            "\n"
            "    beq --\n"
            "+   rts\n"
            ;

        const unsigned char expected[] =
        {
            0xea,
            0xf0, 0x0c,
            0xb9, 0x13, 0x10,
            0xd0, 0xfb,
            0xea,
            0x10, 0xfe,
            0x6c, 0x03, 0x10,
            0x60,
            0x60,
            0x20, 0x18, 0x10,
            0xea,
            0xea,
            0x60,
            0xf0, 0xeb,
            0x60
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "25 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "                                                 Start\n"
            "$1000: $EA               nop                     nop\n"
            "                                             \n"
            "$1001: $F0 $0C           beq $100F               beq +\n"
            "                                             -    \n"
            "$1003: $B9 $13 $10       lda $1013,y             lda ++,y\n"
            "$1006: $D0 $FB           bne $1003               bne -\n"
            "$1008: $EA               nop                     nop\n"
            "                                             -\n"
            "$1009: $10 $FE           bpl $1009               bpl -\n"
            "$100B: $6C $03 $10       jmp ($1003)             jmp (--)\n"
            "$100E: $60               rts                     rts\n"
            "                                                 \n"
            "                                             +\n"
            "$100F: $60               rts                     rts\n"
            "$1010: $20 $18 $10       jsr $1018               jsr ++\n"
            "                                             +\n"
            "$1013: $EA               nop                     nop\n"
            "$1014: $EA               nop                     nop\n"
            "$1015: $60               rts                     rts\n"
            "                                             \n"
            "$1016: $F0 $EB           beq $1003               beq --\n"
            "$1018: $60               rts                 +   rts\n"
            ;
        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expr_forward_branch_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $1000\n"
            "    Start\n"
            "    nop\n"
            "\n"
            "    beq +\n"
            "-    \n"
            "    lda ++,y\n"
            "    bne -\n"
            "    nop\n"
            "-\n"
            "    bpl -\n"
            "    jmp (--)\n"
            "    rts\n"
            "    \n"
            "+\n"
            "    rts\n"
            "    jsr ++\n"
            "+\n"
            "    nop\n"
            "    nop\n"
            "    rts\n"
            "\n"
            "    beq --\n"
            "+   rts\n";

        const unsigned char expected[] =
        {
            0xea,
            0xf0, 0x0c,
            0xb9, 0x13, 0x10,
            0xd0, 0xfb,
            0xea,
            0x10, 0xfe,
            0x6c, 0x03, 0x10,
            0x60,
            0x60,
            0x20, 0x18, 0x10,
            0xea,
            0xea,
            0x60,
            0xf0, 0xeb,
            0x60
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Pass 2\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "25 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $1000\n"
            "                                                 Start\n"
            "$1000: $EA               nop                     nop\n"
            "                                             \n"
            "$1001: $F0 $0C           beq $100F               beq +\n"
            "                                             -    \n"
            "$1003: $B9 $13 $10       lda $1013,y             lda ++,y\n"
            "$1006: $D0 $FB           bne $1003               bne -\n"
            "$1008: $EA               nop                     nop\n"
            "                                             -\n"
            "$1009: $10 $FE           bpl $1009               bpl -\n"
            "$100B: $6C $03 $10       jmp ($1003)             jmp (--)\n"
            "$100E: $60               rts                     rts\n"
            "                                                 \n"
            "                                             +\n"
            "$100F: $60               rts                     rts\n"
            "$1010: $20 $18 $10       jsr $1018               jsr ++\n"
            "                                             +\n"
            "$1013: $EA               nop                     nop\n"
            "$1014: $EA               nop                     nop\n"
            "$1015: $60               rts                     rts\n"
            "                                             \n"
            "$1016: $F0 $EB           beq $1003               beq --\n"
            "$1018: $60               rts                 +   rts\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    //////////////////////////////////////////////////////////////
    /// subexpr
    //////////////////////////////////////////////////////////////
    TEST(assemble_unit_test, subexpr_expr_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var expression\n"
            "   expression = 12345\n"
            "   .word expression\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(12345)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var expression\n"
            "                                                expression = 12345\n"
            "$1000: $39 $30           .db $39, $30           .word expression\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, subexpr_program_counter_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var expression\n"
            "   expression = *\n"
            "   .word expression\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x1000)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var expression\n"
            "                                                expression = *\n"
            "$1000: $00 $10           .db $00, $10           .word expression\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, subexpr_parenthesis_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var expression\n"
            "   expression = 5 * (6 + 4)\n"
            "   .word expression\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x32)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var expression\n"
            "                                                expression = 5 * (6 + 4)\n"
            "$1000: $32 $00           .db $32, $00           .word expression\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, expression_evaluator_unit_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "   .org $1000\n"
            "   .var expression\n"
            "   expression = 5 + 3 * (6 + 4) / 7 + 6 / 3 + 1\n"
            "   .word expression\n"
            ;

        constexpr unsigned char expected[] =
        {
            WORD(0x0c)
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "2 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                .org $1000\n"
            "                                                .var expression\n"
            "                                                expression = 5 + 3 * (6 + 4) / 7 + 6 / 3 + 1\n"
            "$1000: $0C $00           .db $0C, $00           .word expression\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }

    TEST(assemble_unit_test, program_counter_test)
    {
        assemble_unit_test_method_initialize();

        std::string code =
            "    .org $c000\n"
            "    .repeat\n"
            "        .byte 1\n"
            "    .until * > $c0FF\n"
            "    nop\n";

        const unsigned char expected[] =
        {
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xea
        };

        std::string expect_list =
            "pasm.exe -v $$FILE_NAME$$ -o $$BIN_FILE$$ \n"
            "\n"
            "Pass 1\n"
            "Current File $$FILE_NAME$$\n"
            "Final Pass\n"
            "Current File $$FILE_NAME$$\n"
            "\n"
            "257 bytes written to $$BIN_FILE$$\n"
            "\n"
            "\n"
            "; Processing $$FILE_NAME$$\n"
            "                                                 .org $c000\n"
            "                                                 .repeat\n"
            "                                                     .byte 1\n"
            "$C000: $01               .db $01                 .until * > $c0FF\n"
            "$C001: $01               .db $01             \n"
            "$C002: $01               .db $01             \n"
            "$C003: $01               .db $01             \n"
            "$C004: $01               .db $01             \n"
            "$C005: $01               .db $01             \n"
            "$C006: $01               .db $01             \n"
            "$C007: $01               .db $01             \n"
            "$C008: $01               .db $01             \n"
            "$C009: $01               .db $01             \n"
            "$C00A: $01               .db $01             \n"
            "$C00B: $01               .db $01             \n"
            "$C00C: $01               .db $01             \n"
            "$C00D: $01               .db $01             \n"
            "$C00E: $01               .db $01             \n"
            "$C00F: $01               .db $01             \n"
            "$C010: $01               .db $01             \n"
            "$C011: $01               .db $01             \n"
            "$C012: $01               .db $01             \n"
            "$C013: $01               .db $01             \n"
            "$C014: $01               .db $01             \n"
            "$C015: $01               .db $01             \n"
            "$C016: $01               .db $01             \n"
            "$C017: $01               .db $01             \n"
            "$C018: $01               .db $01             \n"
            "$C019: $01               .db $01             \n"
            "$C01A: $01               .db $01             \n"
            "$C01B: $01               .db $01             \n"
            "$C01C: $01               .db $01             \n"
            "$C01D: $01               .db $01             \n"
            "$C01E: $01               .db $01             \n"
            "$C01F: $01               .db $01             \n"
            "$C020: $01               .db $01             \n"
            "$C021: $01               .db $01             \n"
            "$C022: $01               .db $01             \n"
            "$C023: $01               .db $01             \n"
            "$C024: $01               .db $01             \n"
            "$C025: $01               .db $01             \n"
            "$C026: $01               .db $01             \n"
            "$C027: $01               .db $01             \n"
            "$C028: $01               .db $01             \n"
            "$C029: $01               .db $01             \n"
            "$C02A: $01               .db $01             \n"
            "$C02B: $01               .db $01             \n"
            "$C02C: $01               .db $01             \n"
            "$C02D: $01               .db $01             \n"
            "$C02E: $01               .db $01             \n"
            "$C02F: $01               .db $01             \n"
            "$C030: $01               .db $01             \n"
            "$C031: $01               .db $01             \n"
            "$C032: $01               .db $01             \n"
            "$C033: $01               .db $01             \n"
            "$C034: $01               .db $01             \n"
            "$C035: $01               .db $01             \n"
            "$C036: $01               .db $01             \n"
            "$C037: $01               .db $01             \n"
            "$C038: $01               .db $01             \n"
            "$C039: $01               .db $01             \n"
            "$C03A: $01               .db $01             \n"
            "$C03B: $01               .db $01             \n"
            "$C03C: $01               .db $01             \n"
            "$C03D: $01               .db $01             \n"
            "$C03E: $01               .db $01             \n"
            "$C03F: $01               .db $01             \n"
            "$C040: $01               .db $01             \n"
            "$C041: $01               .db $01             \n"
            "$C042: $01               .db $01             \n"
            "$C043: $01               .db $01             \n"
            "$C044: $01               .db $01             \n"
            "$C045: $01               .db $01             \n"
            "$C046: $01               .db $01             \n"
            "$C047: $01               .db $01             \n"
            "$C048: $01               .db $01             \n"
            "$C049: $01               .db $01             \n"
            "$C04A: $01               .db $01             \n"
            "$C04B: $01               .db $01             \n"
            "$C04C: $01               .db $01             \n"
            "$C04D: $01               .db $01             \n"
            "$C04E: $01               .db $01             \n"
            "$C04F: $01               .db $01             \n"
            "$C050: $01               .db $01             \n"
            "$C051: $01               .db $01             \n"
            "$C052: $01               .db $01             \n"
            "$C053: $01               .db $01             \n"
            "$C054: $01               .db $01             \n"
            "$C055: $01               .db $01             \n"
            "$C056: $01               .db $01             \n"
            "$C057: $01               .db $01             \n"
            "$C058: $01               .db $01             \n"
            "$C059: $01               .db $01             \n"
            "$C05A: $01               .db $01             \n"
            "$C05B: $01               .db $01             \n"
            "$C05C: $01               .db $01             \n"
            "$C05D: $01               .db $01             \n"
            "$C05E: $01               .db $01             \n"
            "$C05F: $01               .db $01             \n"
            "$C060: $01               .db $01             \n"
            "$C061: $01               .db $01             \n"
            "$C062: $01               .db $01             \n"
            "$C063: $01               .db $01             \n"
            "$C064: $01               .db $01             \n"
            "$C065: $01               .db $01             \n"
            "$C066: $01               .db $01             \n"
            "$C067: $01               .db $01             \n"
            "$C068: $01               .db $01             \n"
            "$C069: $01               .db $01             \n"
            "$C06A: $01               .db $01             \n"
            "$C06B: $01               .db $01             \n"
            "$C06C: $01               .db $01             \n"
            "$C06D: $01               .db $01             \n"
            "$C06E: $01               .db $01             \n"
            "$C06F: $01               .db $01             \n"
            "$C070: $01               .db $01             \n"
            "$C071: $01               .db $01             \n"
            "$C072: $01               .db $01             \n"
            "$C073: $01               .db $01             \n"
            "$C074: $01               .db $01             \n"
            "$C075: $01               .db $01             \n"
            "$C076: $01               .db $01             \n"
            "$C077: $01               .db $01             \n"
            "$C078: $01               .db $01             \n"
            "$C079: $01               .db $01             \n"
            "$C07A: $01               .db $01             \n"
            "$C07B: $01               .db $01             \n"
            "$C07C: $01               .db $01             \n"
            "$C07D: $01               .db $01             \n"
            "$C07E: $01               .db $01             \n"
            "$C07F: $01               .db $01             \n"
            "$C080: $01               .db $01             \n"
            "$C081: $01               .db $01             \n"
            "$C082: $01               .db $01             \n"
            "$C083: $01               .db $01             \n"
            "$C084: $01               .db $01             \n"
            "$C085: $01               .db $01             \n"
            "$C086: $01               .db $01             \n"
            "$C087: $01               .db $01             \n"
            "$C088: $01               .db $01             \n"
            "$C089: $01               .db $01             \n"
            "$C08A: $01               .db $01             \n"
            "$C08B: $01               .db $01             \n"
            "$C08C: $01               .db $01             \n"
            "$C08D: $01               .db $01             \n"
            "$C08E: $01               .db $01             \n"
            "$C08F: $01               .db $01             \n"
            "$C090: $01               .db $01             \n"
            "$C091: $01               .db $01             \n"
            "$C092: $01               .db $01             \n"
            "$C093: $01               .db $01             \n"
            "$C094: $01               .db $01             \n"
            "$C095: $01               .db $01             \n"
            "$C096: $01               .db $01             \n"
            "$C097: $01               .db $01             \n"
            "$C098: $01               .db $01             \n"
            "$C099: $01               .db $01             \n"
            "$C09A: $01               .db $01             \n"
            "$C09B: $01               .db $01             \n"
            "$C09C: $01               .db $01             \n"
            "$C09D: $01               .db $01             \n"
            "$C09E: $01               .db $01             \n"
            "$C09F: $01               .db $01             \n"
            "$C0A0: $01               .db $01             \n"
            "$C0A1: $01               .db $01             \n"
            "$C0A2: $01               .db $01             \n"
            "$C0A3: $01               .db $01             \n"
            "$C0A4: $01               .db $01             \n"
            "$C0A5: $01               .db $01             \n"
            "$C0A6: $01               .db $01             \n"
            "$C0A7: $01               .db $01             \n"
            "$C0A8: $01               .db $01             \n"
            "$C0A9: $01               .db $01             \n"
            "$C0AA: $01               .db $01             \n"
            "$C0AB: $01               .db $01             \n"
            "$C0AC: $01               .db $01             \n"
            "$C0AD: $01               .db $01             \n"
            "$C0AE: $01               .db $01             \n"
            "$C0AF: $01               .db $01             \n"
            "$C0B0: $01               .db $01             \n"
            "$C0B1: $01               .db $01             \n"
            "$C0B2: $01               .db $01             \n"
            "$C0B3: $01               .db $01             \n"
            "$C0B4: $01               .db $01             \n"
            "$C0B5: $01               .db $01             \n"
            "$C0B6: $01               .db $01             \n"
            "$C0B7: $01               .db $01             \n"
            "$C0B8: $01               .db $01             \n"
            "$C0B9: $01               .db $01             \n"
            "$C0BA: $01               .db $01             \n"
            "$C0BB: $01               .db $01             \n"
            "$C0BC: $01               .db $01             \n"
            "$C0BD: $01               .db $01             \n"
            "$C0BE: $01               .db $01             \n"
            "$C0BF: $01               .db $01             \n"
            "$C0C0: $01               .db $01             \n"
            "$C0C1: $01               .db $01             \n"
            "$C0C2: $01               .db $01             \n"
            "$C0C3: $01               .db $01             \n"
            "$C0C4: $01               .db $01             \n"
            "$C0C5: $01               .db $01             \n"
            "$C0C6: $01               .db $01             \n"
            "$C0C7: $01               .db $01             \n"
            "$C0C8: $01               .db $01             \n"
            "$C0C9: $01               .db $01             \n"
            "$C0CA: $01               .db $01             \n"
            "$C0CB: $01               .db $01             \n"
            "$C0CC: $01               .db $01             \n"
            "$C0CD: $01               .db $01             \n"
            "$C0CE: $01               .db $01             \n"
            "$C0CF: $01               .db $01             \n"
            "$C0D0: $01               .db $01             \n"
            "$C0D1: $01               .db $01             \n"
            "$C0D2: $01               .db $01             \n"
            "$C0D3: $01               .db $01             \n"
            "$C0D4: $01               .db $01             \n"
            "$C0D5: $01               .db $01             \n"
            "$C0D6: $01               .db $01             \n"
            "$C0D7: $01               .db $01             \n"
            "$C0D8: $01               .db $01             \n"
            "$C0D9: $01               .db $01             \n"
            "$C0DA: $01               .db $01             \n"
            "$C0DB: $01               .db $01             \n"
            "$C0DC: $01               .db $01             \n"
            "$C0DD: $01               .db $01             \n"
            "$C0DE: $01               .db $01             \n"
            "$C0DF: $01               .db $01             \n"
            "$C0E0: $01               .db $01             \n"
            "$C0E1: $01               .db $01             \n"
            "$C0E2: $01               .db $01             \n"
            "$C0E3: $01               .db $01             \n"
            "$C0E4: $01               .db $01             \n"
            "$C0E5: $01               .db $01             \n"
            "$C0E6: $01               .db $01             \n"
            "$C0E7: $01               .db $01             \n"
            "$C0E8: $01               .db $01             \n"
            "$C0E9: $01               .db $01             \n"
            "$C0EA: $01               .db $01             \n"
            "$C0EB: $01               .db $01             \n"
            "$C0EC: $01               .db $01             \n"
            "$C0ED: $01               .db $01             \n"
            "$C0EE: $01               .db $01             \n"
            "$C0EF: $01               .db $01             \n"
            "$C0F0: $01               .db $01             \n"
            "$C0F1: $01               .db $01             \n"
            "$C0F2: $01               .db $01             \n"
            "$C0F3: $01               .db $01             \n"
            "$C0F4: $01               .db $01             \n"
            "$C0F5: $01               .db $01             \n"
            "$C0F6: $01               .db $01             \n"
            "$C0F7: $01               .db $01             \n"
            "$C0F8: $01               .db $01             \n"
            "$C0F9: $01               .db $01             \n"
            "$C0FA: $01               .db $01             \n"
            "$C0FB: $01               .db $01             \n"
            "$C0FC: $01               .db $01             \n"
            "$C0FD: $01               .db $01             \n"
            "$C0FE: $01               .db $01             \n"
            "$C0FF: $01               .db $01             \n"
            "$C100: $EA               nop                     nop\n"
            ;

        execute_text(code, expected, _countof(expected), expect_list);
        assemble_unit_test_method_cleanup();
    }
}