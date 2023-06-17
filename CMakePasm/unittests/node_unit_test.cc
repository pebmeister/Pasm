#include <gtest/gtest.h>
#include "unit_test.h"

// ReSharper disable CppStringLiteralToCharPointerConversion
// ReSharper disable StringLiteralTypo
// ReSharper disable CppClangTidyCertErr33C
// ReSharper disable CppClangTidyClangDiagnosticWritableStrings
// ReSharper disable CppDeclaratorNeverUsed
// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyClangDiagnosticDeprecatedDeclarations


const char* names_[4] =
{
    "Test1",
    "Test2",
    "Test3",
    "aVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongName"
};

static void node_unit_test_method_initialize();
static void node_unit_test_method_cleanup();

static void node_unit_test_method_initialize()
{
    initialize();

    free_parse_tree();
    reset_lex();
    current_file_name = (char*)"test.a";
}

static void node_unit_test_method_cleanup()
{
    destroy();
    free_parse_tree();
    program_counter = 0;
}

static int count_nodes()
{
    int count = 0;
    for (auto node_ptr = head_node; node_ptr != nullptr; node_ptr = node_ptr->next)
        ++count;
    return count;
}

TEST(node_unit_test, operator_node_test)
{
    node_unit_test_method_initialize();
    const auto operator_node_ptr = operator_node(STATEMENT, 0);
    auto valid_node = is_valid_parse_node(operator_node_ptr);
    EXPECT_NOT_NULL(operator_node_ptr);
    EXPECT_NE(0, valid_node);

    const auto opcode_node_ptr = opcode_node(_nop, i, 0);
    valid_node = is_valid_parse_node(operator_node_ptr);
    EXPECT_NOT_NULL(opcode_node_ptr);
    EXPECT_NE(0, valid_node);

    const auto operator2_node_ptr = operator_node(STATEMENT, 1, opcode_node_ptr);
    valid_node = is_valid_parse_node(operator_node_ptr);
    EXPECT_NOT_NULL(operator2_node_ptr);
    EXPECT_NE(0, valid_node);
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, opcode_node_test)
{
    node_unit_test_method_initialize();
    // ReSharper disable once CppTooWideScope
    parse_node_ptr value_node_ptr = nullptr;
    int valid_node = 0;

    for (int mode = cpu_6502; mode <= cpu_65C02;  mode++)
    {
        cpu_mode = static_cast<cpu>(mode);
        for (int instruction = 0x00; instruction < _maxOpcode; instruction++)
        {
            if (instruction == 56)
            {
                printf("l\n");
            }
            for (int addressing = 0; addressing < max_addressing_mode; addressing++)
            {
                int addressing_mode = addressing;
                for (auto illegal = 0; illegal < 2; illegal++)
                {
                    allow_illegal_op_codes = illegal;

                    auto opcode = get_op_code(instruction, addressing_mode);
                    parse_node_ptr opcode_node_ptr = nullptr;
                    int nops = 0;

                    switch (addressing_mode)
                    {
                    case i:
                    case A:
                        value_node_ptr = nullptr;
                        opcode_node_ptr = opcode_node(instruction, addressing_mode, 0);
                        break;

                    case I:
                    case zp:
                    case zpi:
                    case zpx:
                    case zpy:
                    case izx:
                    case izy:
                        nops = 1;
                        value_node_ptr = constant_node(0x55, 0);
                        opcode_node_ptr = opcode_node(instruction, addressing_mode, nops, value_node_ptr);
                        break;

                    case a:
                    case aix:
                    case ax:
                    case ay:
                    case ind:
                    case r:
                        nops = 1;
                        value_node_ptr = constant_node(program_counter + 50, 0);
                        opcode_node_ptr = opcode_node(instruction, addressing_mode, nops, value_node_ptr);
                        break;

                    default: 
                        nops = 0;
                        break;
                    }
                    if (opcode_node_ptr->opcode.mode != addressing_mode)
                    {
                        if (get_op_code(instruction, r) != -1)
                        {
                            addressing_mode = r;
                        }
                        switch (addressing_mode)
                        {
                        case a:     /* absolute             */
                            addressing_mode = zp;
                            break;

                        case aix:   /* absolute indirect x  */
                            addressing_mode = izx;
                            break;

                        case ax:    /* absolute x           */
                            addressing_mode = zpx;
                            break;

                        case ay:    /* absolute y           */
                            addressing_mode = zpy;
                            break;

                        case ind:   /* absolute indirect    */
                            addressing_mode = zpi;
                            break;

                        default:
                            break;
                        }
                        opcode = get_op_code(instruction, addressing_mode);
                    }
                    valid_node = is_valid_parse_node(opcode_node_ptr);
                    EXPECT_NOT_NULL(opcode_node_ptr);
                    EXPECT_NE(0, valid_node);
                    EXPECT_EQ(nops, opcode_node_ptr->number_of_ops);
                    EXPECT_EQ(static_cast<int>(type_op_code), static_cast<int>(opcode_node_ptr->type));
                    EXPECT_EQ(instruction, opcode_node_ptr->opcode.instruction);
                    EXPECT_EQ(addressing_mode, opcode_node_ptr->opcode.mode);
                    EXPECT_EQ(opcode, opcode_node_ptr->opcode.opcode);

                    if (nops > 0)
                    {
                        valid_node = is_valid_parse_node(value_node_ptr);
                        EXPECT_NOT_NULL(value_node_ptr);
                        EXPECT_NE(0, valid_node);
                    }
                }
            }
        }
    }
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, id_node_test)
{
    node_unit_test_method_initialize();
    const auto id_node_ptr = id_node(__FILE__);
    const auto valid_node = is_valid_parse_node(id_node_ptr);
    const int is_same(strcmp(id_node_ptr->id.name, __FILE__));

    EXPECT_NOT_NULL(id_node_ptr);
    EXPECT_NE(0, valid_node);
    EXPECT_EQ(static_cast<int>(type_id), static_cast<int>(id_node_ptr->type));
    EXPECT_EQ(0,is_same);
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, macro_id_node_test)
{
    node_unit_test_method_initialize();
    for (const char* name : names_)
    {
        const auto macro_id_node_ptr = macro_id_node(name);
        const auto valid_node = is_valid_parse_node(macro_id_node_ptr);
        const int is_same(strcmp(macro_id_node_ptr->id.name, name));

        EXPECT_NOT_NULL(macro_id_node_ptr);
        EXPECT_NE(0, valid_node);
        EXPECT_EQ(static_cast<int>(type_macro_id), static_cast<int>(macro_id_node_ptr->type));
        EXPECT_EQ(0, is_same);
    }
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, constant_node_test)
{
    node_unit_test_method_initialize();
    const auto const_node_ptr = constant_node(0x4554, 0);
    const auto valid_node = is_valid_parse_node(const_node_ptr);
    EXPECT_NE(0, valid_node);
    EXPECT_EQ(0, const_node_ptr->con.is_program_counter);
    EXPECT_EQ(0x4554, const_node_ptr->con.value );
    EXPECT_EQ(static_cast<int>(type_con), static_cast<int>(const_node_ptr->type));

    program_counter = 0x2222;

    const auto pc_node_ptr = constant_node(0x33, 1);
    const auto pc_valid_node = is_valid_parse_node(pc_node_ptr);
    EXPECT_NE(0, pc_valid_node);
    EXPECT_NE(0, pc_node_ptr->con.is_program_counter);
    EXPECT_EQ(program_counter, pc_node_ptr->con.value);
    EXPECT_EQ(static_cast<int>(type_con), static_cast<int>(pc_node_ptr->type));
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, string_node_test)
{
    node_unit_test_method_initialize();
    const auto escaped_string = escape_string(__FILE__);
    auto string_node_ptr = string_node(escaped_string);
    auto valid_node = is_valid_parse_node(string_node_ptr);
    int is_same(strcmp(string_node_ptr->str.value, __FILE__));

    EXPECT_NOT_NULL(string_node_ptr);
    EXPECT_NE(0, valid_node);
    EXPECT_EQ(static_cast<int>(type_str), static_cast<int>(string_node_ptr->type));
    EXPECT_EQ(0, is_same);

    for (const char* name : names_)
    {
        string_node_ptr = string_node(name);
        valid_node = is_valid_parse_node(string_node_ptr);
        is_same = strcmp(name, string_node_ptr->str.value);

        EXPECT_NOT_NULL(string_node_ptr);
        EXPECT_NE(valid_node, 0);
        EXPECT_EQ(static_cast<int>(string_node_ptr->type), static_cast<int>(type_str));
        EXPECT_EQ(0, is_same);

    }
    free(escaped_string);
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, macro_expand_node_test)
{
    node_unit_test_method_initialize();
    for (const char* name : names_)
    {
        const auto param_node_ptr = allocate_node(0);
        const auto macro_expand_node_ptr = macro_expand_node(name, param_node_ptr);
        auto valid_node = is_valid_parse_node(macro_expand_node_ptr);
        auto is_same(strcmp(name, macro_expand_node_ptr->macro.name));

        EXPECT_NOT_NULL(macro_expand_node_ptr);
        EXPECT_NE(valid_node, 0);
        EXPECT_EQ(static_cast<int>(type_macro_ex), static_cast<int>(macro_expand_node_ptr->type));
        EXPECT_EQ(0, is_same);
    }
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, data_node_test)
{
    node_unit_test_method_initialize();
    const auto const_node_ptr = constant_node(256, 0);
    const auto data_node_ptr = data_node(data_byte, const_node_ptr);

    EXPECT_NOT_NULL(const_node_ptr);
    EXPECT_TRUE(is_valid_parse_node(const_node_ptr));

    EXPECT_NOT_NULL(data_node_ptr);
    EXPECT_TRUE(is_valid_parse_node(data_node_ptr));
    EXPECT_EQ(static_cast<int>(type_data), static_cast<int>(data_node_ptr->type));
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, print_state_node_test)
{
    node_unit_test_method_initialize();
    auto print_state_node_ptr = print_state_node(-1);
    EXPECT_NOT_NULL(print_state_node_ptr);
    EXPECT_TRUE(is_valid_parse_node(print_state_node_ptr));
    EXPECT_EQ(static_cast<int>(type_print), static_cast<int>(print_state_node_ptr->type));
    EXPECT_EQ(-1, print_state_node_ptr->pr.print_state);

    print_state_node_ptr = print_state_node(0);
    EXPECT_NOT_NULL(print_state_node_ptr);
    EXPECT_TRUE(is_valid_parse_node(print_state_node_ptr));
    EXPECT_EQ(static_cast<int>(type_print), static_cast<int>(print_state_node_ptr->type));
    EXPECT_EQ(0, print_state_node_ptr->pr.print_state);
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, allocate_node_unit_test)
{
    node_unit_test_method_initialize();
    for (auto nops = 0; nops < 10; ++nops)
    {
        const auto allocate_node_ptr = allocate_node(nops);
        EXPECT_NOT_NULL(allocate_node_ptr);
        EXPECT_EQ(nops, allocate_node_ptr->number_of_ops);
        if (nops > 0)
        {
            EXPECT_NOT_NULL(allocate_node_ptr->op);
        }
        else
        {
            EXPECT_NULL(allocate_node_ptr->op);
        }
    }
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, is_valid_parse_node_unit_test)
{
    node_unit_test_method_initialize();
    const auto is_valid_parse_node_ptr = allocate_node(0);
    is_valid_parse_node_ptr->type = static_cast<node_type_enum>(- 353);
    EXPECT_NOT_NULL(is_valid_parse_node_ptr);
    EXPECT_FALSE(is_valid_parse_node(is_valid_parse_node_ptr));

    is_valid_parse_node_ptr->type = type_data;
    EXPECT_TRUE(is_valid_parse_node(is_valid_parse_node_ptr));

    const auto is_valid_parse_node_ops_ptr = allocate_node(1);
    is_valid_parse_node_ops_ptr->type = type_data;
    is_valid_parse_node_ops_ptr->op[0] = is_valid_parse_node_ptr;
    is_valid_parse_node_ptr->type = static_cast<node_type_enum>(-353);

    EXPECT_NOT_NULL(is_valid_parse_node_ops_ptr);
    EXPECT_FALSE(is_valid_parse_node(is_valid_parse_node_ops_ptr));

    is_valid_parse_node_ptr->type = type_data;
    EXPECT_TRUE(is_valid_parse_node(is_valid_parse_node_ops_ptr));
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, is_valid_parse_tree_unit_test)
{
    node_unit_test_method_initialize();
    const auto node1_ptr = operator_node('=',0,
        id_node("left_node"),
        constant_node(0x4554, false));
    const auto node2_ptr = opcode_node(0x55, r, 0);
    const auto node3_ptr = id_node("test_id_node");
    const auto node4_ptr = label_node((char*)"test_label_node");
    const auto node5_ptr = macro_id_node("macro_id_node");
    const auto node6_ptr = constant_node(0x55, false);
    const auto node7_ptr = string_node("string_node");
    const auto node8_ptr = macro_expand_node("macro_expand_node", nullptr);
    const auto node9_ptr = data_node(data_byte, node6_ptr);
    const auto node10_ptr = print_state_node(1);

    EXPECT_TRUE(is_valid_parse_tree());

    const auto is_valid_parse_node_ptr = allocate_node(0);
    is_valid_parse_node_ptr->type = static_cast<node_type_enum>(-353);

    EXPECT_FALSE(is_valid_parse_tree());
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, print_node_unit_test)
{
    node_unit_test_method_initialize();
    const char* expected_lines[] =
    {
"NODE [line 0]\n",
"    allocated 1\n",
"    type typeOpr\n",
"    opr '='\n",
"    CHILD NODE [line 0]\n",
"        allocated 1\n",
"        type type_id\n",
"        name left_node\n",
"        i    (nil)\n",
"\n",
"    CHILD NODE [line 0]\n",
"        allocated 1\n",
"        type typeCon\n",
"        IsPC  0\n",
"        value 0x00004554\n",
"\n",
"NODE [line 1]\n",
"    allocated 1\n",
"    type typeOpCode\n",
"    instruction   nop\n",
"    mode          implied\n",
"    opCode        EA\n",
"    PC            0x00000000\n",
"\n",
"NODE [line 2]\n",
"    allocated 1\n",
"    type type_id\n",
"    name test_id_node\n",
"    i    (nil)\n",
"\n",
"NODE [line 3]\n",
"    allocated 1\n",
"    type type_label\n",
"    name test_label_node\n",
"    i    (nil)\n",
"\n",
"NODE [line 4]\n",
"    allocated 1\n",
"    type type_macro_id\n",
"    name macro_id_node\n",
"    i    (nil)\n",
"\n",
"NODE [line 5]\n",
"    allocated 1\n",
"    type typeCon\n",
"    IsPC  0\n",
"    value 0x00000055\n",
"NODE [line 6]\n",
"    allocated 1\n",
"    type typeStr\n",
"    allocated  string_node\n",
"    len        0x0000000B\n",
"    value string_node\n",
"\n",
"NODE [line 7]\n",
"    allocated 1\n",
"\n",
"NODE [line 8]\n",
"    allocated 1\n",
"    type typeData\n",
"    size 1\n",
"    CHILD NODE [line 8]\n",
"        allocated 1\n",
"        type typeCon\n",
"        IsPC  0\n",
"        value 0x00000055\n",
"\n",
"NODE [line 9]\n",
"    allocated 1\n",
"\n",
    };
    constexpr int number_of_lines = _countof(expected_lines);

    const auto test_file = "test_print.txt";
    remove(test_file);
    log_file = fopen(test_file, "w");
    EXPECT_NOT_NULL(log_file);

    yylineno = 0;
    current_node = operator_node('=', 2,
        id_node("left_node"),
        constant_node(0x4554, false));
    print_node(current_node, log_file);

    ++yylineno;
    current_node = opcode_node(_nop, i, 0);
    print_node(current_node, log_file);

    ++yylineno;
    current_node = id_node("test_id_node");
    print_node(current_node, log_file);

    ++yylineno;
    current_node = label_node((char*)"test_label_node");
    print_node(current_node, log_file);

    ++yylineno;
    current_node = macro_id_node("macro_id_node");
    print_node(current_node, log_file);

    ++yylineno;
    current_node = constant_node(0x55, false);
    print_node(current_node, log_file);

    ++yylineno;
    current_node = string_node("string_node");
    print_node(current_node, log_file);

    ++yylineno;
    current_node = macro_expand_node("macro_expand_node", nullptr);
    print_node(current_node, log_file);

    ++yylineno;
    current_node = data_node(data_byte, constant_node(0x55, false));
    print_node(current_node, log_file);

    ++yylineno;
    current_node = print_state_node(1);
    print_node(current_node, log_file);

    EXPECT_TRUE(is_valid_parse_tree());

    current_node = head_node;
    while (current_node)
    {
        current_node = current_node->next;
    }

    fclose(log_file);
    auto lines = read_file_lines(test_file);

    auto current_file_line = lines;
    for (const auto a : expected_lines)
    {
        const char* b = lines->line_content;
        auto cmp = strcmp(a, b);
     
        EXPECT_STREQ(a, b);
        EXPECT_EQ(0, cmp);
        lines = lines->next;
    }

    // remove(test_file);
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, free_parse_node_unit_test)
{
    node_unit_test_method_initialize();
    current_node = operator_node('=', 2,
        id_node("left_node"),
        constant_node(0x4554, false));
    current_node = opcode_node(_nop, i, 0);
    current_node = id_node("test_id_node");
    current_node = label_node((char*)"test_label_node");
    current_node = macro_id_node("macro_id_node");
    current_node = constant_node(0x55, false);
    current_node = string_node("string_node");
    current_node = macro_expand_node("macro_expand_node", nullptr);
    current_node = data_node(data_byte, constant_node(0x55, false));
    current_node = print_state_node(1);
    auto count = count_nodes();

    while (count > 0)
    {
        const auto last_count = count;

        parse_node_ptr node = head_node->next;
        if (!node)
            node = head_node;

        const auto prev = node->prev;
        const auto next = node->next;

        free_parse_node(node);

        if (prev)
            prev->next = next;
        if (next)
            next->prev = prev;

        if (next == nullptr && prev == nullptr)
            head_node = nullptr;

        count = count_nodes();
        EXPECT_TRUE(last_count > count);
    }
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, remove_parse_node_unit_test)
{
    node_unit_test_method_initialize();
    current_node = operator_node( '=', 2,
        id_node("left_node"),
        constant_node(0x4554, false));
    current_node = opcode_node(_nop, i, 0);
    current_node = id_node("test_id_node");
    current_node = label_node((char*)"test_label_node");
    current_node = macro_id_node("macro_id_node");
    current_node = constant_node(0x55, false);
    current_node = string_node("string_node");
    current_node = macro_expand_node("macro_expand_node", nullptr);
    current_node = data_node(data_byte, constant_node(0x55, false));
    current_node = print_state_node(1);
    auto count = count_nodes();

    while (count > 0)
    {
        parse_node_ptr node = head_node;
        if (node->next)
            node = node->next;

        const auto last_count = count;
        remove_parse_node(node);
        count = count_nodes();

        EXPECT_TRUE(last_count > count);
    }
    EXPECT_NULL(head_node);
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, free_parse_tree_unit_test)
{
    node_unit_test_method_initialize();
    current_node = operator_node('=', 2,
        id_node("left_node"),
        constant_node(0x4554, false));
    current_node = opcode_node(_nop, i, 0);
    current_node = id_node("test_id_node");
    current_node = label_node((char*)"test_label_nod e");
    current_node = macro_id_node("macro_id_node");
    current_node = constant_node(0x55, false);
    current_node = string_node("string_node");
    current_node = macro_expand_node("macro_expand_node", nullptr);
    current_node = data_node(data_byte, constant_node(0x55, false));
    current_node = print_state_node(1);

    auto count = count_nodes();
    EXPECT_TRUE(count > 0);

    free_parse_tree();

    count = count_nodes();
    EXPECT_TRUE(count == 0);
    node_unit_test_method_cleanup();
}

TEST(node_unit_test, generate_list_node_test)
{
    node_unit_test_method_initialize();
    current_file_name = (char*)"test.a";
    generate_list_node(nullptr);
    node_unit_test_method_cleanup();
}
