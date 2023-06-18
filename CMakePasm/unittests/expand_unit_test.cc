#include <gtest/gtest.h>
#include "unit_test.h"

#pragma warning(disable: 4311 4090 4996 )

static int test_num = 0;

static void expand_unit_test_method_initialize()
{
    initialize();

    free_parse_tree();
    program_counter = 0;
    generate_output_reset();

    reset_lex();
    final_pass = true;
}

static void expand_unit_test_method_cleanup()
{
    destroy();

    if (output_file) 
    {
        fclose(output_file); 
        output_file = nullptr;
    }

    program_counter = 0;
    generate_output_reset();

    dict_destroy(symbol_dictionary);
    symbol_dictionary = nullptr;

    if (changed_sym_stack != nullptr)
    {
        free_stack(changed_sym_stack);
        changed_sym_stack = nullptr;
    }
}

static void expand_file_output_template(const unsigned char expected[], const size_t sz, const parse_node_ptr* nodes, const size_t num_nodes)
{    
    const auto output_file_name = "expand_file_output_template.bin";
    current_file_name = (char*)"test.a";
    output_file = open_file(output_file_name, "wb");

    yylineno = 1;
    for (size_t i = 0; i < num_nodes; ++i)
    {
        yylineno++;
        expand_node(nodes[i]);
    }

    fclose(output_file);
    
    output_file = nullptr;
    yylineno = 0;

    const unsigned char* buffer = static_cast<unsigned char*>(malloc(sz));
    if (buffer == nullptr) return;

    output_file = open_file(output_file_name, "rb");
    fseek(output_file, 0, SEEK_END);
    const size_t pos = ftell(output_file);
    fseek(output_file, 0, SEEK_SET);
    // ReSharper disable once CppCStyleCast
    fread((void*)buffer, 1, sz, output_file);
    fclose(output_file);

    output_file = nullptr;
    remove(output_file_name);

    EXPECT_EQ(sz, pos);

    for (auto i = 0; i < static_cast<int>((sz - 1)); ++i)
    {
        if (expected[i] != buffer[i])
        {
            const auto a = expected[i];
            const auto b = buffer[i];

            free((char*)buffer);  // NOLINT(clang-diagnostic-cast-qual)
            EXPECT_EQ(a, b);
            return;
        }
    }
  
    // ReSharper disable once CppCStyleCast
    free((void*)buffer);
}

TEST(expand_unit_test, expand_constant_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0x11
    };
    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, 
            constant_node(0x1000, false)),

        data_node(data_byte, operator_node(EXPRLIST, 1, 
            constant_node(0x11, false)))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_data_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        "expand_data_node_test"
    };

    parse_node_ptr nodes[] =
    {
        data_node(0, operator_node(EXPRLIST, 1, string_node("expand_data_node_test"))),
    };

    expand_file_output_template(expected, _countof(expected) -1, nodes, _countof(nodes));

    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_id_node_test)
{
    expand_unit_test_method_initialize();
    const auto output_file_name = "expand_id_node_test.bin";
    current_file_name = (char*)"test.a";
    output_file = open_file(output_file_name, "wb");

    const auto id_node_ptr = id_node( "test_id");
    EXPECT_NOT_NULL(id_node_ptr);
    const auto result = expand_node(id_node_ptr);
    EXPECT_EQ(0, result);
    EXPECT_NOT_NULL(id_node_ptr->id.name);

    auto str_result = stricmp( "test_id", id_node_ptr->id.name);

    EXPECT_EQ(0, str_result);
    const auto symbol_ptr = look_up_symbol( "test_id");
    EXPECT_NOT_NULL(symbol_ptr);
    str_result = stricmp( "test_id", symbol_ptr->name);
    EXPECT_EQ(0, str_result);

    fclose(output_file);
    output_file = nullptr;
    remove(output_file_name);
    
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_label_node_test)
{
    expand_unit_test_method_initialize();
    const auto label_node_ptr = label_node((char*)"label_name");
    EXPECT_NOT_NULL(label_node_ptr);
    const auto result = expand_node(label_node_ptr);
    EXPECT_EQ(0, result);
    auto str_result = stricmp( "label_name", label_node_ptr->id.name);
    EXPECT_EQ(0, str_result);

    const auto symbol_ptr = look_up_symbol("label_name");
    EXPECT_NOT_NULL(symbol_ptr);
    str_result = stricmp( "label_name", symbol_ptr->name);
    EXPECT_EQ(0, str_result);
    
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_macro_expansion_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0x69, 0x29, 0xEA
    };

    parse_node_ptr nodes[] =
    {
        operator_node(MACRO, 2,
            macro_id_node("macro_name"),
            operator_node(STATEMENT, 2,
                opcode_node(_adc, I, 1,
                    constant_node(0x29, false)),
                opcode_node(_nop, i, 0))),

        operator_node(ORG, 1, constant_node(0x1000, false)),

        macro_expand_node("macro_name", nullptr)
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_macro_id_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0x69, 0x49, 0xEA
    };

    parse_node_ptr nodes[] =
    {
        operator_node(MACRO, 2,
            macro_id_node("macro_name"),
            operator_node(STATEMENT, 2,
                opcode_node(_adc, I, 1,
                    constant_node(0x49, false)),
                opcode_node(_nop, i, 0))),

        operator_node(ORG, 1, constant_node(0x1000, false)),

        macro_expand_node("macro_name", nullptr)
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_op_code_node_test)
{
    expand_unit_test_method_initialize();
    current_file_name = (char*)"test.a";
    const auto* output_file_name = "expand_op_code_node_test.bin";
    output_file = open_file(output_file_name, "wb");
    int result;

    // org $1000;

    const parse_node_ptr org_value_node = constant_node(0x1000, 0);
    const parse_node_ptr org_node = operator_node(ORG, 1, org_value_node);
    const auto org_result = expand_node(org_node);
    EXPECT_EQ(0, org_result);
    EXPECT_EQ(0x1000, program_counter);


    ///////////////////////////////////////////////////////////
    cpu_mode = cpu_6502;
    allow_illegal_op_codes = false;
    ///////////////////////////////////////////////////////////

    // _adc
    result = expand_node(opcode_node(_adc, I, 1, constant_node(0x29, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1002, program_counter);
    result = expand_node(opcode_node(_adc, zp, 1, constant_node(0x6b, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1004, program_counter);
    result = expand_node(opcode_node(_adc, zpx, 1, constant_node(0xd6, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1006, program_counter);
    result = expand_node(opcode_node(_adc, a, 1, constant_node(0x6784, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1009, program_counter);
    result = expand_node(opcode_node(_adc, ax, 1, constant_node(0x4be1, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x100c, program_counter);
    result = expand_node(opcode_node(_adc, ay, 1, constant_node(0x3d6c, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x100f, program_counter);
    result = expand_node(opcode_node(_adc, izx, 1, constant_node(0x3, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1011, program_counter);
    result = expand_node(opcode_node(_adc, izy, 1, constant_node(0x21, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1013, program_counter);

    // _and
    result = expand_node(opcode_node(_and, I, 1, constant_node(0xbb, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1015, program_counter);
    result = expand_node(opcode_node(_and, zp, 1, constant_node(0xef, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1017, program_counter);
    result = expand_node(opcode_node(_and, zpx, 1, constant_node(0x5f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1019, program_counter);
    result = expand_node(opcode_node(_and, a, 1, constant_node(0x6df1, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x101c, program_counter);
    result = expand_node(opcode_node(_and, ax, 1, constant_node(0x5bf1, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x101f, program_counter);
    result = expand_node(opcode_node(_and, ay, 1, constant_node(0x41bb, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1022, program_counter);
    result = expand_node(opcode_node(_and, izx, 1, constant_node(0x10, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1024, program_counter);
    result = expand_node(opcode_node(_and, izy, 1, constant_node(0xec, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1026, program_counter);

    // _asl
    result = expand_node(opcode_node(_asl, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1027, program_counter);
    result = expand_node(opcode_node(_asl, A, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1028, program_counter);
    result = expand_node(opcode_node(_asl, zp, 1, constant_node(0xbe, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x102a, program_counter);
    result = expand_node(opcode_node(_asl, zpx, 1, constant_node(0xd4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x102c, program_counter);
    result = expand_node(opcode_node(_asl, a, 1, constant_node(0x13db, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x102f, program_counter);
    result = expand_node(opcode_node(_asl, ax, 1, constant_node(0x153c, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1032, program_counter);

    // _bcc
    result = expand_node(opcode_node(_bcc, r, 1, constant_node(0x103a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1034, program_counter);

    // _bcs
    result = expand_node(opcode_node(_bcs, r, 1, constant_node(0x107b, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1036, program_counter);

    // _beq
    result = expand_node(opcode_node(_beq, r, 1, constant_node(0x1085, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1038, program_counter);

    // _bit
    result = expand_node(opcode_node(_bit, zp, 1, constant_node(0x99, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x103a, program_counter);
    result = expand_node(opcode_node(_bit, a, 1, constant_node(0x124, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x103d, program_counter);

    // _bmi
    result = expand_node(opcode_node(_bmi, r, 1, constant_node(0x104e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x103f, program_counter);

    // _bne
    result = expand_node(opcode_node(_bne, r, 1, constant_node(0x1092, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1041, program_counter);

    // _bpl
    result = expand_node(opcode_node(_bpl, r, 1, constant_node(0x10a8, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1043, program_counter);

    // _brk
    result = expand_node(opcode_node(_brk, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1044, program_counter);

    // _bvc
    result = expand_node(opcode_node(_bvc, r, 1, constant_node(0x1099, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1046, program_counter);

    // _bvs
    result = expand_node(opcode_node(_bvs, r, 1, constant_node(0x104d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1048, program_counter);

    // _clc
    result = expand_node(opcode_node(_clc, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1049, program_counter);

    // _cld
    result = expand_node(opcode_node(_cld, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x104a, program_counter);

    // _cli
    result = expand_node(opcode_node(_cli, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x104b, program_counter);

    // _clv
    result = expand_node(opcode_node(_clv, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x104c, program_counter);

    // _cmp
    result = expand_node(opcode_node(_cmp, I, 1, constant_node(0x5c, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x104e, program_counter);
    result = expand_node(opcode_node(_cmp, zp, 1, constant_node(0x33, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1050, program_counter);
    result = expand_node(opcode_node(_cmp, zpx, 1, constant_node(0xec, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1052, program_counter);
    result = expand_node(opcode_node(_cmp, a, 1, constant_node(0x2d12, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1055, program_counter);
    result = expand_node(opcode_node(_cmp, ax, 1, constant_node(0x74d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1058, program_counter);
    result = expand_node(opcode_node(_cmp, ay, 1, constant_node(0x4dc8, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x105b, program_counter);
    result = expand_node(opcode_node(_cmp, izx, 1, constant_node(0xa7, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x105d, program_counter);
    result = expand_node(opcode_node(_cmp, izy, 1, constant_node(0x22, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x105f, program_counter);

    // _cpx
    result = expand_node(opcode_node(_cpx, I, 1, constant_node(0xcd, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1061, program_counter);
    result = expand_node(opcode_node(_cpx, zp, 1, constant_node(0xcc, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1063, program_counter);
    result = expand_node(opcode_node(_cpx, a, 1, constant_node(0x711f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1066, program_counter);

    // _cpy
    result = expand_node(opcode_node(_cpy, I, 1, constant_node(0x60, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1068, program_counter);
    result = expand_node(opcode_node(_cpy, zp, 1, constant_node(0xd4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x106a, program_counter);
    result = expand_node(opcode_node(_cpy, a, 1, constant_node(0x777d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x106d, program_counter);

    // _dec
    result = expand_node(opcode_node(_dec, zp, 1, constant_node(0x4e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x106f, program_counter);
    result = expand_node(opcode_node(_dec, zpx, 1, constant_node(0x4a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1071, program_counter);
    result = expand_node(opcode_node(_dec, a, 1, constant_node(0x3b25, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1074, program_counter);
    result = expand_node(opcode_node(_dec, ax, 1, constant_node(0x1f1f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1077, program_counter);

    // _dex
    result = expand_node(opcode_node(_dex, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1078, program_counter);

    // _dey
    result = expand_node(opcode_node(_dey, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1079, program_counter);

    // _eor
    result = expand_node(opcode_node(_eor, I, 1, constant_node(0xcb, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x107b, program_counter);
    result = expand_node(opcode_node(_eor, zp, 1, constant_node(0xee, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x107d, program_counter);
    result = expand_node(opcode_node(_eor, zpx, 1, constant_node(0x2f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x107f, program_counter);
    result = expand_node(opcode_node(_eor, a, 1, constant_node(0x6bfc, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1082, program_counter);
    result = expand_node(opcode_node(_eor, ax, 1, constant_node(0x7f96, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1085, program_counter);
    result = expand_node(opcode_node(_eor, ay, 1, constant_node(0x7ff5, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1088, program_counter);
    result = expand_node(opcode_node(_eor, izx, 1, constant_node(0x93, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x108a, program_counter);
    result = expand_node(opcode_node(_eor, izy, 1, constant_node(0x6d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x108c, program_counter);

    // _inc
    result = expand_node(opcode_node(_inc, zp, 1, constant_node(0x35, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x108e, program_counter);
    result = expand_node(opcode_node(_inc, zpx, 1, constant_node(0x33, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1090, program_counter);
    result = expand_node(opcode_node(_inc, a, 1, constant_node(0x6b89, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1093, program_counter);
    result = expand_node(opcode_node(_inc, ax, 1, constant_node(0x30a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1096, program_counter);

    // _inx
    result = expand_node(opcode_node(_inx, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1097, program_counter);

    // _iny
    result = expand_node(opcode_node(_iny, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1098, program_counter);

    // _jmp
    result = expand_node(opcode_node(_jmp, a, 1, constant_node(0x311c, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x109b, program_counter);
    result = expand_node(opcode_node(_jmp, ind, 1, constant_node(0xbdb, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x109e, program_counter);

    // _jsr
    result = expand_node(opcode_node(_jsr, a, 1, constant_node(0x57ae, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10a1, program_counter);

    // _lda
    result = expand_node(opcode_node(_lda, I, 1, constant_node(0x39, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10a3, program_counter);
    result = expand_node(opcode_node(_lda, zp, 1, constant_node(0x21, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10a5, program_counter);
    result = expand_node(opcode_node(_lda, zpx, 1, constant_node(0x10, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10a7, program_counter);
    result = expand_node(opcode_node(_lda, a, 1, constant_node(0x2350, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10aa, program_counter);
    result = expand_node(opcode_node(_lda, ax, 1, constant_node(0x23ee, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10ad, program_counter);
    result = expand_node(opcode_node(_lda, ay, 1, constant_node(0x4b40, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10b0, program_counter);
    result = expand_node(opcode_node(_lda, izx, 1, constant_node(0xd0, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10b2, program_counter);
    result = expand_node(opcode_node(_lda, izy, 1, constant_node(0xa1, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10b4, program_counter);

    // _ldx
    result = expand_node(opcode_node(_ldx, I, 1, constant_node(0x5a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10b6, program_counter);
    result = expand_node(opcode_node(_ldx, zp, 1, constant_node(0x50, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10b8, program_counter);
    result = expand_node(opcode_node(_ldx, zpy, 1, constant_node(0x63, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10ba, program_counter);
    result = expand_node(opcode_node(_ldx, a, 1, constant_node(0x5f32, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10bd, program_counter);
    result = expand_node(opcode_node(_ldx, ay, 1, constant_node(0x3bf6, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10c0, program_counter);

    // _ldy
    result = expand_node(opcode_node(_ldy, I, 1, constant_node(0xd8, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10c2, program_counter);
    result = expand_node(opcode_node(_ldy, zp, 1, constant_node(0xf6, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10c4, program_counter);
    result = expand_node(opcode_node(_ldy, zpx, 1, constant_node(0xa8, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10c6, program_counter);
    result = expand_node(opcode_node(_ldy, a, 1, constant_node(0xddc, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10c9, program_counter);
    result = expand_node(opcode_node(_ldy, ax, 1, constant_node(0x4dad, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10cc, program_counter);

    // _lsr
    result = expand_node(opcode_node(_lsr, A, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10cd, program_counter);
    result = expand_node(opcode_node(_lsr, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10ce, program_counter);
    result = expand_node(opcode_node(_lsr, zp, 1, constant_node(0x80, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10d0, program_counter);
    result = expand_node(opcode_node(_lsr, zpx, 1, constant_node(0x72, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10d2, program_counter);
    result = expand_node(opcode_node(_lsr, a, 1, constant_node(0x4df2, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10d5, program_counter);
    result = expand_node(opcode_node(_lsr, ax, 1, constant_node(0x4944, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10d8, program_counter);

    // _nop
    result = expand_node(opcode_node(_nop, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10d9, program_counter);

    // _ora
    result = expand_node(opcode_node(_ora, I, 1, constant_node(0x6e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10db, program_counter);
    result = expand_node(opcode_node(_ora, zp, 1, constant_node(0x79, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10dd, program_counter);
    result = expand_node(opcode_node(_ora, zpx, 1, constant_node(0xec, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10df, program_counter);
    result = expand_node(opcode_node(_ora, a, 1, constant_node(0x376b, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10e2, program_counter);
    result = expand_node(opcode_node(_ora, ax, 1, constant_node(0x67c4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10e5, program_counter);
    result = expand_node(opcode_node(_ora, ay, 1, constant_node(0x4330, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10e8, program_counter);
    result = expand_node(opcode_node(_ora, izx, 1, constant_node(0x36, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10ea, program_counter);
    result = expand_node(opcode_node(_ora, izy, 1, constant_node(0x92, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10ec, program_counter);

    // _pha
    result = expand_node(opcode_node(_pha, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10ed, program_counter);

    // _php
    result = expand_node(opcode_node(_php, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10ee, program_counter);

    // _pla
    result = expand_node(opcode_node(_pla, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10ef, program_counter);

    // _plp
    result = expand_node(opcode_node(_plp, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10f0, program_counter);

    // _rol
    result = expand_node(opcode_node(_rol, A, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10f1, program_counter);
    result = expand_node(opcode_node(_rol, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10f2, program_counter);
    result = expand_node(opcode_node(_rol, zp, 1, constant_node(0x67, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10f4, program_counter);
    result = expand_node(opcode_node(_rol, zpx, 1, constant_node(0xb6, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10f6, program_counter);
    result = expand_node(opcode_node(_rol, a, 1, constant_node(0x5522, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10f9, program_counter);
    result = expand_node(opcode_node(_rol, ax, 1, constant_node(0x3ff6, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10fc, program_counter);

    // _ror
    result = expand_node(opcode_node(_ror, A, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10fd, program_counter);
    result = expand_node(opcode_node(_ror, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x10fe, program_counter);
    result = expand_node(opcode_node(_ror, zp, 1, constant_node(0x2a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1100, program_counter);
    result = expand_node(opcode_node(_ror, zpx, 1, constant_node(0xea, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1102, program_counter);
    result = expand_node(opcode_node(_ror, a, 1, constant_node(0x419d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1105, program_counter);
    result = expand_node(opcode_node(_ror, ax, 1, constant_node(0x13e1, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1108, program_counter);

    // _rti
    result = expand_node(opcode_node(_rti, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1109, program_counter);

    // _rts
    result = expand_node(opcode_node(_rts, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x110a, program_counter);

    // _sbc
    result = expand_node(opcode_node(_sbc, I, 1, constant_node(0x5, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x110c, program_counter);
    result = expand_node(opcode_node(_sbc, zp, 1, constant_node(0x31, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x110e, program_counter);
    result = expand_node(opcode_node(_sbc, zpx, 1, constant_node(0x4e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1110, program_counter);
    result = expand_node(opcode_node(_sbc, a, 1, constant_node(0x59b0, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1113, program_counter);
    result = expand_node(opcode_node(_sbc, ax, 1, constant_node(0x27ca, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1116, program_counter);
    result = expand_node(opcode_node(_sbc, ay, 1, constant_node(0x3799, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1119, program_counter);
    result = expand_node(opcode_node(_sbc, izx, 1, constant_node(0xb, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x111b, program_counter);
    result = expand_node(opcode_node(_sbc, izy, 1, constant_node(0x35, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x111d, program_counter);

    // _sec
    result = expand_node(opcode_node(_sec, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x111e, program_counter);

    // _sed
    result = expand_node(opcode_node(_sed, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x111f, program_counter);

    // _sei
    result = expand_node(opcode_node(_sei, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1120, program_counter);

    // _sta
    result = expand_node(opcode_node(_sta, zp, 1, constant_node(0xc9, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1122, program_counter);
    result = expand_node(opcode_node(_sta, zpx, 1, constant_node(0xb0, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1124, program_counter);
    result = expand_node(opcode_node(_sta, a, 1, constant_node(0x7149, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1127, program_counter);
    result = expand_node(opcode_node(_sta, ax, 1, constant_node(0x692c, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x112a, program_counter);
    result = expand_node(opcode_node(_sta, ay, 1, constant_node(0x4b80, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x112d, program_counter);
    result = expand_node(opcode_node(_sta, izx, 1, constant_node(0x96, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x112f, program_counter);
    result = expand_node(opcode_node(_sta, izy, 1, constant_node(0xdb, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1131, program_counter);

    // _stx
    result = expand_node(opcode_node(_stx, zp, 1, constant_node(0x2, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1133, program_counter);
    result = expand_node(opcode_node(_stx, zpy, 1, constant_node(0x12, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1135, program_counter);
    result = expand_node(opcode_node(_stx, a, 1, constant_node(0x13e9, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1138, program_counter);

    // _sty
    result = expand_node(opcode_node(_sty, zp, 1, constant_node(0xc0, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x113a, program_counter);
    result = expand_node(opcode_node(_sty, zpx, 1, constant_node(0x10, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x113c, program_counter);
    result = expand_node(opcode_node(_sty, a, 1, constant_node(0x33ea, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x113f, program_counter);

    // _tax
    result = expand_node(opcode_node(_tax, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1140, program_counter);

    // _tya
    result = expand_node(opcode_node(_tya, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1141, program_counter);

    // _tsx
    result = expand_node(opcode_node(_tsx, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1142, program_counter);

    // _txa
    result = expand_node(opcode_node(_txa, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1143, program_counter);

    // _txs
    result = expand_node(opcode_node(_txs, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1144, program_counter);


    ///////////////////////////////////////////////////////////
    cpu_mode = cpu_6502;
    allow_illegal_op_codes = true;
    ///////////////////////////////////////////////////////////

    // _slo
    result = expand_node(opcode_node(_slo, zp, 1, constant_node(0xec, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1146, program_counter);
    result = expand_node(opcode_node(_slo, zpx, 1, constant_node(0x15, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1148, program_counter);
    result = expand_node(opcode_node(_slo, izx, 1, constant_node(0xaa, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x114a, program_counter);
    result = expand_node(opcode_node(_slo, izy, 1, constant_node(0x20, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x114c, program_counter);
    result = expand_node(opcode_node(_slo, a, 1, constant_node(0x5d67, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x114f, program_counter);
    result = expand_node(opcode_node(_slo, ax, 1, constant_node(0x3dd6, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1152, program_counter);
    result = expand_node(opcode_node(_slo, ay, 1, constant_node(0xfbf, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1155, program_counter);

    // _rla
    result = expand_node(opcode_node(_rla, zp, 1, constant_node(0x43, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1157, program_counter);
    result = expand_node(opcode_node(_rla, zpx, 1, constant_node(0x41, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1159, program_counter);
    result = expand_node(opcode_node(_rla, izx, 1, constant_node(0x82, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x115b, program_counter);
    result = expand_node(opcode_node(_rla, izy, 1, constant_node(0x6f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x115d, program_counter);
    result = expand_node(opcode_node(_rla, a, 1, constant_node(0x55dc, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1160, program_counter);
    result = expand_node(opcode_node(_rla, ax, 1, constant_node(0x378e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1163, program_counter);
    result = expand_node(opcode_node(_rla, ay, 1, constant_node(0xd66, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1166, program_counter);

    // _sre
    result = expand_node(opcode_node(_sre, zp, 1, constant_node(0xfc, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1168, program_counter);
    result = expand_node(opcode_node(_sre, zpx, 1, constant_node(0x65, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x116a, program_counter);
    result = expand_node(opcode_node(_sre, izx, 1, constant_node(0x9d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x116c, program_counter);
    result = expand_node(opcode_node(_sre, izy, 1, constant_node(0x75, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x116e, program_counter);
    result = expand_node(opcode_node(_sre, a, 1, constant_node(0x3d61, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1171, program_counter);
    result = expand_node(opcode_node(_sre, ax, 1, constant_node(0x2fff, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1174, program_counter);
    result = expand_node(opcode_node(_sre, ay, 1, constant_node(0x6d69, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1177, program_counter);

    // _rra
    result = expand_node(opcode_node(_rra, zp, 1, constant_node(0xb7, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1179, program_counter);
    result = expand_node(opcode_node(_rra, zpx, 1, constant_node(0x9b, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x117b, program_counter);
    result = expand_node(opcode_node(_rra, izx, 1, constant_node(0xef, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x117d, program_counter);
    result = expand_node(opcode_node(_rra, izy, 1, constant_node(0x4f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x117f, program_counter);
    result = expand_node(opcode_node(_rra, a, 1, constant_node(0x271e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1182, program_counter);
    result = expand_node(opcode_node(_rra, ax, 1, constant_node(0x5f9d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1185, program_counter);
    result = expand_node(opcode_node(_rra, ay, 1, constant_node(0x499c, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1188, program_counter);

    // _sax
    result = expand_node(opcode_node(_sax, zp, 1, constant_node(0x2f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x118a, program_counter);
    result = expand_node(opcode_node(_sax, zpy, 1, constant_node(0xd3, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x118c, program_counter);
    result = expand_node(opcode_node(_sax, izx, 1, constant_node(0xdd, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x118e, program_counter);
    result = expand_node(opcode_node(_sax, a, 1, constant_node(0x33e6, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1191, program_counter);

    // _lax
    result = expand_node(opcode_node(_lax, zp, 1, constant_node(0x5d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1193, program_counter);
    result = expand_node(opcode_node(_lax, I, 1, constant_node(0x62, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1195, program_counter);
    result = expand_node(opcode_node(_lax, zpy, 1, constant_node(0x87, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1197, program_counter);
    result = expand_node(opcode_node(_lax, izx, 1, constant_node(0xce, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1199, program_counter);
    result = expand_node(opcode_node(_lax, izy, 1, constant_node(0x93, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x119b, program_counter);
    result = expand_node(opcode_node(_lax, a, 1, constant_node(0x777, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x119e, program_counter);
    result = expand_node(opcode_node(_lax, ay, 1, constant_node(0x4502, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11a1, program_counter);

    // _dcp
    result = expand_node(opcode_node(_dcp, zp, 1, constant_node(0xef, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11a3, program_counter);
    result = expand_node(opcode_node(_dcp, zpx, 1, constant_node(0x54, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11a5, program_counter);
    result = expand_node(opcode_node(_dcp, izx, 1, constant_node(0x89, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11a7, program_counter);
    result = expand_node(opcode_node(_dcp, izy, 1, constant_node(0x80, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11a9, program_counter);
    result = expand_node(opcode_node(_dcp, a, 1, constant_node(0x1953, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11ac, program_counter);
    result = expand_node(opcode_node(_dcp, ax, 1, constant_node(0x6bcb, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11af, program_counter);
    result = expand_node(opcode_node(_dcp, ay, 1, constant_node(0xfc9, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11b2, program_counter);

    // _isc
    result = expand_node(opcode_node(_isc, zp, 1, constant_node(0x20, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11b4, program_counter);
    result = expand_node(opcode_node(_isc, zpx, 1, constant_node(0x7d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11b6, program_counter);
    result = expand_node(opcode_node(_isc, izx, 1, constant_node(0x5b, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11b8, program_counter);
    result = expand_node(opcode_node(_isc, izy, 1, constant_node(0xec, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11ba, program_counter);
    result = expand_node(opcode_node(_isc, a, 1, constant_node(0x259e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11bd, program_counter);
    result = expand_node(opcode_node(_isc, ax, 1, constant_node(0x2b0c, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11c0, program_counter);
    result = expand_node(opcode_node(_isc, ay, 1, constant_node(0x11f4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11c3, program_counter);

    // _anc
    result = expand_node(opcode_node(_anc, I, 1, constant_node(0x33, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11c5, program_counter);

    // _anc2
    result = expand_node(opcode_node(_anc2, I, 1, constant_node(0x3f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11c7, program_counter);

    // _alr
    result = expand_node(opcode_node(_alr, I, 1, constant_node(0xf9, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11c9, program_counter);

    // _arr
    result = expand_node(opcode_node(_arr, I, 1, constant_node(0x21, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11cb, program_counter);

    // _xaa
    result = expand_node(opcode_node(_xaa, I, 1, constant_node(0x4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11cd, program_counter);

    // _usbc
    result = expand_node(opcode_node(_usbc, I, 1, constant_node(0x79, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11cf, program_counter);

    // _ahx
    result = expand_node(opcode_node(_ahx, izy, 1, constant_node(0x90, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11d1, program_counter);
    result = expand_node(opcode_node(_ahx, ay, 1, constant_node(0x135, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11d4, program_counter);

    // _shy
    result = expand_node(opcode_node(_shy, ax, 1, constant_node(0x7cf, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11d7, program_counter);

    // _shx
    result = expand_node(opcode_node(_shx, ay, 1, constant_node(0x6732, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11da, program_counter);

    // _tas
    result = expand_node(opcode_node(_tas, ay, 1, constant_node(0x6d22, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11dd, program_counter);

    // _las
    result = expand_node(opcode_node(_las, ay, 1, constant_node(0x1bf4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11e0, program_counter);


    ///////////////////////////////////////////////////////////
    cpu_mode = cpu_65C02;
    allow_illegal_op_codes = false;
    ///////////////////////////////////////////////////////////

    // _bra
    result = expand_node(opcode_node(_bra, r, 1, constant_node(0x123d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11e2, program_counter);

    // _phx
    result = expand_node(opcode_node(_phx, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11e3, program_counter);

    // _phy
    result = expand_node(opcode_node(_phy, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11e4, program_counter);

    // _plx
    result = expand_node(opcode_node(_plx, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11e5, program_counter);

    // _ply
    result = expand_node(opcode_node(_ply, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11e6, program_counter);

    // _stz
    result = expand_node(opcode_node(_stz, zp, 1, constant_node(0x16, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11e8, program_counter);
    result = expand_node(opcode_node(_stz, zpx, 1, constant_node(0xd4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11ea, program_counter);
    result = expand_node(opcode_node(_stz, a, 1, constant_node(0xf90, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11ed, program_counter);
    result = expand_node(opcode_node(_stz, ax, 1, constant_node(0x3b2d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11f0, program_counter);

    // _trb
    result = expand_node(opcode_node(_trb, zp, 1, constant_node(0xa8, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11f2, program_counter);
    result = expand_node(opcode_node(_trb, a, 1, constant_node(0x57d3, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11f5, program_counter);

    // _tsb
    result = expand_node(opcode_node(_tsb, zp, 1, constant_node(0xd4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11f7, program_counter);
    result = expand_node(opcode_node(_tsb, a, 1, constant_node(0x975, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11fa, program_counter);

    // _bbr0
    result = expand_node(opcode_node(_bbr0, r, 1, constant_node(0x121a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11fc, program_counter);

    // _bbr1
    result = expand_node(opcode_node(_bbr1, r, 1, constant_node(0x1271, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x11fe, program_counter);

    // _bbr2
    result = expand_node(opcode_node(_bbr2, r, 1, constant_node(0x1276, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1200, program_counter);

    // _bbr3
    result = expand_node(opcode_node(_bbr3, r, 1, constant_node(0x1251, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1202, program_counter);

    // _bbr4
    result = expand_node(opcode_node(_bbr4, r, 1, constant_node(0x1221, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1204, program_counter);

    // _bbr5
    result = expand_node(opcode_node(_bbr5, r, 1, constant_node(0x1264, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1206, program_counter);

    // _bbr6
    result = expand_node(opcode_node(_bbr6, r, 1, constant_node(0x1249, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1208, program_counter);

    // _bbr7
    result = expand_node(opcode_node(_bbr7, r, 1, constant_node(0x1279, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x120a, program_counter);

    // _bbs0
    result = expand_node(opcode_node(_bbs0, r, 1, constant_node(0x1255, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x120c, program_counter);

    // _bbs1
    result = expand_node(opcode_node(_bbs1, r, 1, constant_node(0x1224, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x120e, program_counter);

    // _bbs2
    result = expand_node(opcode_node(_bbs2, r, 1, constant_node(0x1258, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1210, program_counter);

    // _bbs3
    result = expand_node(opcode_node(_bbs3, r, 1, constant_node(0x1252, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1212, program_counter);

    // _bbs4
    result = expand_node(opcode_node(_bbs4, r, 1, constant_node(0x127b, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1214, program_counter);

    // _bbs5
    result = expand_node(opcode_node(_bbs5, r, 1, constant_node(0x126c, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1216, program_counter);

    // _bbs6
    result = expand_node(opcode_node(_bbs6, r, 1, constant_node(0x1280, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1218, program_counter);

    // _bbs7
    result = expand_node(opcode_node(_bbs7, r, 1, constant_node(0x1245, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x121a, program_counter);

    // _rmb0
    result = expand_node(opcode_node(_rmb0, zp, 1, constant_node(0xea, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x121c, program_counter);

    // _rmb1
    result = expand_node(opcode_node(_rmb1, zp, 1, constant_node(0xe0, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x121e, program_counter);

    // _rmb2
    result = expand_node(opcode_node(_rmb2, zp, 1, constant_node(0xc7, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1220, program_counter);

    // _rmb3
    result = expand_node(opcode_node(_rmb3, zp, 1, constant_node(0x3e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1222, program_counter);

    // _rmb4
    result = expand_node(opcode_node(_rmb4, zp, 1, constant_node(0x87, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1224, program_counter);

    // _rmb5
    result = expand_node(opcode_node(_rmb5, zp, 1, constant_node(0x55, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1226, program_counter);

    // _rmb6
    result = expand_node(opcode_node(_rmb6, zp, 1, constant_node(0x21, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1228, program_counter);

    // _rmb7
    result = expand_node(opcode_node(_rmb7, zp, 1, constant_node(0x3f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x122a, program_counter);

    // _smb0
    result = expand_node(opcode_node(_smb0, zp, 1, constant_node(0xb2, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x122c, program_counter);

    // _smb1
    result = expand_node(opcode_node(_smb1, zp, 1, constant_node(0x39, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x122e, program_counter);

    // _smb2
    result = expand_node(opcode_node(_smb2, zp, 1, constant_node(0xf4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1230, program_counter);

    // _smb3
    result = expand_node(opcode_node(_smb3, zp, 1, constant_node(0x44, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1232, program_counter);

    // _smb4
    result = expand_node(opcode_node(_smb4, zp, 1, constant_node(0x35, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1234, program_counter);

    // _smb5
    result = expand_node(opcode_node(_smb5, zp, 1, constant_node(0xd2, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1236, program_counter);

    // _smb6
    result = expand_node(opcode_node(_smb6, zp, 1, constant_node(0xc6, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1238, program_counter);

    // _smb7
    result = expand_node(opcode_node(_smb7, zp, 1, constant_node(0xf3, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x123a, program_counter);

    // _wai
    result = expand_node(opcode_node(_wai, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x123b, program_counter);

    // _stp
    result = expand_node(opcode_node(_stp, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x123c, program_counter);

    // _adc
    result = expand_node(opcode_node(_adc, I, 1, constant_node(0xb4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x123e, program_counter);
    result = expand_node(opcode_node(_adc, zp, 1, constant_node(0xa1, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1240, program_counter);
    result = expand_node(opcode_node(_adc, zpx, 1, constant_node(0x8, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1242, program_counter);
    result = expand_node(opcode_node(_adc, a, 1, constant_node(0x3bb1, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1245, program_counter);
    result = expand_node(opcode_node(_adc, ax, 1, constant_node(0x4d85, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1248, program_counter);
    result = expand_node(opcode_node(_adc, ay, 1, constant_node(0x513e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x124b, program_counter);
    result = expand_node(opcode_node(_adc, izx, 1, constant_node(0xd6, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x124d, program_counter);
    result = expand_node(opcode_node(_adc, izy, 1, constant_node(0x7f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x124f, program_counter);
    result = expand_node(opcode_node(_adc, ind, 1, constant_node(0x4ff8, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1252, program_counter);

    // _and
    result = expand_node(opcode_node(_and, I, 1, constant_node(0xa2, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1254, program_counter);
    result = expand_node(opcode_node(_and, zp, 1, constant_node(0xb2, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1256, program_counter);
    result = expand_node(opcode_node(_and, zpx, 1, constant_node(0x34, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1258, program_counter);
    result = expand_node(opcode_node(_and, a, 1, constant_node(0x1796, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x125b, program_counter);
    result = expand_node(opcode_node(_and, ax, 1, constant_node(0x5f73, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x125e, program_counter);
    result = expand_node(opcode_node(_and, ay, 1, constant_node(0x470e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1261, program_counter);
    result = expand_node(opcode_node(_and, izx, 1, constant_node(0x4d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1263, program_counter);
    result = expand_node(opcode_node(_and, izy, 1, constant_node(0x35, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1265, program_counter);
    result = expand_node(opcode_node(_and, ind, 1, constant_node(0x192f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1268, program_counter);

    // _bit
    result = expand_node(opcode_node(_bit, I, 1, constant_node(0xb4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x126a, program_counter);
    result = expand_node(opcode_node(_bit, zp, 1, constant_node(0xc1, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x126c, program_counter);
    result = expand_node(opcode_node(_bit, zpx, 1, constant_node(0x1f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x126e, program_counter);
    result = expand_node(opcode_node(_bit, a, 1, constant_node(0x2df7, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1271, program_counter);
    result = expand_node(opcode_node(_bit, ax, 1, constant_node(0x3f4a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1274, program_counter);

    // _cmp
    result = expand_node(opcode_node(_cmp, I, 1, constant_node(0x54, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1276, program_counter);
    result = expand_node(opcode_node(_cmp, zp, 1, constant_node(0x2f, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1278, program_counter);
    result = expand_node(opcode_node(_cmp, zpx, 1, constant_node(0xa5, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x127a, program_counter);
    result = expand_node(opcode_node(_cmp, a, 1, constant_node(0x4f68, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x127d, program_counter);
    result = expand_node(opcode_node(_cmp, ax, 1, constant_node(0x5976, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1280, program_counter);
    result = expand_node(opcode_node(_cmp, ay, 1, constant_node(0x67fa, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1283, program_counter);
    result = expand_node(opcode_node(_cmp, izx, 1, constant_node(0x29, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1285, program_counter);
    result = expand_node(opcode_node(_cmp, izy, 1, constant_node(0x5, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1287, program_counter);
    result = expand_node(opcode_node(_cmp, ind, 1, constant_node(0x6f11, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x128a, program_counter);

    // _dec
    result = expand_node(opcode_node(_dec, A, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x128b, program_counter);
    result = expand_node(opcode_node(_dec, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x128c, program_counter);
    result = expand_node(opcode_node(_dec, zp, 1, constant_node(0x22, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x128e, program_counter);
    result = expand_node(opcode_node(_dec, zpx, 1, constant_node(0xfc, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1290, program_counter);
    result = expand_node(opcode_node(_dec, a, 1, constant_node(0x5d24, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1293, program_counter);
    result = expand_node(opcode_node(_dec, ax, 1, constant_node(0x588, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1296, program_counter);

    // _eor
    result = expand_node(opcode_node(_eor, I, 1, constant_node(0xce, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1298, program_counter);
    result = expand_node(opcode_node(_eor, zp, 1, constant_node(0x7b, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x129a, program_counter);
    result = expand_node(opcode_node(_eor, zpx, 1, constant_node(0x7a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x129c, program_counter);
    result = expand_node(opcode_node(_eor, a, 1, constant_node(0x49db, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x129f, program_counter);
    result = expand_node(opcode_node(_eor, ax, 1, constant_node(0x2725, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12a2, program_counter);
    result = expand_node(opcode_node(_eor, ay, 1, constant_node(0x1743, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12a5, program_counter);
    result = expand_node(opcode_node(_eor, izx, 1, constant_node(0xf2, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12a7, program_counter);
    result = expand_node(opcode_node(_eor, izy, 1, constant_node(0xab, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12a9, program_counter);
    result = expand_node(opcode_node(_eor, ind, 1, constant_node(0x6df4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12ac, program_counter);

    // _inc
    result = expand_node(opcode_node(_inc, A, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12ad, program_counter);
    result = expand_node(opcode_node(_inc, i, 0));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12ae, program_counter);
    result = expand_node(opcode_node(_inc, zp, 1, constant_node(0xa4, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12b0, program_counter);
    result = expand_node(opcode_node(_inc, zpx, 1, constant_node(0xe6, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12b2, program_counter);
    result = expand_node(opcode_node(_inc, a, 1, constant_node(0x29d8, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12b5, program_counter);
    result = expand_node(opcode_node(_inc, ax, 1, constant_node(0xb28, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12b8, program_counter);

    // _jmp
    result = expand_node(opcode_node(_jmp, a, 1, constant_node(0x9ce, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12bb, program_counter);
    result = expand_node(opcode_node(_jmp, ind, 1, constant_node(0x530b, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12be, program_counter);
    result = expand_node(opcode_node(_jmp, aix, 1, constant_node(0x69f5, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12c1, program_counter);

    // _lda
    result = expand_node(opcode_node(_lda, I, 1, constant_node(0xb, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12c3, program_counter);
    result = expand_node(opcode_node(_lda, zp, 1, constant_node(0x99, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12c5, program_counter);
    result = expand_node(opcode_node(_lda, zpx, 1, constant_node(0x8d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12c7, program_counter);
    result = expand_node(opcode_node(_lda, a, 1, constant_node(0x273d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12ca, program_counter);
    result = expand_node(opcode_node(_lda, ax, 1, constant_node(0x3b97, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12cd, program_counter);
    result = expand_node(opcode_node(_lda, ay, 1, constant_node(0x4127, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12d0, program_counter);
    result = expand_node(opcode_node(_lda, izx, 1, constant_node(0x9d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12d2, program_counter);
    result = expand_node(opcode_node(_lda, izy, 1, constant_node(0x82, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12d4, program_counter);
    result = expand_node(opcode_node(_lda, ind, 1, constant_node(0x5f76, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12d7, program_counter);

    // _ora
    result = expand_node(opcode_node(_ora, I, 1, constant_node(0x55, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12d9, program_counter);
    result = expand_node(opcode_node(_ora, zp, 1, constant_node(0x3a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12db, program_counter);
    result = expand_node(opcode_node(_ora, zpx, 1, constant_node(0x3d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12dd, program_counter);
    result = expand_node(opcode_node(_ora, a, 1, constant_node(0x6fc9, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12e0, program_counter);
    result = expand_node(opcode_node(_ora, ax, 1, constant_node(0x5dcd, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12e3, program_counter);
    result = expand_node(opcode_node(_ora, ay, 1, constant_node(0x2768, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12e6, program_counter);
    result = expand_node(opcode_node(_ora, izx, 1, constant_node(0x4d, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12e8, program_counter);
    result = expand_node(opcode_node(_ora, izy, 1, constant_node(0x59, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12ea, program_counter);
    result = expand_node(opcode_node(_ora, ind, 1, constant_node(0x96a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12ed, program_counter);

    // _sbc
    result = expand_node(opcode_node(_sbc, I, 1, constant_node(0xdd, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12ef, program_counter);
    result = expand_node(opcode_node(_sbc, zp, 1, constant_node(0x68, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12f1, program_counter);
    result = expand_node(opcode_node(_sbc, zpx, 1, constant_node(0x56, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12f3, program_counter);
    result = expand_node(opcode_node(_sbc, a, 1, constant_node(0x7b61, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12f6, program_counter);
    result = expand_node(opcode_node(_sbc, ax, 1, constant_node(0x940, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12f9, program_counter);
    result = expand_node(opcode_node(_sbc, ay, 1, constant_node(0x7114, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12fc, program_counter);
    result = expand_node(opcode_node(_sbc, izx, 1, constant_node(0x5, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x12fe, program_counter);
    result = expand_node(opcode_node(_sbc, izy, 1, constant_node(0x64, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1300, program_counter);
    result = expand_node(opcode_node(_sbc, ind, 1, constant_node(0xd6a, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1303, program_counter);

    // _sta
    result = expand_node(opcode_node(_sta, zp, 1, constant_node(0xe5, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1305, program_counter);
    result = expand_node(opcode_node(_sta, zpx, 1, constant_node(0x2e, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1307, program_counter);
    result = expand_node(opcode_node(_sta, a, 1, constant_node(0x2528, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x130a, program_counter);
    result = expand_node(opcode_node(_sta, ax, 1, constant_node(0x75c1, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x130d, program_counter);
    result = expand_node(opcode_node(_sta, ay, 1, constant_node(0x478c, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1310, program_counter);
    result = expand_node(opcode_node(_sta, izx, 1, constant_node(0x2b, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1312, program_counter);
    result = expand_node(opcode_node(_sta, izy, 1, constant_node(0xb7, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1314, program_counter);
    result = expand_node(opcode_node(_sta, ind, 1, constant_node(0x3f0b, false)));
    EXPECT_EQ(0, result);
    EXPECT_EQ(0x1317, program_counter);

    unsigned char expected[] =
    {

        0x69, 0x29, 0x65, 0x6b, 0x75, 0xd6, 0x6d, 0x84, 0x67, 0x7d,     // 0x1000
        0xe1, 0x4b, 0x79, 0x6c, 0x3d, 0x61, 0x03, 0x71, 0x21, 0x29,     // 0x100a
        0xbb, 0x25, 0xef, 0x35, 0x5f, 0x2d, 0xf1, 0x6d, 0x3d, 0xf1,     // 0x1014
        0x5b, 0x39, 0xbb, 0x41, 0x21, 0x10, 0x31, 0xec, 0x0a, 0x0a,     // 0x101e
        0x06, 0xbe, 0x16, 0xd4, 0x0e, 0xdb, 0x13, 0x1e, 0x3c, 0x15,     // 0x1028
        0x90, 0x06, 0xb0, 0x45, 0xf0, 0x4d, 0x24, 0x99, 0x2c, 0x24,     // 0x1032
        0x01, 0x30, 0x0f, 0xd0, 0x51, 0x10, 0x65, 0x00, 0x50, 0x53,     // 0x103c
        0x70, 0x05, 0x18, 0xd8, 0x58, 0xb8, 0xc9, 0x5c, 0xc5, 0x33,     // 0x1046
        0xd5, 0xec, 0xcd, 0x12, 0x2d, 0xdd, 0x4d, 0x07, 0xd9, 0xc8,     // 0x1050
        0x4d, 0xc1, 0xa7, 0xd1, 0x22, 0xe0, 0xcd, 0xe4, 0xcc, 0xec,     // 0x105a
        0x1f, 0x71, 0xc0, 0x60, 0xc4, 0xd4, 0xcc, 0x7d, 0x77, 0xc6,     // 0x1064
        0x4e, 0xd6, 0x4a, 0xce, 0x25, 0x3b, 0xde, 0x1f, 0x1f, 0xca,     // 0x106e
        0x88, 0x49, 0xcb, 0x45, 0xee, 0x55, 0x2f, 0x4d, 0xfc, 0x6b,     // 0x1078
        0x5d, 0x96, 0x7f, 0x59, 0xf5, 0x7f, 0x41, 0x93, 0x51, 0x6d,     // 0x1082
        0xe6, 0x35, 0xf6, 0x33, 0xee, 0x89, 0x6b, 0xfe, 0x0a, 0x03,     // 0x108c
        0xe8, 0xc8, 0x4c, 0x1c, 0x31, 0x6c, 0xdb, 0x0b, 0x20, 0xae,     // 0x1096
        0x57, 0xa9, 0x39, 0xa5, 0x21, 0xb5, 0x10, 0xad, 0x50, 0x23,     // 0x10a0
        0xbd, 0xee, 0x23, 0xb9, 0x40, 0x4b, 0xa1, 0xd0, 0xb1, 0xa1,     // 0x10aa
        0xa2, 0x5a, 0xa6, 0x50, 0xb6, 0x63, 0xae, 0x32, 0x5f, 0xbe,     // 0x10b4
        0xf6, 0x3b, 0xa0, 0xd8, 0xa4, 0xf6, 0xb4, 0xa8, 0xac, 0xdc,     // 0x10be
        0x0d, 0xbc, 0xad, 0x4d, 0x4a, 0x4a, 0x46, 0x80, 0x56, 0x72,     // 0x10c8
        0x4e, 0xf2, 0x4d, 0x5e, 0x44, 0x49, 0xea, 0x09, 0x6e, 0x05,     // 0x10d2
        0x79, 0x15, 0xec, 0x0d, 0x6b, 0x37, 0x1d, 0xc4, 0x67, 0x19,     // 0x10dc
        0x30, 0x43, 0x01, 0x36, 0x11, 0x92, 0x48, 0x08, 0x68, 0x28,     // 0x10e6
        0x2a, 0x2a, 0x26, 0x67, 0x36, 0xb6, 0x2e, 0x22, 0x55, 0x3e,     // 0x10f0
        0xf6, 0x3f, 0x6a, 0x6a, 0x66, 0x2a, 0x76, 0xea, 0x6e, 0x9d,     // 0x10fa
        0x41, 0x7e, 0xe1, 0x13, 0x40, 0x60, 0xe9, 0x05, 0xe5, 0x31,     // 0x1104
        0xf5, 0x4e, 0xed, 0xb0, 0x59, 0xfd, 0xca, 0x27, 0xf9, 0x99,     // 0x110e
        0x37, 0xe1, 0x0b, 0xf1, 0x35, 0x38, 0xf8, 0x78, 0x85, 0xc9,     // 0x1118
        0x95, 0xb0, 0x8d, 0x49, 0x71, 0x9d, 0x2c, 0x69, 0x99, 0x80,     // 0x1122
        0x4b, 0x81, 0x96, 0x91, 0xdb, 0x86, 0x02, 0x96, 0x12, 0x8e,     // 0x112c
        0xe9, 0x13, 0x84, 0xc0, 0x94, 0x10, 0x8c, 0xea, 0x33, 0xaa,     // 0x1136
        0x98, 0xba, 0x8a, 0x9a, 0x07, 0xec, 0x17, 0x15, 0x03, 0xaa,     // 0x1140
        0x13, 0x20, 0x0f, 0x67, 0x5d, 0x1f, 0xd6, 0x3d, 0x1b, 0xbf,     // 0x114a
        0x0f, 0x27, 0x43, 0x37, 0x41, 0x23, 0x82, 0x33, 0x6f, 0x2f,     // 0x1154
        0xdc, 0x55, 0x3f, 0x8e, 0x37, 0x3b, 0x66, 0x0d, 0x47, 0xfc,     // 0x115e
        0x57, 0x65, 0x43, 0x9d, 0x53, 0x75, 0x4f, 0x61, 0x3d, 0x5f,     // 0x1168
        0xff, 0x2f, 0x5b, 0x69, 0x6d, 0x67, 0xb7, 0x77, 0x9b, 0x63,     // 0x1172
        0xef, 0x73, 0x4f, 0x6f, 0x1e, 0x27, 0x7f, 0x9d, 0x5f, 0x7b,     // 0x117c
        0x9c, 0x49, 0x87, 0x2f, 0x97, 0xd3, 0x83, 0xdd, 0x8f, 0xe6,     // 0x1186
        0x33, 0xa7, 0x5d, 0xab, 0x62, 0xb7, 0x87, 0xa3, 0xce, 0xb3,     // 0x1190
        0x93, 0xaf, 0x77, 0x07, 0xbf, 0x02, 0x45, 0xc7, 0xef, 0xd7,     // 0x119a
        0x54, 0xc3, 0x89, 0xd3, 0x80, 0xcf, 0x53, 0x19, 0xdf, 0xcb,     // 0x11a4
        0x6b, 0xdb, 0xc9, 0x0f, 0xe7, 0x20, 0xf7, 0x7d, 0xe3, 0x5b,     // 0x11ae
        0xf3, 0xec, 0xef, 0x9e, 0x25, 0xff, 0x0c, 0x2b, 0xfb, 0xf4,     // 0x11b8
        0x11, 0x0b, 0x33, 0x2b, 0x3f, 0x4b, 0xf9, 0x6b, 0x21, 0x8b,     // 0x11c2
        0x04, 0xeb, 0x79, 0x93, 0x90, 0x9f, 0x35, 0x01, 0x9c, 0xcf,     // 0x11cc
        0x07, 0x9e, 0x32, 0x67, 0x9b, 0x22, 0x6d, 0xbb, 0xf4, 0x1b,     // 0x11d6
        0x80, 0x5b, 0xda, 0x5a, 0xfa, 0x7a, 0x64, 0x16, 0x74, 0xd4,     // 0x11e0
        0x9c, 0x90, 0x0f, 0x9e, 0x2d, 0x3b, 0x14, 0xa8, 0x1c, 0xd3,     // 0x11ea
        0x57, 0x04, 0xd4, 0x0c, 0x75, 0x09, 0x0f, 0x1e, 0x1f, 0x73,     // 0x11f4
        0x2f, 0x76, 0x3f, 0x4f, 0x4f, 0x1d, 0x5f, 0x5e, 0x6f, 0x41,     // 0x11fe
        0x7f, 0x6f, 0x8f, 0x49, 0x9f, 0x16, 0xaf, 0x48, 0xbf, 0x40,     // 0x1208
        0xcf, 0x67, 0xdf, 0x56, 0xef, 0x68, 0xff, 0x2b, 0x07, 0xea,     // 0x1212
        0x17, 0xe0, 0x27, 0xc7, 0x37, 0x3e, 0x47, 0x87, 0x57, 0x55,     // 0x121c
        0x67, 0x21, 0x77, 0x3f, 0x87, 0xb2, 0x97, 0x39, 0xa7, 0xf4,     // 0x1226
        0xb7, 0x44, 0xc7, 0x35, 0xd7, 0xd2, 0xe7, 0xc6, 0xf7, 0xf3,     // 0x1230
        0xcb, 0xdb, 0x69, 0xb4, 0x65, 0xa1, 0x75, 0x08, 0x6d, 0xb1,     // 0x123a
        0x3b, 0x7d, 0x85, 0x4d, 0x79, 0x3e, 0x51, 0x61, 0xd6, 0x71,     // 0x1244
        0x7f, 0x72, 0xf8, 0x4f, 0x29, 0xa2, 0x25, 0xb2, 0x35, 0x34,     // 0x124e
        0x2d, 0x96, 0x17, 0x3d, 0x73, 0x5f, 0x39, 0x0e, 0x47, 0x21,     // 0x1258
        0x4d, 0x31, 0x35, 0x32, 0x2f, 0x19, 0x89, 0xb4, 0x24, 0xc1,     // 0x1262
        0x34, 0x1f, 0x2c, 0xf7, 0x2d, 0x3c, 0x4a, 0x3f, 0xc9, 0x54,     // 0x126c
        0xc5, 0x2f, 0xd5, 0xa5, 0xcd, 0x68, 0x4f, 0xdd, 0x76, 0x59,     // 0x1276
        0xd9, 0xfa, 0x67, 0xc1, 0x29, 0xd1, 0x05, 0xd2, 0x11, 0x6f,     // 0x1280
        0x3a, 0x3a, 0xc6, 0x22, 0xd6, 0xfc, 0xce, 0x24, 0x5d, 0xde,     // 0x128a
        0x88, 0x05, 0x49, 0xce, 0x45, 0x7b, 0x55, 0x7a, 0x4d, 0xdb,     // 0x1294
        0x49, 0x5d, 0x25, 0x27, 0x59, 0x43, 0x17, 0x41, 0xf2, 0x51,     // 0x129e
        0xab, 0x52, 0xf4, 0x6d, 0x1a, 0x1a, 0xe6, 0xa4, 0xf6, 0xe6,     // 0x12a8
        0xee, 0xd8, 0x29, 0xfe, 0x28, 0x0b, 0x4c, 0xce, 0x09, 0x6c,     // 0x12b2
        0x0b, 0x53, 0x7c, 0xf5, 0x69, 0xa9, 0x0b, 0xa5, 0x99, 0xb5,     // 0x12bc
        0x8d, 0xad, 0x3d, 0x27, 0xbd, 0x97, 0x3b, 0xb9, 0x27, 0x41,     // 0x12c6
        0xa1, 0x9d, 0xb1, 0x82, 0xb2, 0x76, 0x5f, 0x09, 0x55, 0x05,     // 0x12d0
        0x3a, 0x15, 0x3d, 0x0d, 0xc9, 0x6f, 0x1d, 0xcd, 0x5d, 0x19,     // 0x12da
        0x68, 0x27, 0x01, 0x4d, 0x11, 0x59, 0x12, 0x6a, 0x09, 0xe9,     // 0x12e4
        0xdd, 0xe5, 0x68, 0xf5, 0x56, 0xed, 0x61, 0x7b, 0xfd, 0x40,     // 0x12ee
        0x09, 0xf9, 0x14, 0x71, 0xe1, 0x05, 0xf1, 0x64, 0xf2, 0x6a,     // 0x12f8
        0x0d, 0x85, 0xe5, 0x95, 0x2e, 0x8d, 0x28, 0x25, 0x9d, 0xc1,     // 0x1302
        0x75, 0x99, 0x8c, 0x47, 0x81, 0x2b, 0x91, 0xb7, 0x92, 0x0b,     // 0x130c
        0x3f,
    };

    unsigned char buffer[_countof(expected) + 1] = { 0 };
    fclose(output_file);


    output_file = open_file(output_file_name, "rb");
    fseek(output_file, 0, SEEK_SET);
    fread(buffer, 1, _countof(expected), output_file);
    fclose(output_file);

    output_file = nullptr;
    remove(output_file_name);

    for (auto i = 0; i < static_cast<int>((_countof(expected))); ++i)
    {
        if (expected[i] != buffer[i])
        {
            EXPECT_EQ(expected[i], buffer[i]);
        }
    }
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_and_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xad, WORD(0x0400)
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        operator_node(VAR, 1, 
                operator_node(EXPRLIST,  1, id_node("expand_operator_equal_node_1"))),
        operator_node(VAR, 1, 
                operator_node(EXPRLIST, 1, id_node("expand_operator_equal_node_2"))),

        operator_node('=', 2, 
            id_node("expand_operator_equal_node_1"),
            constant_node(50, false)),

        operator_node('=', 2,
            id_node("expand_operator_equal_node_2"),
            constant_node(25, false)),

        operator_node(IF, 3,
            operator_node(AND, 2,
                operator_node(EQ, 2,
                    id_node("expand_operator_equal_node_1"),
                    constant_node(50, -1)),
                operator_node(EQ, 2,
                    id_node("expand_operator_equal_node_2"),
                    constant_node(25, -1))),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));

    constexpr unsigned char expected_2[] =
    {
        0xad, WORD(0x0800)
    };

    const parse_node_ptr nodes_2[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        operator_node(VAR, 1, 
                operator_node(EXPRLIST, 1, id_node("expand_operator_equal_node_3"))),
        operator_node(VAR, 1, 
                operator_node(EXPRLIST, 1, id_node("expand_operator_equal_node_4"))),

        operator_node('=', 2,
            id_node("expand_operator_equal_node_3"),
            constant_node(50, false)),

        operator_node('=', 2,
            id_node("expand_operator_equal_node_4"),
            constant_node(26, false)),

        operator_node(IF, 3,
            operator_node(AND, 2,
                operator_node(EQ, 2,
                    id_node("expand_operator_equal_node_3"),
                    constant_node(50, -1)),
                operator_node(EQ, 2,
                    id_node("expand_operator_equal_node_4"),
                    constant_node(25, -1))),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))
    };

    expand_file_output_template(expected_2, _countof(expected_2), nodes_2, _countof(nodes_2));
    
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_bit_and_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x1155 & 0x5446),
        WORD(0x1234 & 0x5678)
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        data_node(data_word, operator_node(EXPRLIST, 1,
            operator_node(BIT_AND, 2,
                constant_node(0x1155, false),
                constant_node(0x5446, false)))),

        data_node(data_word, operator_node(EXPRLIST, 1,
            operator_node(BIT_AND, 2,
                constant_node(0x1234, false),
                constant_node(0x5678, false)))),

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_bit_or_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x1155 | 0x5446),
        WORD(0x1234 | 0x5678),
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        data_node(data_word, operator_node(EXPRLIST, 1,
            operator_node(BIT_OR, 2,
                constant_node(0x1155, false),
                constant_node(0x5446, false)))),


        data_node(data_word, operator_node(EXPRLIST, 1,
            operator_node(BIT_OR, 2,
                constant_node(0x1234, false),
                constant_node(0x5678, false)))),
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_divide_node_test)
{
    expand_unit_test_method_initialize();


    constexpr unsigned char expected[] =
    {
        WORD(300 / 25),
        WORD(0x7654 / 0x12),
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        data_node(data_word, operator_node(EXPRLIST, 1,
            operator_node('/', 2,
                constant_node(300, false),
                constant_node(25, false)))),


        data_node(data_word, operator_node(EXPRLIST, 1,
            operator_node('/', 2,
                constant_node(0x7654, false),
                constant_node(0x12, false)))),
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_do_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0x69, 0x00,
        0x69, 0x10,
        0x69, 0x20,
        0x69, 0x30,
        0x69, 0x40,
        0x69, 0x50,
        0x69, 0x60,
        0x69, 0x70,
        0x69, 0x80,
        0x69, 0x90,
        0x69, 0xa0,
        0x69, 0xb0,
        0x69, 0xc0,
        0x69, 0xd0,
        0x69, 0xe0,
        0x69, 0xf0,
    };

    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),
        operator_node(VAR, 1,
                operator_node(EXPRLIST, 1, id_node("my_var"))),
        operator_node(DO, 2,
            operator_node(STATEMENT, 2,
                opcode_node(_adc, I, 1, id_node("my_var")),
                operator_node(STATEMENT, 1,
                    operator_node('=', 2,
                        id_node("my_var"),
                        operator_node('+', 2,
                            id_node("my_var"),
                            constant_node(0x10, 0))))),

            operator_node('<', 2,
                id_node("my_var"), constant_node(0xFF, 0)))
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_ds_node_test)
{
    expand_unit_test_method_initialize();
    const auto* output_file_name = "expand_operator_ds_node_test.bin";
    current_file_name = (char*)"test.a";

    output_file = open_file(output_file_name, "wb");

    expand_node(operator_node(ORG, 1, constant_node(0x1000, false)));
    const int result = expand_node(operator_node(DS, 1,
            constant_node(22, 0)));

    fclose(output_file);
    EXPECT_EQ(22, result);

    output_file = open_file(output_file_name, "rb");
    fseek(output_file, 0, SEEK_END);
    const size_t pos = ftell(output_file);
    fclose(output_file);
    output_file = nullptr;
    remove(output_file_name);
    EXPECT_EQ(static_cast<size_t>(22), pos);
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_end_node_test)
{
    expand_unit_test_method_initialize();
    const auto* output_file_name = "expand_operator_end_node_test.bin";
    current_file_name = (char*)"test.a";
    output_file = open_file(output_file_name, "wb");


    end_expansion = 0;
    expand_node(operator_node(END, 0));

    fclose(output_file);
    output_file = nullptr;
    remove(output_file_name);

    EXPECT_NE(0, end_expansion);

    end_expansion = 0;
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_end_section_node_test)
{
    expand_unit_test_method_initialize();
    const auto* output_file_name = "expand_operator_end_node_test.bin";
    current_file_name = (char*)"test.a";
    output_file = open_file(output_file_name, "wb");


    expand_node(
        operator_node(ORG, 1, 
            constant_node(0x1000, false)));

    expand_node(
        operator_node('=', 2, 
            label_node((char*)"expand_operator_global"), 
            constant_node(program_counter, -1)));

    expand_node(
        opcode_node(_nop, i, 0));

    expand_node(
        operator_node(SECTION, 1, id_node("Section_One")));

    expand_node(
        opcode_node(_nop, i, 0));

    expand_node(
        operator_node('=', 2,
            label_node((char*)"expand_operator_inside_section"),
            constant_node(program_counter, -1)));

    expand_node(
        operator_node(ENDSECTION, 0));

    expand_node(
        operator_node('=', 2,
            label_node((char*)"expand_operator_global_2"),
            constant_node(program_counter, -1)));

    fclose(output_file);
    output_file = nullptr;
    remove(output_file_name);


    auto symbol_ptr = static_cast<symbol_table_ptr>(dict_search(symbol_dictionary, "expand_operator_inside_section"));
    EXPECT_NULL(symbol_ptr);

    symbol_ptr = static_cast<symbol_table_ptr>(dict_search(symbol_dictionary, "Section_One.expand_operator_inside_section"));
    EXPECT_NOT_NULL(symbol_ptr);

    symbol_ptr = static_cast<symbol_table_ptr>(dict_search(symbol_dictionary, "expand_operator_global"));
    EXPECT_NOT_NULL(symbol_ptr);

    symbol_ptr = static_cast<symbol_table_ptr>(dict_search(symbol_dictionary, "expand_operator_global_2"));
    EXPECT_NOT_NULL(symbol_ptr);
    
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_equ_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x1000)
    };

    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, 
            constant_node(0x1000, false)),

        operator_node(EQU, 2,
            id_node("expand_operator_equ_node"),
            constant_node(program_counter, -1)),

        data_node(data_word,
            operator_node(EXPRLIST, 1, id_node("expand_operator_equ_node")))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_equal_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xad, WORD(0x0400)
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node('=', 2,
            label_node((char*)"expand_operator_equal_node"),
            constant_node(program_counter, -1)),

        operator_node(IF, 3,
            operator_node(EQ, 2,
                id_node("expand_operator_equal_node"),
                constant_node(program_counter, -1)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))

    };
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_expression_list_node_test)
{
    expand_unit_test_method_initialize();
    unsigned char expected[] =
    {
        "\x20\x30" "\x50" "expand_operator_expression_list_node"
    };

    const parse_node_ptr nodes[] =
    {
        data_node(data_word,
            operator_node(EXPRLIST, 1, constant_node(0x3020, 0))),

        data_node(data_byte,
            operator_node(EXPRLIST, 1, constant_node(0x50, 0))),

        data_node(data_string,
            operator_node(EXPRLIST, 1, string_node("expand_operator_expression_list_node")))
    };

    expand_file_output_template(expected, _countof(expected) - 1, nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_string_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        "expand_string_node_test"
    };

    const parse_node_ptr nodes[] =
    {
        data_node(data_string, operator_node(EXPRLIST, 1, string_node("expand_string_node_test")))
    };

    expand_file_output_template(expected, _countof(expected) - 1, nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_fill_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0x21, 0x21, 0x21, 0x21, 0x21,
        0x12, 0x12, 0x12, 0x12, 0x12,
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node(FILL, 2, 
            constant_node(0x21, false), 
            constant_node(0x05, false)),
        operator_node(FILL, 2, 
            constant_node(0x12, false), 
            constant_node(0x05, false)),
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_for_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
       1, 11, 21, 31, 41, 51, 61, 71, 81, 91
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node(FOR,  5,                             
            operator_node('=', 2,
                            id_node("for_var"),
                constant_node(1, false)),
            constant_node(100, false),
            data_node(data_byte,
                operator_node(EXPRLIST, 1, id_node("for_var"))),
                    id_node("for_var"),
            constant_node(10, false))
    };
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_for_reg_node_test)
{
    expand_unit_test_method_initialize();
  
    constexpr unsigned char expected[] =
    {
        0xa2, 0x01, 0xea, 0xe8, 0xe0, 0x46, 0x90, 0xfa
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),
            
       operator_node(REGX, 4,
            constant_node(0x01, false),
            constant_node(0x45, false),
            opcode_node(_nop, i, 0),
            constant_node(1, false)
        ),
        constant_node(0x00, false),
    };
    
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));

    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_greater_than_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xad, WORD(0x0400),
        0xad, WORD(0x0800)
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node('=', 2,
            label_node((char*)"expand_operator_greater_than_node"),
            constant_node(program_counter, -1)),

        operator_node(IF, 3,
            operator_node('>', 2,
                id_node("expand_operator_greater_than_node"),
                constant_node(0x0300, 0)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false))),

        operator_node(IF, 3,
            operator_node('>', 2,
                id_node("expand_operator_greater_than_node"),
                constant_node(program_counter + 100, -1)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))

    };
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_greater_than_or_equal_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xad, WORD(0x0400),
        0xad, WORD(0x0800)
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node('=', 2,
            label_node((char*)"expand_operator_greater_than_or_equal_node"),
            constant_node(program_counter, -1)),

        operator_node(IF, 3,
            operator_node(GE, 2,
                id_node("expand_operator_greater_than_or_equal_node"),
                constant_node(program_counter, -1)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false))),

        operator_node(IF, 3,
            operator_node(GE, 2,
                id_node("expand_operator_greater_than_node"),
                constant_node(program_counter + 100, -1)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))

    };
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_hi_byte_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0x11
    };
    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        data_node(data_byte, operator_node(EXPRLIST, 1,
            operator_node(HIBYTE, 1, 
                constant_node(0x1122, false))))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_if_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xad, WORD(0x0400),
        0xad, WORD(0x0800)
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node('=', 2,
            label_node((char*)"expand_operator_if_node"),
            constant_node(program_counter, -1)),

        operator_node(IF, 3,
            operator_node(GE, 2,
                id_node("expand_operator_if_node"),
                constant_node(program_counter, -1)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false))),

        operator_node(IF, 3,
            operator_node(GE, 2,
                id_node("expand_operator_if_node"),
                constant_node(program_counter + 100, -1)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))

    };
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_include_node_test)
{
    expand_unit_test_method_initialize();
    FILE* temp_file = open_file("test.a", "w");
    EXPECT_NOT_NULL(temp_file);
    const auto text = "\tNOP\n\n";
    fwrite(text , 1, strlen(text), temp_file);
    fclose(temp_file);

    const auto output_file_name = "expand_file_output_template.bin";
    current_file_name = (char*)"testinc.a";
    output_file = open_file(output_file_name, "wb");

    yylineno = 0;
    current_file_name = (char*)"dummy.a";
    expand_node(operator_node(INC, 1, string_node("test.a")));

    fclose(output_file);
    remove(output_file_name);
    remove("text.a");
    output_file = nullptr;

    // fclose(yyin);
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_less_than_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xad, WORD(0x0800),
        0xad, WORD(0x0400)
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node('=', 2,
            label_node((char*)"expand_operator_less_than_node"),
            constant_node(program_counter, -1)),

        operator_node(IF, 3,
            operator_node('<', 2,
                id_node("expand_operator_less_than_node"),
                constant_node(0x0300, 0)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false))),

        operator_node(IF, 3,
            operator_node('<', 2,
                id_node("expand_operator_less_than_node"),
                constant_node(program_counter + 100, -1)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))

    };
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_less_than_or_equal_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xad, WORD(0x0400),
        0xad, WORD(0x0800)
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node('=', 2,
            label_node((char*)"expand_operator_less_than_or_equal_node"),
            constant_node(program_counter, -1)),

        operator_node(IF, 3,
            operator_node(LE, 2,
                id_node("expand_operator_less_than_or_equal_node"),
                constant_node(program_counter, -1)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false))),

        operator_node(IF, 3,
            operator_node(LE, 2,
                id_node("expand_operator_less_than_or_equal_node"),
                constant_node(program_counter + 100, -1)),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))

    };
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_lo_byte_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0x22
    };
    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        data_node(data_byte, operator_node(EXPRLIST, 1,
            operator_node(LOBYTE, 1,
                constant_node(0x1122, false))))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_load_node_test)
{
    expand_unit_test_method_initialize();
    FILE* temp_file = open_file("loadtest.bin", "wb");
    EXPECT_NOT_NULL(temp_file);
    const unsigned char text[] =
    {
        0x01, 0x02, 0x03, 0x04, 0x05,
        0x06, 0x07, 0x08, 0x09, 0x0a
    };

    fwrite(text, 1, _countof(text), temp_file);

    fclose(temp_file);

    constexpr unsigned char expected[] =
    {
        0x01, 0x02, 0x03, 0x04, 0x05,
        0x06, 0x07, 0x08, 0x09, 0x0a
    };

    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node(LOAD, 1, string_node("loadtest.bin"))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));

    remove("loadtest.bin");
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_macro_definition_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0x69, 0x29, 0xea
    };

    parse_node_ptr nodes[] =
    {
        operator_node(MACRO, 2,
            macro_id_node("macro_name"),
            operator_node(STATEMENT, 2,
                opcode_node(_adc, I, 1,
                    constant_node(0x29, false)),
                opcode_node(_nop, i, 0))),

        operator_node(ORG, 1, constant_node(0x1000, false)),

        macro_expand_node("macro_name", nullptr)
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_minus_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xa2, 0xff,
        0xca,
        0xd0, 0xfd
    };

    yylineno = 3;
    current_file_name = (char*)"test.a";
    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        opcode_node(_ldx, I, 1,
            constant_node(0xFF, 0)),

        operator_node('=', 2,
            id_node("-"),
            constant_node(program_counter, true)),

        opcode_node(_dex, i, 0),

        opcode_node(_bne, r, 1,
            label_node((char*)"-"))
    };
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_multiply_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD((0x2356 * 2))
    };
    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        data_node(data_word, 
        operator_node(EXPRLIST, 1,
            operator_node('*', 2,
                constant_node(0x2356, false),
                constant_node(0x02, false))))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x1122)
    };
    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        data_node(data_word, operator_node(EXPRLIST, 1,
                constant_node(0x1122, false)))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_not_equal_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xad, WORD(0x0800)
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        operator_node(VAR, 1, 
                operator_node(EXPRLIST, 1, id_node("expand_operator_not_equal_node_1"))),
        operator_node(VAR, 1, 
                operator_node(EXPRLIST, 1, id_node("expand_operator_not_equal_node_2"))),

        operator_node('=', 2,
            id_node("expand_operator_not_equal_node_1"),
            constant_node(50, false)),

        operator_node('=', 2,
            id_node("expand_operator_not_equal_node_2"),
            constant_node(25, false)),

        operator_node(IF, 3,
            operator_node(AND, 2,
                operator_node(NE, 2,
                    id_node("expand_operator_not_equal_node_1"),
                    constant_node(50, -1)),
                operator_node(NE, 2,
                    id_node("expand_operator_not_equal_node_2"),
                    constant_node(25, -1))),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));

    constexpr unsigned char expected_2[] =
    {
        0xad, WORD(0x0400)
    };

    const parse_node_ptr nodes_2[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        operator_node(VAR, 1, 
               operator_node(EXPRLIST, 1, id_node("expand_operator_equal_node_3"))),
        operator_node(VAR, 1, 
               operator_node(EXPRLIST, 1, id_node("expand_operator_equal_node_4"))),

        operator_node('=', 2,
            id_node("expand_operator_equal_node_3"),
            constant_node(500, false)),

        operator_node('=', 2,
            id_node("expand_operator_equal_node_4"),
            constant_node(260, false)),

        operator_node(IF, 3,
            operator_node(AND, 2,
                operator_node(NE, 2,
                    id_node("expand_operator_equal_node_3"),
                    constant_node(50, -1)),
                operator_node(NE, 2,
                    id_node("expand_operator_equal_node_4"),
                    constant_node(25, -1))),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))
    };

    expand_file_output_template(expected_2, _countof(expected_2), nodes_2, _countof(nodes_2));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_not_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xad, WORD(0x0800),
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node('=', 2,
            label_node((char*)"expand_operator_not_node"),
            constant_node(program_counter, -1)),

        operator_node(IF, 3,
            operator_node(NOT, 1,
                operator_node(GE, 2,
                    id_node("expand_operator_not_node"),
                    constant_node(program_counter, -1)),
                opcode_node(_lda, a, 1, constant_node(0x0400, false)),
                opcode_node(_lda, a, 1, constant_node(0x0800, false)))),
    };
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));

    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_ones_complement_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD((~(0x1122) & 0xFFFF))
    };
    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        data_node(data_word, operator_node(EXPRLIST, 1,
            operator_node('~', 1,
                constant_node(0x1122, false))))
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_or_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0xad, WORD(0x0400)
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        operator_node(VAR, 1,  
                operator_node(EXPRLIST, 1, id_node("expand_operator_or_node_1"))),
        operator_node(VAR, 1, 
                operator_node(EXPRLIST, 1, id_node("expand_operator_or_node_2"))),

        operator_node('=', 2,
            id_node("expand_operator_or_node_1"),
            constant_node(50, false)),

        operator_node('=', 2,
            id_node("expand_operator_or_node_2"),
            constant_node(25, false)),

        operator_node(IF, 3,
            operator_node(OR, 2,
                operator_node(EQ, 2,
                    id_node("expand_operator_or_node_1"),
                    constant_node(5000, -1)),
                operator_node(EQ, 2,
                    id_node("expand_operator_or_node_2"),
                    constant_node(25, -1))),
            opcode_node(_lda, a, 1, constant_node(0x0400, false)),
            opcode_node(_lda, a, 1, constant_node(0x0800, false)))
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_org_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x1000)
    };
    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        data_node(data_word, operator_node(EXPRLIST, 1,
                constant_node(program_counter, true)))
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));

    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_plus_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x1122 + 0x3434)
    };
    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        data_node(data_word, operator_node(EXPRLIST, 1,
            operator_node('+', 2,
                constant_node(0x1122, false),
                constant_node(0x3434, false))))
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_print_all_node_test)
{
    expand_unit_test_method_initialize();
    expand_node(operator_node(VAR, 1,
        operator_node(EXPRLIST, 1, id_node("expand_operator_print_all_node"))));

    expand_node(operator_node(PRINTALL, 1,
        id_node("expand_operator_print_all_node")));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_print_node_test)
{
    expand_unit_test_method_initialize();
    expand_node(operator_node(VAR, 1, 
        operator_node(EXPRLIST, 1, id_node("expand_operator_print_node"))));

    expand_node(operator_node(PRINT, 1,
        operator_node(EXPRLIST, 1, id_node("expand_operator_print_all_node"))));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_program_counter_assign_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x0000),
        WORD(0X0000),
        WORD(0x0000),
        WORD(0X0000)
    };

    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        operator_node(PCASSIGN, 1,
            constant_node(0x1008, false))
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_repeat_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
       0x69, 5,
       0x69, 15,
       0x69, 25,
       0x69, 35,
       0x69, 45,
       0x69, 55,
       0x69, 65,
       0x69, 75,
       0x69, 85,
       0x69, 95,
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

operator_node(VAR, 1,
                operator_node(EXPRLIST, 2,
                    id_node("repeat_var"),
                    constant_node(5, false))),

        operator_node(REPEAT,  2,

            operator_node(STATEMENT, 2,
                opcode_node(_adc, I, 1, id_node("repeat_var")),

                operator_node(STATEMENT, 1,
                    operator_node('=', 2,
                        id_node("repeat_var"),
                        operator_node('+', 2,
                            id_node("repeat_var"),
                            constant_node(10, 0))))),

            operator_node(GE, 2,
                id_node("repeat_var"),
                constant_node(100, false)))
    };
    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_section_node_test)
{
    expand_unit_test_method_initialize();
    current_file_name = (char*)"test.a";

    expand_node(
        operator_node(ORG, 1,
            constant_node(0x1000, false)));

    expand_node(
        operator_node('=', 2,
            label_node((char*)"expand_operator_global"),
            constant_node(program_counter, -1)));

    expand_node(
        opcode_node(_nop, i, 0));

    expand_node(
        operator_node(SECTION, 1, id_node("Section_One")));

    expand_node(
        opcode_node(_nop, i, 0));

    expand_node(
        operator_node('=', 2,
            label_node((char*)"expand_operator_inside_section"),
            constant_node(program_counter, -1)));

    expand_node(
        operator_node(ENDSECTION, 0));

    expand_node(
        operator_node('=', 2,
            label_node((char*)"expand_operator_global_2"),
            constant_node(program_counter, -1)));

    auto symbol_ptr = static_cast<symbol_table_ptr>(dict_search(symbol_dictionary, "expand_operator_inside_section"));
    EXPECT_NULL(symbol_ptr);

    symbol_ptr = static_cast<symbol_table_ptr>(dict_search(symbol_dictionary, "Section_One.expand_operator_inside_section"));
    EXPECT_NOT_NULL(symbol_ptr);

    symbol_ptr = static_cast<symbol_table_ptr>(dict_search(symbol_dictionary, "expand_operator_global"));
    EXPECT_NOT_NULL(symbol_ptr);

    symbol_ptr = static_cast<symbol_table_ptr>(dict_search(symbol_dictionary, "expand_operator_global_2"));
    EXPECT_NOT_NULL(symbol_ptr);

    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_shift_left_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x2200)
    };

    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        data_node(data_word,
        operator_node(EXPRLIST, 1,
            operator_node(EXPRLIST, 1,
                operator_node(SHIFT_LEFT, 2,
                    constant_node(0x0022, false),
                    constant_node(0x08, false)))))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_shift_right_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x0022)
    };

    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        data_node(data_word,
        operator_node(EXPRLIST, 1,
            operator_node(EXPRLIST, 1,
                operator_node(SHIFT_RIGHT, 2,
                    constant_node(0x2200, false),
                    constant_node(0x08, false)))))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}


TEST(expand_unit_test, expand_statement_node_test)
{
    expand_unit_test_method_initialize();
    // .org $1000
    // .var my_var
    // do
    //     adc #my_var
    //     my_var = may_var + $10
    // while my_var < $FF

    constexpr unsigned char expected[] =
    {
        0x69, 0x00,
        0x69, 0x10,
        0x69, 0x20,
        0x69, 0x30,
        0x69, 0x40,
        0x69, 0x50,
        0x69, 0x60,
        0x69, 0x70,
        0x69, 0x80,
        0x69, 0x90,
        0x69, 0xa0,
        0x69, 0xb0,
        0x69, 0xc0,
        0x69, 0xd0,
        0x69, 0xe0,
        0x69, 0xf0,
    };

    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),
        operator_node(VAR, 1, 
                operator_node(EXPRLIST, 1, id_node("my_var"))),
        operator_node(DO, 2,
            operator_node(STATEMENT, 2,
                opcode_node(_adc, I, 1, id_node("my_var")),
                operator_node(STATEMENT, 1,
                    operator_node('=', 2,
                        id_node("my_var"),
                        operator_node('+', 2,
                            id_node("my_var"),
                            constant_node(0x10, 0))))),

            operator_node('<', 2,
                id_node("my_var"), constant_node(0xFF, 0)))
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_uminus_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD( (- 0x1234) & 0xFFFF),
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        operator_node(VAR, 1, 
            operator_node(EXPRLIST, 1,  id_node("expand_operator_uminus_node"))),

        operator_node('=', 2,
            id_node("expand_operator_uminus_node"),
                operator_node(UMINUS, 1,
            constant_node(0x1234, false))),

        data_node(data_word, operator_node(EXPRLIST, 1,
        operator_node(BIT_AND, 2,
                id_node("expand_operator_uminus_node"),
                constant_node(0xFFFF, false))))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));
    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_variable_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x0000),
        WORD(0x2345),
    };

    const parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),

        operator_node(VAR, 1,
                operator_node(EXPRLIST, 1,  id_node("expand_operator_variable_node_1_test"))),

        data_node(data_word, operator_node(EXPRLIST, 1, id_node("expand_operator_variable_node_1_test"))),

        operator_node(VAR, 1,
                operator_node(EXPRLIST, 2,  
                    id_node("expand_operator_variable_node_2_test"),
                    constant_node(0x2345, false))),

        data_node(data_word, operator_node(EXPRLIST, 1, id_node("expand_operator_variable_node_2_test"))),
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));

    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_while_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        0x69, 0x00,
        0x69, 0x10,
        0x69, 0x20,
        0x69, 0x30,
        0x69, 0x40,
        0x69, 0x50,
        0x69, 0x60,
        0x69, 0x70,
        0x69, 0x80,
        0x69, 0x90,
        0x69, 0xa0,
        0x69, 0xb0,
        0x69, 0xc0,
        0x69, 0xd0,
        0x69, 0xe0,
        0x69, 0xf0,
    };

    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1, constant_node(0x1000, false)),
        operator_node(VAR, 1,
                operator_node(EXPRLIST, 1, id_node("my_var"))),

        operator_node(WHILE, 2,
            operator_node('<', 2,
                id_node("my_var"), constant_node(0xFF, 0)),

            operator_node(STATEMENT, 2,
                opcode_node(_adc, I, 1, id_node("my_var")),
                operator_node(STATEMENT, 1,
                    operator_node('=', 2,
                        id_node("my_var"),
                        operator_node('+', 2,
                            id_node("my_var"),
                            constant_node(0x10, 0))))))

    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));

    expand_unit_test_method_cleanup();
}

TEST(expand_unit_test, expand_operator_xor_node_test)
{
    expand_unit_test_method_initialize();
    constexpr unsigned char expected[] =
    {
        WORD(0x1122 ^ 0x3434)
    };
    parse_node_ptr nodes[] =
    {
        operator_node(ORG, 1,
            constant_node(0x1000, false)),

        data_node(data_word, operator_node(EXPRLIST, 1,
            operator_node('^', 2,
                constant_node(0x1122, false),
                constant_node(0x3434, false))))
    };

    expand_file_output_template(expected, _countof(expected), nodes, _countof(nodes));

}

TEST(expand_unit_test, expand_print_state_node_test)
{ 
    expand_unit_test_method_initialize();
    current_file_name = (char*)"test.a";
    expand_node(print_state_node(0));
    EXPECT_EQ(0, print_list_state);
    expand_node(print_state_node(1));
    EXPECT_EQ(1, print_list_state);
    expand_unit_test_method_cleanup();
}
