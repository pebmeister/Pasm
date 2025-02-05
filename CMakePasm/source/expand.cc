// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppClangTidyConcurrencyMtUnsafe
// ReSharper disable CppClangTidyCertErr33C
// ReSharper disable CppClangTidyClangDiagnosticImplicitIntConversion
// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticShorten64To32
// ReSharper disable CppPointerConversionDropsQualifiers
// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyPerformanceNoIntToPtr
// ReSharper disable CppClangTidyClangDiagnosticIntToVoidPointerCast
#pragma warning(disable:4996 4267 4312 4090)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#include "expand.h"
#include "node.h"
#include "opcodes.h"
#include "pasm.tab.h"
#include "error.h"
#include "file.h"
#include "flex.h"
#include "genoutput.h"
#include "memory.h"
#include "pasm.h"

extern bool Debug_AddList;
extern void print_node(parse_node_ptr p, std::ostream& file);

enum {
    max_macro_param_name_len = 25
};

static int program_counter_assigned = 0;
static int code_generated = 0;
int max_macro_param = 0;
extern int origin;

// for 6502 and 65C02
int max_address = 0xFFFF;

// number of symbols whose values changed
int sym_value_changed = 0;

// nest level of expand_node();
int expand_level = 0;

std::allocator<macro_dict_entry> macro_entry_allocator;
std::map<std::string, macro_dict_entry*> macro_dict;

int print_list_state = 1;

// ReSharper disable once CppInconsistentNaming

/// <summary>
/// how to expand_node an expression
/// </summary>
enum ExprExpansionType {
    macro_parameter,
    data_bytes,
    data_strings,
    print,
    print_all,
    symbol
};

/// <summary>
/// function pointer to expand_node node
/// </summary>
typedef int (*expr)(parse_node_ptr p);

//
// table lookup for operators
//
typedef struct op_table {
    int tag;
    expr function;
} op_table;

//
// table for FOR REGX REGY
const int cmp[] =
{
    _cpy,
    _cpx
};

const int ld[] =
{
    _ldy,
    _ldx
};

const int inc[] =
{
    _iny,
    _inx
};

const int dec[] =
{
    _dey,
    _dex
};

char* current_scope = nullptr;
char* last_label = nullptr;

// 
// function prototypes
//
int expand_constant_node(parse_node_ptr p);
int expand_id_node(parse_node_ptr p);
int expand_label_node(parse_node_ptr p);
int expand_macro_expansion_node(parse_node_ptr p);
int expand_macro_id_node(parse_node_ptr p);
int expand_data_node(parse_node_ptr p);
int expand_op_code_node(parse_node_ptr p);
int expand_print_state_node(parse_node_ptr p);
int expand_operator_node(parse_node_ptr p);
int expand_operator_lo_byte_node(parse_node_ptr p);
int expand_operator_hi_byte_node(parse_node_ptr p);
int expand_operator_program_counter_assign_node(parse_node_ptr p);
int expand_operator_org_node(parse_node_ptr p);
int expand_operator_expression_list_node(parse_node_ptr p);
int expand_operator_macro_definition_node(parse_node_ptr p);
int expand_operator_while_node(parse_node_ptr p);
int expand_operator_repeat_node(parse_node_ptr p);
int expand_operator_do_node(parse_node_ptr p);
int expand_operator_for_node(parse_node_ptr p);
int expand_operator_if_node(parse_node_ptr p);
int expand_operator_print_node(parse_node_ptr p);
int expand_operator_print_all_node(parse_node_ptr p);
int expand_operator_ds_node(parse_node_ptr p);
int expand_operator_statement_node(parse_node_ptr p);
int expand_operator_end_node(parse_node_ptr p);
int expand_operator_equ_node(parse_node_ptr p);
int expand_operator_uminus_node(parse_node_ptr p);
int expand_operator_ones_complement_node(parse_node_ptr p);
int expand_operator_plus_node(parse_node_ptr p);
int expand_operator_minus_node(parse_node_ptr p);
int expand_operator_multiply_node(parse_node_ptr p);
int expand_operator_divide_node(parse_node_ptr p);
int expand_operator_bit_or_node(parse_node_ptr p);
int expand_operator_bit_and_node(parse_node_ptr p);
int expand_operator_xor_node(parse_node_ptr p);
int expand_operator_less_than_node(parse_node_ptr p);
int expand_operator_greater_than_node(parse_node_ptr p);
int expand_operator_or_node(parse_node_ptr p);
int expand_operator_and_node(parse_node_ptr p);
int expand_operator_equal_node(parse_node_ptr p);
int expand_operator_not_equal_node(parse_node_ptr p);
int expand_operator_greater_than_or_equal_node(parse_node_ptr p);
int expand_operator_less_than_or_equal_node(parse_node_ptr p);
int expand_operator_shift_left_node(parse_node_ptr p);
int expand_operator_shift_right_node(parse_node_ptr p);
int expand_operator_not_node(parse_node_ptr p);
int expand_operator_for_reg_node(parse_node_ptr p);
int expand_operator_section_node(parse_node_ptr p);
int expand_operator_end_section_node(parse_node_ptr p);
int expand_operator_variable_node(parse_node_ptr p);
int expand_operator_include_node(parse_node_ptr p);
int expand_operator_load_node(parse_node_ptr p);
int expand_operator_fill_node(parse_node_ptr p);

//
// end_expansion flag
int end_expansion = 0;

//
// MacroParameterIndex Index
//
int macro_parameter_index = 0;

//
// flag to unroll loops
// always true for now
//
bool unroll_loop = true;

//
// Count nest level for FORX
//
int for_x_count = 0;

//
// Count nest level for FORY
//
int for_y_count = 0;

//
// node to inject a byte
//
static parse_node_ptr generate_byte_node = nullptr;
static parse_node_ptr generate_fill_node1 = nullptr;
static parse_node_ptr generate_fill_node2 = nullptr;

//
// expansion type for expressions
//
static ExprExpansionType expansion_type = macro_parameter;

std::map<int, expr> expand_table_dictionary =
{
    { type_con,         expand_constant_node                    },
    { type_id,          expand_id_node                          },
    { type_label,       expand_label_node                       },
    { type_macro_id,    expand_macro_id_node                    },
    { type_macro_ex,    expand_macro_expansion_node             },
    { type_data,        expand_data_node                        },
    { type_op_code,     expand_op_code_node                     },
    { type_print,       expand_print_state_node                 },
    { type_opr,         expand_operator_node                    }
};

std::map<int, expr> expand_operator_table_dictionary =
{
    { LOBYTE,       expand_operator_lo_byte_node                },
    { HIBYTE,       expand_operator_hi_byte_node                },
    { PCASSIGN,     expand_operator_program_counter_assign_node },
    { ORG,          expand_operator_org_node                    },
    { EXPRLIST,     expand_operator_expression_list_node        },
    { MACRO,        expand_operator_macro_definition_node       },
    { WHILE,        expand_operator_while_node                  },
    { REPEAT,       expand_operator_repeat_node                 },
    { SECTION,      expand_operator_section_node                },
    { ENDSECTION,   expand_operator_end_section_node            },
    { DO,           expand_operator_do_node                     },
    { FOR,          expand_operator_for_node                    },
    { REGX,         expand_operator_for_reg_node                },
    { REGY,         expand_operator_for_reg_node                },
    { IF,           expand_operator_if_node                     },
    { PRINT,        expand_operator_print_node                  },
    { PRINTALL,     expand_operator_print_all_node              },
    { DS,           expand_operator_ds_node                     },
    { STATEMENT,    expand_operator_statement_node              },
    { END,          expand_operator_end_node                    },
    { EQU,          expand_operator_equ_node                    },
    { '=',          expand_operator_equ_node                    },
    { UMINUS,       expand_operator_uminus_node                 },
    { '~',          expand_operator_ones_complement_node        },
    { '+',          expand_operator_plus_node                   },
    { '-',          expand_operator_minus_node                  },
    { '*',          expand_operator_multiply_node               },
    { '/',          expand_operator_divide_node                 },
    { BIT_OR,       expand_operator_bit_or_node                 },
    { BIT_AND,      expand_operator_bit_and_node                },
    { '^',          expand_operator_xor_node                    },
    { '<',          expand_operator_less_than_node              },
    { '>',          expand_operator_greater_than_node           },
    { EQ,           expand_operator_equal_node                  },
    { NE,           expand_operator_not_equal_node              },
    { GE,           expand_operator_greater_than_or_equal_node  },
    { LE,           expand_operator_less_than_or_equal_node     },
    { OR,           expand_operator_or_node                     },
    { AND,          expand_operator_and_node                    },
    { NOT,          expand_operator_not_node                    },
    { SHIFT_LEFT,   expand_operator_shift_left_node             },
    { SHIFT_RIGHT,  expand_operator_shift_right_node            },
    { VAR,          expand_operator_variable_node               },
    { INC,          expand_operator_include_node                },
    { LOAD,         expand_operator_load_node                   },
    { FILL,         expand_operator_fill_node                   },
};

//
// Validate - label name
//
int is_plus_minus_symbol_name_valid(char* name)
{
    if (name == nullptr) return 0;
    if (name[0] != '-' && name[0] != '+') return 0;

    for (char* ptr = name; *ptr; ++ptr)
        if (name[0] != *ptr)
            return 0;
    return -1;
}

//
// constant node
//
int expand_constant_node(const parse_node_ptr p)
{
    if (p->con.is_program_counter) {
        if (p->con.value != program_counter) {
            p->con.value = program_counter;
        }
    }
    return p->con.value;
}

//
// include node
//
int expand_operator_include_node(const parse_node_ptr p)
{
    char* file = p->operands[0]->str.value;
    open_include_file(file);

    return 1;
}

//
// Fill mode
//
int expand_operator_fill_node(const parse_node_ptr p)
{
    const int byt = expand_node(p->operands[0]);
    int count = expand_node(p->operands[1]);

    if (byt < 0 || byt > 255) {
        error(error_value_out_of_range);
        exit(-1);
    }
    if (count < 0) {
        error(error_value_out_of_range);
        exit(-1);
    }
    if (final_pass) {
        data_size = 0;
        generate_fill_node2 = nullptr;
        generate_fill_node1 = nullptr;

        while (count > 0) {
            if (count > 1) {
                data_size = data_word;
                if (generate_fill_node2 == nullptr) {
                    generate_fill_node2 = data_node(data_word, constant_node(byt << 8 | byt, false));
                }
                int bytes = generate_output(output_file, generate_fill_node2);
                if (bytes == 0)
                    bytes = 2;
                generate_list_node(generate_fill_node2);
                program_counter += bytes;
                count -= bytes;

            }
            else if (count == 1) {
                data_size = data_byte;
                if (generate_fill_node1 == nullptr) {
                    generate_fill_node1 = data_node(data_byte, constant_node(byt, false));
                }
                int bytes = generate_output(output_file, generate_fill_node1);
                if (bytes == 0)
                    bytes = 1;
                generate_list_node(generate_fill_node1);
                program_counter += bytes;
                count -= bytes;

            }
        }
    }
    else {
        program_counter += count > 0 ? count : 1;
    }
    return byt;
}

//
// Load node
//
int expand_operator_load_node(const parse_node_ptr p)
{
    const char* file = p->operands[0]->str.value;
    FILE* fd = open_file(file, "rb");
    if (fd == nullptr) {
        error(error_cant_open_include_file);
    }

    fseek(fd, 0, SEEK_END);
    const size_t len = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    size_t pos = ftell(fd);

    while (!feof(fd) && len - pos > 0) {
        pos = ftell(fd);
        if (len - pos > 1) {
            const unsigned char a = fgetc(fd);
            const unsigned char b = fgetc(fd);

            data_size = 2;
            expand_node(
                data_node(data_size,
                    operator_node(EXPRLIST, 1,
                        constant_node((a | b << 8) & 0xFFFF, false))));
            continue;
        }
        if (len - pos > 0) {
            data_size = 1;
            const unsigned char ch = fgetc(fd);
            expand_node(
                data_node(data_size,
                    operator_node(EXPRLIST, 1,
                        constant_node(ch, false))));
        }
        pos = ftell(fd);
    }
    fclose(fd);
    return 1;
}

//
// Variable
//
int expand_operator_variable_node(const parse_node_ptr p)
{
    const parse_node_ptr pp = p->operands[0];
    if (pp) {
        expansion_type = symbol;
        return expand_node(pp);
    }
    error(error_missing_parameter);
    return 0;
}

int expand_plus_minus_node(const parse_node_ptr p)
{
    if (!is_plus_minus_symbol_name_valid(p->id.name)) {
        error(error_adding_symbol);
        return 0;
    }

    const int index = p->id.name[0] == '-'
        ? find_minus_symbol(strlen(p->id.name), current_file_name, yylineno)
        : find_plus_symbol(strlen(p->id.name), current_file_name, yylineno);

    const symbol_table_ptr sym = p->id.symbol_ptr == nullptr ? add_symbol(p->id.name) : p->id.symbol_ptr;
    sym->is_plus_minus = true;
    if (index >= 0) {
        sym->value = p->id.name[0] == '-'
            ? minus_symbol_table[index].value
            : plus_symbol_table[index].value;

        sym->is_initialized = true;
        sym->times_accessed++;
    }
    p->id.symbol_ptr = sym;

    return sym->value;
}

//
// symbol
//
int expand_id_node(const parse_node_ptr p)
{
    if (p->id.name[0] == '-' || p->id.name[0] == '+')
        return expand_plus_minus_node(p);

    symbol_table_ptr sym = p->id.symbol_ptr;
    if (p->id.name[0] == '@') {
        sym = add_symbol(p->id.name);
        p->id.symbol_ptr = sym;
    }
    if (sym == nullptr) {
        sym = add_symbol(p->id.name);
        p->id.symbol_ptr = sym;
        if (sym == nullptr) {
            // FatalError(error_out_of_memory);
            return 0;
        }
    }
    return sym->is_initialized ? sym->value : 0;
}

//
// Label node
//
int expand_label_node(const parse_node_ptr p)
{
    if (p->id.name[0] == '-' || p->id.name[0] == '+')
        return expand_plus_minus_node(p);

    symbol_table_ptr symbol_ptr = p->id.symbol_ptr;
    if (symbol_ptr == nullptr) {
        p->id.symbol_ptr = add_symbol(p->id.name);
        symbol_ptr = p->id.symbol_ptr;
        if (symbol_ptr == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }
    }
    symbol_ptr->is_label = true;
    if (!symbol_ptr->is_initialized) {
        for (auto& op : p->operands) {
            expand_node(op);
        }
    }
 
    return symbol_ptr->is_initialized ? symbol_ptr->value : 0;
}

//
// Reg loop
//
int expand_operator_for_reg_node(const parse_node_ptr p)
{
    // must convert to 1 or 0
    const int is_reg_x = p->opr.opr == REGX ? 1 : 0;

    parse_node_ptr loop_branch = nullptr;
    const int cmp_op = cmp[is_reg_x];
    const int ld_op = ld[is_reg_x];
    int inc_op = inc[is_reg_x];
    int dec_op = dec[is_reg_x];
    int* for_loop_counter = is_reg_x ? &for_x_count : &for_y_count;

    // check for nested loop
    if (*for_loop_counter > 0) {
        error(error_for_reg_cant_be_nested);
        return 0;
    }

    // get the start value
    const int start = expand_node(p->operands[0]);

    // range check
    if (start < 0 || start > 255) {
        error(error_value_out_of_range);
        return 0;
    }

    // get the end value
    const int end = expand_node(p->operands[1]);

    // range check
    if (end < 0 || end > 255) {
        error(error_value_out_of_range);
        return 0;
    }

    // get flag for increment / decrement
    const int increment_flag = expand_node(p->operands[3]);

    // check for loop that would have zero iterations
    // we may want to warn user
    if ((increment_flag > 0 && start > end) || (increment_flag < 0 && start < end)) {
        return 0;
    }

    // increment nest level
    if (++*for_loop_counter != 1) {
        error(error_value_out_of_range);
    }

    // inject (ldx | ldy) #start opcode node
    const parse_node_ptr loop_initialize = opcode_node(ld_op, I, 1,
        constant_node(start, false));
    expand_node(loop_initialize);

    // save program counter
    const int start_loop_program_counter = program_counter;

    // loop body
    expand_node(p->operands[2]);

    // optimize start = end
    if (start == end) {
        if (--*for_loop_counter != 0) {
            error(error_value_out_of_range);
        }
        return 0;
    }

    // inject inx or dex | iny or dey
    const parse_node_ptr loop_next = opcode_node(increment_flag > 0 ? inc_op : dec_op, i, 0);
    expand_node(loop_next);

    // check to see if we need a compare
    if (end != 0) {
        parse_node_ptr cmp_node;
        if (increment_flag > 0 && end < 255) {
            cmp_node = opcode_node(cmp_op, I, 1, constant_node(end + 1, false));
        }
        else {
            cmp_node = opcode_node(cmp_op, I, 1, constant_node(end, false));
        }
        expand_node(cmp_node);
    }

    // figure out branch condition
    if (increment_flag > 0) {
        if (end < 255) {
            loop_branch = opcode_node(_bcc, r, 1,
                constant_node(start_loop_program_counter, false));
        }
        else {
            loop_branch = opcode_node(_bne, r, 1,
                constant_node(start_loop_program_counter, false));
        }
    }
    else if (increment_flag < 0) {
        loop_branch = opcode_node(_bcs, r, 1, constant_node(start_loop_program_counter, 0));
    }
    expand_node(loop_branch);

    if (--*for_loop_counter != 0) {
        error(error_value_out_of_range);
    }
    return 0;
}

//
// Data node
//
int expand_data_node(const parse_node_ptr p)
{
    origin_specified = true;

    expansion_type = data_bytes;
    data_size = p->data.size;
    if (p->data.size == data_string) {
        expansion_type = data_strings;
        data_size = 0;
    }
    const int result = expand_node(static_cast<parse_node_ptr>(p->data.data));
    return result;
}

//
// Section
//
int expand_operator_section_node(const parse_node_ptr p)
{
    char* name = p->operands[0]->id.name;
    if (current_scope != nullptr) {
        const size_t len = strlen(current_scope) + strlen(name) + 2;
        char* temp_name = static_cast<char*>(MALLOC(len));
        if (temp_name == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }

        sprintf(temp_name, "%s.%s", current_scope, name);
        FREE(current_scope);
        current_scope = temp_name;
    }
    else {
        current_scope = (char*)STRDUP(name);  // NOLINT(clang-diagnostic-incompatible-pointer-types-discards-qualifiers)
        if (current_scope == nullptr) {
            error(error_out_of_memory);
            return 0;
        }
    }

    return 0;
}

/// <summary>
/// end scope.
/// </summary>
/// <param name="p">The parseNode.</param>
/// <returns>0 in all paths</returns>
int expand_operator_end_section_node(parse_node_ptr p)
{
    if (current_scope == nullptr) {
        error(error_end_section_without_section);
        return 0;
    }

    long int index = strlen(current_scope);
    while (index >= 0 && current_scope[index] != '.')
        index--;
    if (index >= 0)
        current_scope[index] = 0;
    else {
        FREE(current_scope);
        current_scope = nullptr;
    }
    return 0;
}

int get_op_byte_count(const parse_node_ptr p)
{
    int byte_count = 0;

    if (p->type != type_op_code)
        return byte_count;

    switch (p->opcode.mode) {
        case A:     /* Accumulator          */
        case i:     /* implied              */
            byte_count = 0;
            break;

        case I:     /* immediate            */
        case r:     /* relative             */
        case zp:    /* zero page            */
        case zpi:   /* zero page indirect   */
        case zpx:   /* zero page x          */
        case zpy:   /* zero page y          */
        case izx:  /* zero page indirect x */
        case izy:  /* zero page indirect y */
            byte_count = 1;
            break;

        case a:     /* absolute             */
        case ax:    /* absolute x           */
        case ay:    /* absolute y           */
        case aix:   /* absolute indirect x  */
        case ind:   /* absolute indirect    */
            byte_count = 2;
            break;

        default:
            error(error_invalid_opcode_or_mode);
            return 0;
    }
    return byte_count;
}

//
// Opcode
//
int expand_op_code_node(const parse_node_ptr p)
{
    int mode_check = p->opcode.mode;
    const int save_mode = p->opcode.mode;
    const int save_op_code = p->opcode.opcode;
    int op_bytes;

    if (program_counter_assigned > 0)
        origin_specified = true;

    p->opcode.program_counter = program_counter;

    switch (p->opcode.mode) {
        default:
        case A:     /* Accumulator          */
        case i:     /* implied              */
            op_bytes = 0;
            break;

        case I:     /* immediate            */
        case zp:    /* zero page            */
        case zpi:   /* zero page indirect   */
        case zpx:   /* zero page x          */
        case zpy:   /* zero page y          */
        case izx:   /* zero page indirect x */
        case izy:   /* zero page indirect y */
            op_bytes = 1;
            break;

        case r:     /* relative             */
            op_bytes = 2;
            break;

        case a:     /* absolute             */
            op_bytes = 2;
            mode_check = zp;
            break;

        case aix:   /* absolute indirect x   */
            op_bytes = 2;
            mode_check = izx;
            break;

        case ax:    /* absolute x           */
            op_bytes = 2;
            mode_check = zpx;
            break;

        case ay:    /* absolute y           */
            op_bytes = 2;
            mode_check = zpy;
            break;

        case ind:   /* absolute indirect    */
            op_bytes = 2;
            mode_check = zpi;
            break;
    }

    if (op_bytes > 0 && !has_uninitialized_symbol(p)) {
        int out_of_range = 0;
        int large_op = 0;
        int code2;
        auto op_count = p->operands.size();
        for (auto& pp: p->operands) {
            int op_value = expand_node(pp);

            // fix for local labels
            const char ch = (pp->type == type_id) ? pp->id.name[0] : 0;
            if (op_value == 0 && p->opcode.mode == r && op_count == 1 && pp->type == type_id
                && (ch == '@' || ch == '-' || ch == '+'))
                op_value = program_counter;

            // ReSharper disable once CppRedundantParentheses
            large_op = large_op | ((op_value & ~0xFF) != 0);

            out_of_range = out_of_range | ((op_value & ~0xFFFF) != 0 || (op_bytes < 2 && large_op));
            if (out_of_range && op_bytes < 2 && (op_value & ~0xFFFF) == 0) {
                switch (p->opcode.mode) {
                    case zp:
                        mode_check = a;
                        break;

                    case zpi:
                        mode_check = ind;
                        break;

                    case zpx:
                        mode_check = ax;
                        break;

                    case zpy:
                        mode_check = ay;
                        break;

                    default:
                        error(error_value_out_of_range);
                        break;
                }
                code2 = get_op_code(p->opcode.instruction, mode_check);
                if (code2 != -1) {
                    p->opcode.mode = mode_check;
                    p->opcode.opcode = code2;
                }
                else {
                    error(error_value_out_of_range);
                }
                break;
            }

            if (p->opcode.mode == r && op_count == 1) {
                // make sure a branch is in range
                const int op = op_value - (p->opcode.program_counter + 2);

                if (op > 128 || op < -127) {
                    // reverse the logic and insert a jmp
                    switch (p->opcode.instruction) {
                        default:
                            break;

                        case _bcs:
                            p->opcode.instruction = _bcc;
                            break;

                        case _bcc:
                            p->opcode.instruction = _bcs;
                            break;

                        case _bne:
                            p->opcode.instruction = _beq;
                            break;

                        case _beq:
                            p->opcode.instruction = _bne;
                            break;

                        case _bmi:
                            p->opcode.instruction = _bpl;
                            break;

                        case _bpl:
                            p->opcode.instruction = _bmi;
                            break;
                    }
                    code2 = get_op_code(p->opcode.instruction, r);
                    if (code2 != -1) {
                        const parse_node_ptr target = pp;
                        const parse_node_ptr jmp = opcode_node(_jmp, a, 1, target);

                        if (target == nullptr || jmp == nullptr) {
                            error(error_out_of_memory);
                            exit(-1);
                        }

                        p->opcode.opcode = code2;
                        if (final_pass) {
                            pp = constant_node(program_counter + 5, 0);
                            generate_list_node(p);
                            generate_output(output_file, p);
                        }
                        program_counter += get_op_code_byte_count(p) + 1;
                        if (final_pass) {
                            jmp->opcode.program_counter = program_counter;
                            generate_list_node(jmp);
                            generate_output(output_file, jmp);

                            warn(error_branch_out_of_range);
                        }
                        program_counter += get_op_code_byte_count(jmp) + 1;
                        p->opcode.opcode = save_op_code;
                        p->opcode.mode = save_mode;

                        return 0;
                    }
                }
            }
        }

        // page zero optimization
        if (!large_op && p->opcode.mode != mode_check) {
            code2 = get_op_code(p->opcode.instruction, mode_check);
            if (code2 != -1) {
                p->opcode.mode = mode_check;
                p->opcode.opcode = code2;
            }
        }
    }

    if (final_pass) {
        generate_list_node(p);
        generate_output(output_file, p);
    }

    program_counter += get_op_code_byte_count(p) + 1;
    code_generated += get_op_code_byte_count(p) + 1;

    p->opcode.opcode = save_op_code;
    p->opcode.mode = save_mode;

    return 0;
}

int expand_print_state_node(const parse_node_ptr p)
{
    if (final_pass) {
        generate_list_node(p);

        if (p->pr.print_state)
            generate_list_node(nullptr);
    }
    return 1;
}

//
// Operator Not
//
int expand_operator_not_node(const parse_node_ptr p)
{
    return !expand_node(p->operands[0]);
}

//
// Operator Shift left
//
int expand_operator_shift_left_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) << expand_node(p->operands[1]);
}

//
// Operator Shift right
//
int expand_operator_shift_right_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) >> expand_node(p->operands[1]);
}

//
// Operator LoByte
//
int expand_operator_lo_byte_node(const parse_node_ptr p)
{
    const int value = expand_node(p->operands[0]) & 0xFFFF;
    const int lo = value & 0xFF;
    return lo;
}

//
// Operator HiByte
//
int expand_operator_hi_byte_node(const parse_node_ptr p)
{
    const int value = expand_node(p->operands[0]) & 0xFFFF;
    const int hi = (value & 0xFF00) >> 8;
    return hi;
}

/// <summary>
/// Operator program_counter assign
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_program_counter_assign_node(const parse_node_ptr p)
{
    program_counter_assigned++;
    const int op = expand_node(p->operands[0]);
    origin_specified = true;

    program_counter = op;
    origin = op;
    return 0;
}

/// <summary>
/// Operator Org
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_org_node(const parse_node_ptr p)
{
    program_counter_assigned++;

    if (org_origin_specified) {
        error(error_org_specified_more_than_once);
        return 0;
    }
    const int op = expand_node(p->operands[0]);
    program_counter = op;
    origin = op;
    origin_specified = true;
    org_origin_specified = true;
    return 0;
}

/// <summary>
/// Operator Expression List
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_expression_list_node(const parse_node_ptr p)
{
    symbol_table_ptr symbol_ptr = nullptr;
    char sym_name[max_macro_param_name_len] = { 0 };

    for (auto& pp: p->operands) {
        if (pp->type == type_opr && pp->opr.opr == EXPRLIST) {
            expand_node(pp);
            continue;
        }

        for (auto& ppp: pp->operands)
            expand_node(ppp);

        switch (expansion_type)  // NOLINT(hicpp-multiway-paths-covered)
        {
            case symbol:
                switch (pp->type)  // NOLINT(clang-diagnostic-switch-enum)
                {
                    case type_id:
                    case type_label:
                        symbol_ptr = pp->id.symbol_ptr;
                        if (symbol_ptr == nullptr) {
                            pp->id.symbol_ptr = add_symbol(pp->id.name);
                            symbol_ptr = pp->id.symbol_ptr;
                            if (symbol_ptr == nullptr) {
                                error(error_adding_symbol);
                                break;
                            }
                            symbol_ptr->is_var = true;
                        }
                        symbol_ptr->value = pp->type == type_label ? program_counter : 0;
                        symbol_ptr->is_initialized = true;
                        break;

                    default:
                        if (symbol_ptr != nullptr) {
                            set_symbol_value(symbol_ptr, expand_node(pp));
                            symbol_ptr = nullptr;
                        }
                        else {
                            error(error_initializing_variable);
                            return 0;
                        }
                        break;
                }
                break;

            case macro_parameter:
                switch (pp->type)  // NOLINT(clang-diagnostic-switch-enum)
                {
                    case type_con:
                    case type_id:
                    case type_label:
                    case type_opr:
                        break;

                    case type_macro_id:
                    case type_macro_ex:
                    case type_data:
                    case type_str:
                    default:
                        error(error_value_out_of_range);
                        return 0;
                }
                if (++macro_parameter_index > max_macro_param)
                    max_macro_param = macro_parameter_index;

                sprintf(sym_name, "\\%d", macro_parameter_index);
                symbol_ptr = add_symbol(sym_name);
                if (symbol_ptr != nullptr) {
                    set_symbol_value(symbol_ptr, expand_node(pp));
                    symbol_ptr->is_initialized = true;
                }
                else {
                    error(error_out_of_memory);
                    exit(-1);
                }
                break;

            case data_bytes:
                if (!has_uninitialized_symbol(pp)) {
                    if (final_pass) {
                        switch (pp->type)  // NOLINT(clang-diagnostic-switch-enum)
                        {
                            case type_con:
                            case type_id:
                            case type_label:
                            case type_opr:
                            case type_str:
                                break;

                            case type_macro_id:
                            case type_macro_ex:
                            case type_data:
                            default:
                                error(error_value_out_of_range);
                                return 0;
                        }
                        generate_list_node(pp);
                        generate_output(output_file, pp);
                    }
                }
                if (pp->type == type_str) {
                    int length = pp->str.len;
                    if (length % 2 && data_size == 2)
                        length++;
                    program_counter += length;
                    break;
                }
                program_counter += data_size;
                break;

            case data_strings:
                if (pp->type != type_str && pp->type != type_con &&
                    pp->type != type_opr && pp->type != type_id &&
                    pp->type != type_label) {
                    error(error_value_out_of_range);
                    return 0;
                }
                if (final_pass) {
                    generate_list_node(pp);
                    generate_output(output_file, pp);
                }
                if (pp->type == type_str) {
                    program_counter += pp->str.len;
                }
                else {
                    const int op = expand_node(pp);
                    const int hi = (op & 0xFF00) >> 8;
                    if (hi != 0)
                        program_counter++;
                    program_counter++;
                }
                break;

            case print_all:
            case print:
                if (pp->type == type_str) {
                    if (final_pass || expansion_type == print_all) {
                        fprintf(console, " ");
                        for (int char_index = 0; char_index < pp->str.len; char_index++) {
                            fprintf(console, "%c", pp->str.value[char_index]);
                        }
                    }
                }
                else {
                    const int op = expand_node(pp);
                    const int hi = (op & 0xFF00) >> 8;
                    const int lo = op & 0xFF;

                    if (final_pass || expansion_type == print_all) {
                        if (hi) {
                            fprintf(console, " $%02X", hi);
                        }
                        fprintf(console, " $%02X", lo);
                    }
                }
                break;
        }
    }
    return 0;
}

/// <summary>
/// Holds the number of times a macro is executed
/// </summary>

/// <summary>
/// Create or return a Macro entry
/// </summary>
static macro_dict_entry* create_macro_entry(std::string name)
{
    auto macro_entry = macro_dict.find(name);

    if (macro_dict.find(name) == macro_dict.end()) {
        auto entry = macro_entry_allocator.allocate(1);
        entry->times_executed = 0;
        macro_dict[name] = entry;
    }

    return macro_dict[name];
}

/// <summary>
/// Reset the Macro dictionary
/// </summary>
void reset_macro_dict(void)
{
    for (const auto& [key, value] : macro_dict) {
        macro_entry_allocator.deallocate(value, 1);
    }
    code_generated = 0;
    macro_dict.clear();
}

/// <summary>
/// Operator Macro Symbol
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_macro_id_node(const parse_node_ptr p)
{
    const macro_dict_entry* macro_dict_entry = create_macro_entry(p->id.name);

    char* temp = last_label;
    sprintf(internal_buffer, "%s::%8.8X", p->id.name, macro_dict_entry->times_executed);
    last_label = const_cast<char*>(STRDUP(internal_buffer));

    expand_id_node(p);

    const symbol_table_ptr sym = p->id.symbol_ptr;
    if (!sym || !sym->macro_node) {
        last_label = temp;
        return 0;
    }
    expand_node(static_cast<parse_node_ptr>(sym->macro_node));

    last_label = temp;

    return sym->value;
}

/// <summary>
/// Operator Macro Expansion (macro call)
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_macro_expansion_node(const parse_node_ptr p)
{
    macro_dict_entry* macro_dict_entry = create_macro_entry(p->macro.name);
    push_macro_params();

    expansion_type = macro_parameter;
    macro_parameter_index = 0;

    if (p->macro.macro_params != nullptr)
        expand_node(static_cast<parse_node_ptr>(p->macro.macro_params));

    char* temp = last_label;
    sprintf(internal_buffer, "%s::%.4X", p->macro.name, macro_dict_entry->times_executed);
    last_label = (char*)STRDUP(internal_buffer);

    if (p->macro.macro != nullptr)
        expand_node(static_cast<parse_node_ptr>(p->macro.macro));

    macro_dict_entry->times_executed++;

    last_label = temp;
    pop_macro_params();

    return 0;
}

/// <summary>
/// Operator Macro definition
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_macro_definition_node(const parse_node_ptr p)
{
    const parse_node_ptr macro_id = p->operands[0];
    // expand_node(macroId);
    create_macro_entry(macro_id->id.name);

    const symbol_table_ptr sym = add_symbol(macro_id->id.name);
    if (sym == nullptr) {
        error(error_out_of_memory);
        exit(-1);
    }

    p->id.symbol_ptr = sym;
    sym->is_macro_name = true;
    sym->is_initialized = true;
    sym->macro_node = p->operands[1];
    sym->value = program_counter;
    return 0;
}

/// <summary>
/// Operator While
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_while_node(const parse_node_ptr p)
{
    while (expand_node(p->operands[0])) {
        expand_node(p->operands[1]);

        if (program_counter > max_address) {
            error(error_infinite_loop_detected);
            return 0;
        }
    }
    return 0;
}

/// <summary>
/// Operator Repeat
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_repeat_node(const parse_node_ptr p)
{
    do {
        expand_node(p->operands[0]);

        if (program_counter > max_address) {
            error(error_infinite_loop_detected);
            return 0;
        }
    } while (expand_node(p->operands[1]) == 0);
    return 0;
}

/// <summary>
/// Operator Do
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_do_node(const parse_node_ptr p)
{
    do {
        expand_node(p->operands[0]);
        if (program_counter > max_address) {
            error(error_infinite_loop_detected);
            return 0;
        }
    } while (expand_node(p->operands[1]));
    return 0;
}

/// <summary>
/// Operator For
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_for_node(const parse_node_ptr p)
{
    symbol_table_ptr start_sym = nullptr;
    int step_val = 1;

    if (p->operands.size() == 5)
        step_val = expand_node(p->operands[4]);

    const parse_node_ptr pp = p->operands[0];
    if (pp->operands[0]->type == type_id) {
        if (pp->operands[0]->id.symbol_ptr == nullptr)
            pp->operands[0]->id.symbol_ptr = add_symbol(pp->operands[0]->id.name);
        start_sym = pp->operands[0]->id.symbol_ptr;
        if (start_sym == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }
    }

    expand_node(p->operands[0]);
    if (p->operands[3]->id.symbol_ptr == nullptr)
        p->operands[3]->id.symbol_ptr = add_symbol(p->operands[3]->id.name);

    const symbol_table_ptr sym = p->operands[3]->id.symbol_ptr;
    if (sym == nullptr) {
        error(error_out_of_memory);
        exit(-1);
    }
    const int end_value = expand_node(p->operands[1]);

    if (start_sym && start_sym != sym) {
        error(error_expected_next);
        return 0;
    }

    if (unroll_loop) {
        do {
            if (step_val > 0 && sym->value > end_value)
                break;
            if (step_val < 0 && sym->value < end_value)
                break;

            expand_node(p->operands[2]);

            if (program_counter > max_address) {
                error(error_infinite_loop_detected);
                return 0;
            }
            set_symbol_value(sym, sym->value + step_val);
        } while (true);
    }
    else {
        error(error_value_out_of_range);
    }
    return 0;
}

/// <summary>
/// Operator If
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_if_node(const parse_node_ptr p)
{
    if (expand_node(p->operands[0])) {
        expand_node(p->operands[1]);
    }
    else if (p->operands.size() > 2) {
        expand_node(p->operands[2]);
    }
    return 0;
}

/// <summary>
/// Operator PrintAll
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_print_all_node(const parse_node_ptr p)
{
    expansion_type = print_all;

    for (auto& op: p->operands) {
        expand_node(op);
    }
    fprintf(console, "\n");
    return 0;
}

/// <summary>
/// Operator Print
/// </summary>
/// <param name="p">The parse_node_ptr p.</param> 
/// <returns>int.</returns>
int expand_operator_print_node(const parse_node_ptr p)
{
    expansion_type = print;
    for (auto& op : p->operands) {
        expand_node(op);
    }
    if (final_pass)
        fprintf(console, "\n");
    return 0;
}

/// <summary>
/// Operator Ds
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_ds_node(const parse_node_ptr p)
{
    const int op = expand_node(p->operands[0]);
    int count = op;
    if (count < 0) {
        error(error_value_out_of_range);
        exit(-1);
    }

    program_counter += count > 0 ? count : 1;

    return op;
}

/// <summary>
/// Operator Statement
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_statement_node(const parse_node_ptr p)
{
    for (auto& op: p->operands) {
        expand_node(op);
    }
    return 0;
}

/// <summary>
/// Operator end_expansion
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_end_node(parse_node_ptr p)
{
    end_expansion = -1;
    return 0;
}

/// <summary>
/// Operator Equate
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_equ_node(const parse_node_ptr p)
{
    const int op = expand_node(p->operands[1]);

    if (stricmp(p->operands[0]->id.name, "-") == 0) {
        int index = find_minus_symbol_definition(current_file_name, yylineno);
        if (index < 0) add_minus_symbol(current_file_name, yylineno, op);

        index = find_minus_symbol(strlen(p->operands[0]->id.name), current_file_name, yylineno);

        if (index >= 0) {
            if (minus_symbol_table[index].value != op) {
                minus_symbol_table[index].value = op;
                sym_value_changed++;
            }
        }
        return op;
    }

    if (stricmp(p->operands[0]->id.name, "+") == 0) {
        int index = find_plus_symbol_definition(current_file_name, yylineno);
        if (index < 0) add_plus_symbol(current_file_name, yylineno, op);

        index = find_plus_symbol(strlen(p->operands[0]->id.name), current_file_name, yylineno - 1);

        if (index >= 0) {
            if (plus_symbol_table[index].value != op) {
                plus_symbol_table[index].value = op;
                sym_value_changed++;
            }
        }
        return op;
    }

    if (p->operands[0]->id.symbol_ptr == nullptr || p->operands[0]->id.name[0] == '@')
        p->operands[0]->id.symbol_ptr = add_symbol(p->operands[0]->id.name);
    symbol_table_ptr sym = p->operands[0]->id.symbol_ptr;
    if (sym == nullptr) {
        error(error_out_of_memory);
        exit(-1);
    }

    if (!sym->is_initialized || sym->value != op) {
        if (!sym->scope && current_scope) {
            const int len = strlen(sym->name) + strlen(current_scope) + 2;
            char* temp = static_cast<char*>(MALLOC(len));
            if (temp == nullptr) {
                error(error_out_of_memory);
                exit(-1);
            }

            sprintf(temp, "%s.%s", current_scope, sym->name);
            sym = add_symbol(temp);
            FREE(temp);
        }
        set_symbol_value(sym, op);
        sym->is_initialized = true;
        if (p->operands[0]->type == type_label)
            sym->is_label = true;
    }
    return op;
}

/// <summary>
/// Operator UMinus
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_uminus_node(const parse_node_ptr p)
{
    return -expand_node(p->operands[0]);
}

/// <summary>
/// Operator Ones Complement
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_ones_complement_node(const parse_node_ptr p)
{
    int mask = 0xFF;
    const int v = expand_node(p->operands[0]);
    if (v & ~0xFF)
        mask = 0xFFFF;
    return ~v & mask;
}

/// <summary>
/// Operator Plus
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_plus_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) + expand_node(p->operands[1]);
}

/// <summary>
/// Operator Minus
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_minus_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) - expand_node(p->operands[1]);
}

/// <summary>
/// Operator Multiply
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_multiply_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) * expand_node(p->operands[1]);
}

/// <summary>
/// Operator Divide
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_divide_node(const parse_node_ptr p)
{
    if (expand_node(p->operands[1]) == 0) {
        error(error_divide_by_zero);
        return 0;
    }
    return expand_node(p->operands[0]) / expand_node(p->operands[1]);
}

/// <summary>
/// Operator BitOr
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_bit_or_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) | expand_node(p->operands[1]);
}

/// <summary>
/// Operator BitAnd
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_bit_and_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) & expand_node(p->operands[1]);
}

/// <summary>
/// Operator XOR
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_xor_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) ^ expand_node(p->operands[1]);
}

/// <summary>
/// Operator LessThan
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_less_than_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) < expand_node(p->operands[1]);
}

/// <summary>
/// Operator GreaterThan
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_greater_than_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) > expand_node(p->operands[1]);
}

/// <summary>
/// Operator Or
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_or_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) || expand_node(p->operands[1]); // intentional logical or NOT BIT OR!
}

/// <summary>
/// Operator And
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_and_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) && expand_node(p->operands[1]); // intentional logical and NOT BIT AND!
}

/// <summary>
/// Operator Equals
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_equal_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) == expand_node(p->operands[1]);
}

/// <summary>
/// Operator Not Equal
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_not_equal_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) != expand_node(p->operands[1]);
}

/// <summary>
/// Operator Greater Than Or Equal
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_greater_than_or_equal_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) >= expand_node(p->operands[1]);
}

/// <summary>
/// Operator LessThan Or Equal
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_less_than_or_equal_node(const parse_node_ptr p)
{
    return expand_node(p->operands[0]) <= expand_node(p->operands[1]);
}

/// <summary>
/// Expand an operator
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_operator_node(const parse_node_ptr p)
{
    if (expand_operator_table_dictionary.find(p->opr.opr) == expand_operator_table_dictionary.end()) {
        error(error_unknown_operator_type);
        return 0;
    }
    auto fun = expand_operator_table_dictionary[p->opr.opr];
    return (*fun)(p);
}

/// <summary>
/// Expands parse_node_ptr p
/// </summary>
/// <param name="p">The parse_node_ptr p.</param>
/// <returns>int.</returns>
int expand_node(const parse_node_ptr p)
{
    if (p == nullptr) {
        return 0;
    }
    expand_level++;

    if (end_expansion) {
        expand_level--;
        return 0;
    }

    if (expand_table_dictionary.find(p->type) == expand_table_dictionary.end()) {
        error(error_unknown_operator_type);
        return 0;
    }

    auto fun = expand_table_dictionary[p->type];
    const int result = (*fun)(p);
    expand_level--;

    return result;
}

/// <summary>
/// Determines if p is an is_initialized symbol.
/// </summary>
/// <param name="p">The node pointer.</param>
int is_uninitialized_symbol(const parse_node_ptr p)
{
    if (p->type == type_label || p->type == type_id) {
        if (p->id.symbol_ptr == nullptr)
            return true;
        const symbol_table_ptr sym = p->id.symbol_ptr;
        if (sym->is_macro_param || sym->is_macro_name)
            return false;

        return !sym->is_initialized;
    }
    return false;
}

/// <summary>
/// Determines p has uninitialized symbol
/// </summary>
/// <param name="p">The p.</param>
bool has_uninitialized_symbol(const parse_node_ptr p)
{
    for (auto& op: p->operands) {
        if (has_uninitialized_symbol(op) || is_uninitialized_symbol(op))
            return true;
    }
    return false;
}

void dump_op_table(void)
{
    for (const auto& [key, value] : expand_operator_table_dictionary) {
        fprintf(console, "%d\n", key);
        fprintf(console, "%p\n", value);
    }
}

void dump_table_dictionary(void)
{
    for (const auto& [key, value] : expand_table_dictionary) {
        fprintf(console, "%d\n", key);
        fprintf(console, "%p\n", value);
    }
}
