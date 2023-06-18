#include <gtest/gtest.h>
#include "unit_test.h"

#pragma warning(disable : 4996 4090)


static void opcode_unit_test_method_initialize();
static void opcode_unit_test_method_cleanup();

static void opcode_unit_test_method_initialize()
{
    initialize();

    allow_illegal_op_codes = 0;
    cpu_mode = cpu_6502;
}

static void opcode_unit_test_method_cleanup()
{
    destroy();
}

static void positive_test(const ops_ptr ops)
{
    for (auto op_ptr = ops; op_ptr->in != _maxOpcode; op_ptr++)
    {
        for (auto mode_index = 0; mode_index < op_ptr->number_of_modes; ++mode_index)
        {
            const auto mode = static_cast<addressing_modes>(op_ptr->modes[mode_index]);
            auto op_code = get_op_code(op_ptr->in, mode);
            EXPECT_EQ(op_ptr->opcodes[mode_index], op_code);
        }
    }
}

static void negative_test(const ops_ptr ops)
{
    for (auto op_ptr = ops; op_ptr->in != _maxOpcode; op_ptr++)
    {
        for (auto mode_index_1 =  - 1; mode_index_1 <= max_addressing_mode; ++mode_index_1)
        {
            bool has_index = false;
            for (auto mode_index = 0; !has_index && mode_index < op_ptr->number_of_modes; ++mode_index)
            {
                if (op_ptr->modes[mode_index] == mode_index_1)
                    has_index = true;
            }

            if (!has_index)
            {
                const auto mode = static_cast<addressing_modes>(mode_index_1);
                auto op_code = get_op_code(op_ptr->in, mode);

                if (op_code != -1)
                {
                    if (cpu_mode == cpu_65C02 && ops == cpu_6502_ops)
                    {
                        // mode is 65C02 but with 6502 instructions
                        // skip new modes for existing instructions
                        if (
                            (op_ptr->in == _adc && mode == ind) ||
                            (op_ptr->in == _and && mode == ind) ||
                            (op_ptr->in == _bit && (mode == I || mode == ax || mode == zpx)) ||
                            (op_ptr->in == _cmp && mode == ind) ||
                            (op_ptr->in == _dec && (mode == A || mode == i)) ||
                            (op_ptr->in == _eor && mode == ind) ||
                            (op_ptr->in == _inc && (mode == A || mode == i)) ||
                            (op_ptr->in == _jmp && mode == aix) ||
                            (op_ptr->in == _lda && mode == ind) ||
                            (op_ptr->in == _ora && mode == ind) ||
                            (op_ptr->in == _sbc && mode == ind) ||
                            (op_ptr->in == _sta && mode == ind)
                            )
                            continue;
                    }
                    op_code = get_op_code(op_ptr->in, mode);
                }

                EXPECT_EQ(-1, op_code);
            }
        }
    }

    auto op_code = get_op_code(-1, 0);
    EXPECT_EQ(-1, op_code);

    op_code = get_op_code(_maxOpcode, 0);
    EXPECT_EQ(-1, op_code);
}

TEST(opcode_unit_test, allow_illegal_op_codes_unit_test)
{
    opcode_unit_test_method_initialize();
    for (auto op_ptr = cpu_6502_illegal_ops; op_ptr->in != _maxOpcode; op_ptr++)
    {
        for (auto mode_index = 0; mode_index < op_ptr->number_of_modes; ++mode_index)
        {
            const auto mode = static_cast<addressing_modes>(op_ptr->modes[mode_index]);

            allow_illegal_op_codes = 0;
            auto op_code = get_op_code(op_ptr->in, mode);
            EXPECT_EQ(-1, op_code);

            allow_illegal_op_codes = -1;
            op_code = get_op_code(op_ptr->in, mode);
            EXPECT_EQ(op_ptr->opcodes[mode_index], op_code);
        }
    }
    opcode_unit_test_method_cleanup();
}

TEST(opcode_unit_test, cpu_mode_unit_test)
{
    opcode_unit_test_method_initialize();
    for (auto op_ptr = cpu_65c02_ops; op_ptr->in != _maxOpcode; op_ptr++)
    {
        for (auto mode_index = 0; mode_index < op_ptr->number_of_modes; ++mode_index)
        {
            const auto mode = static_cast<addressing_modes>(op_ptr->modes[mode_index]);
            int op_code = -1;

            // skip modes added to existing instructions in the 6502
            bool skip_6502 = false;
            if (
                (op_ptr->in == _adc && mode != ind) ||
                (op_ptr->in == _and && mode != ind) ||
                (op_ptr->in == _bit && mode != zpx && mode != ax && mode != I) ||
                (op_ptr->in == _cmp && mode != ind) ||
                (op_ptr->in == _dec && mode != A && mode != i) ||
                (op_ptr->in == _eor && mode != ind) ||
                (op_ptr->in == _inc && mode != A && mode != i) ||
                (op_ptr->in == _jmp && mode != aix) ||
                (op_ptr->in == _lda && mode != ind) ||
                (op_ptr->in == _ora && mode != ind) ||
                (op_ptr->in == _sbc && mode != ind) ||
                (op_ptr->in == _sta && mode != ind)
                    )
                skip_6502 = true;
            if (!skip_6502)
            {
                cpu_mode = cpu_6502;
                op_code = get_op_code(op_ptr->in, mode);
                EXPECT_EQ(-1, op_code);
            }
            cpu_mode = cpu_65C02;
            op_code = get_op_code(op_ptr->in, mode);
            EXPECT_EQ(op_ptr->opcodes[mode_index], op_code);
        }
    }
    opcode_unit_test_method_cleanup();
}

TEST(opcode_unit_test, get_op_code_unit_test)
{
    opcode_unit_test_method_initialize();
    
    allow_illegal_op_codes = 0;

    // Positive tests
    cpu_mode = cpu_6502;
    positive_test(cpu_6502_ops);

    cpu_mode = cpu_65C02;
    positive_test(cpu_65c02_ops);

    cpu_mode = cpu_6502;
    allow_illegal_op_codes = -1;
    positive_test(cpu_6502_illegal_ops);
    
    opcode_unit_test_method_cleanup();
}

TEST(opcode_unit_test, get_op_code_negative_unit_test)
{
    opcode_unit_test_method_initialize();
    
    allow_illegal_op_codes = 0;

    // negative tests
    cpu_mode = cpu_6502;
    negative_test(cpu_6502_ops);

    cpu_mode = cpu_65C02;
    negative_test(cpu_65c02_ops);

    cpu_mode = cpu_6502;
    allow_illegal_op_codes = -1; 
    negative_test(cpu_6502_illegal_ops);
    
    opcode_unit_test_method_cleanup();
}

TEST(opcode_unit_test, instruction_to_string_unit_test)
{
    opcode_unit_test_method_initialize();
    
    const auto ops_array = new ops_ptr[4]{ cpu_6502_ops, cpu_65c02_ops, cpu_6502_illegal_ops, nullptr };
    for (auto ops = ops_array;  *ops != nullptr; ++ops)
    {
        const auto ops_ptr_array_start = *ops;
        for (auto op_ptr = ops_ptr_array_start; op_ptr != nullptr && op_ptr->in != _maxOpcode; op_ptr++)
        {
            const auto str = instruction_to_string(op_ptr->in);
            EXPECT_NOT_NULL(str);
            EXPECT_TRUE(strlen(str) > 0);
            // ReSharper disable once CppDeprecatedEntity
            EXPECT_FALSE(stricmp(str, "Unknown") == 0);  // NOLINT(clang-diagnostic-deprecated-declarations)
        }
    }
    
    opcode_unit_test_method_cleanup();
}

TEST(opcode_unit_test, mode_to_string_unit_test)
{
    opcode_unit_test_method_initialize();
    for (int address_mode = 0; address_mode < max_addressing_mode; ++address_mode)
    {
        const auto mode = static_cast<addressing_modes>(address_mode);
        const auto str = mode_to_string(mode);
        EXPECT_NOT_NULL(str);
        EXPECT_TRUE(strlen(str) > 0);
        // ReSharper disable once CppDeprecatedEntity
        EXPECT_FALSE(stricmp(str, "Unknown") == 0);  // NOLINT(clang-diagnostic-deprecated-declarations)        
    }
    opcode_unit_test_method_cleanup();
}

TEST(opcode_unit_test, find_modified_instructions)
{
    opcode_unit_test_method_initialize();
    allow_illegal_op_codes = 0;

    // negative tests
    cpu_mode = cpu_65C02;
    negative_test(cpu_6502_ops);
    opcode_unit_test_method_cleanup();
}
