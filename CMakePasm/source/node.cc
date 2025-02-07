// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppClangTidyCertErr33C
// ReSharper disable CppClangTidyClangDiagnosticDeprecatedDeclarations
// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyClangDiagnosticImplicitInt
// ReSharper disable CppClangTidyClangDiagnosticIntConversion
// ReSharper disable CppPointerConversionDropsQualifiers
// ReSharper disable CppClangTidyClangDiagnosticIncompatiblePointerTypesDiscardsQualifiers
#pragma warning(disable : 4996 4090)
#include <map>
#include <ctype.h>
#include <fstream>
#include <iomanip>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "node.h"
#include "error.h"
#include "expand.h"
#include "flex.h"
#include "genlist.h"
#include "memory.h"
#include "opcodes.h"
#include "pasm.h"
#include "pasm.tab.h"
#include "str.h"
#include "sym.h"

#define MIN(a,b)    ((a)<(b)?(a):(b))
#define HIBYTE(a)   (((a) & 0xFF00) >> 8)
#define LOBYTE(a)   ((a) & 0xFF)

void print_tab(std::stringstream& file, int tab);
static std::allocator<parse_node> parsenode_allocator;

static int is_name_valid(char* name);

std::vector<parse_node_ptr> parse_nodes;

/// <summary>
/// The print nest level
/// </summary>
int print_nest_level = 0;

std::map<int, std::string> op_lookup =
{
    { INC,          "INC"           },
    { LOAD,         "LOAD"          },
    { LOBYTE,       "LOBYTE"        },
    { HIBYTE,       "HIBYTE"        },
    { PCASSIGN,     "PCASSIGN"      },
    { ORG,          "ORG"           },
    { EXPRLIST,     "EXPRLIST"      },
    { MACRO,        "MACRO"         },
    { WHILE,        "WHILE"         },
    { REPEAT,       "REPEAT"        },
    { SECTION,      "SECTION"       },
    { ENDSECTION,   "ENDSECTION"    },
    { DO,           "DO"            },
    { FOR,          "FOR"           },
    { REGX,         "REGX"          },
    { REGY,         "REGY"          },
    { IF,           "IF"            },
    { PRINT,        "PRINT"         },
    { STATEMENT,    "STATEMENT"     },
    { END,          "END"           },
    { UMINUS,       "UMINUS"        },
    { BIT_OR,       "BIT_OR"        },
    { BIT_AND,      "BIT_AND"       },
    { EQ,           "EQ"            },
    { NE,           "NE"            },
    { GE,           "GE"            },
    { LE,           "LE"            },
    { OR,           "OR"            },
    { AND,          "AND"           },
    { NOT,          "NOT"           },
    { SHIFT_LEFT,   "SHIFT_LEFT"    },
    { SHIFT_RIGHT,  "SHIFT_RIGHT"   },
    { '=',          "'='"           },
    { '~',          "'~'"           },
    { '+',          "'+'"           },
    { '-',          "'-'"           },
    { '/',          "'/'"           },
    { '^',          "'^'"           },
    { '<',          "'<'"           },
    { '>',          "'>'"           },
};

inline void print_byte(std::stringstream& str_stream, int op)
{
    str_stream << "$" <<
        std::uppercase << std::right << std::hex << std::setw(2) << std::setfill('0') << LOBYTE(op);
}

inline void print_2byte(std::stringstream& str_stream, int op)
{
    str_stream << "$" <<
        std::uppercase << std::right << std::hex << std::setw(4) << std::setfill('0') << (op & 0xFFFF);
}

inline void print_word(std::stringstream& str_stream, int op)
{
    str_stream << "$" <<
        std::uppercase << std::right << std::hex << std::setw(2) << std::setfill('0') << HIBYTE(op) <<
        std::uppercase << std::right << std::hex << std::setw(2) << std::setfill('0') << LOBYTE(op);
}

/// <summary>
/// Allocates the node.
/// </summary>
/// <returns>parseNode *.</returns>
parse_node_ptr allocate_node()
{
    auto p = parsenode_allocator.allocate(1);
    if (p == nullptr) {
        error(error_out_of_memory);
        exit(1);  // NOLINT(concurrency-mt-unsafe)
    }
    memset(p, 0, sizeof(parse_node));

    parse_nodes.push_back(p);
    return p;
}

/// <summary>
/// adds a constant value
/// </summary>
/// <param name="value">The value.</param>
/// <param name="is_program_counter"></param>
/// <returns>parseNode *.</returns>
parse_node_ptr constant_node(const int value, const int is_program_counter)
{
    const parse_node_ptr p = allocate_node();
    p->type = type_con;

    /* copy information */
    p->con.value = value;
    p->con.is_program_counter = is_program_counter;
    if (p->con.is_program_counter)
        p->con.value = program_counter;

    return p;
}

//
// create a string node
//
parse_node_ptr string_node(const char* value)
{
    const parse_node_ptr p = allocate_node();

    char* str = const_cast<char*>(STRDUP(value));
    if (str == nullptr) {
        error(error_out_of_memory);
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }

    p->type = type_str;
    p->str.allocated = str;
    p->str.len = static_cast<int>(strlen(str));
    if (*str == '\'' || *str == '"') {
        str[p->str.len - 1] = 0;
        str++;
    }

    /* copy information */
    if (p->str.value)
        FREE(p->str.value);

    p->str.value = sanitize_string(str);
    if (p->str.value == nullptr) {
        error(error_out_of_memory);
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }
    p->str.len = static_cast<int>(strlen(p->str.value));

    return p;
}

parse_node_ptr id_node_common(const char* name, const node_type_enum  node_type)
{
    /* MALLOC node */
    // ReSharper disable once CppLocalVariableMayBeConst
    parse_node_ptr p = allocate_node();

    /* copy information */
    p->type = node_type;
    p->id.name = const_cast<char*>(STRDUP(name));
    if (p->id.name == nullptr) {
        error(error_out_of_memory);
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }
    return p;
}

/// <summary>
/// Identifier specified name.
/// </summary>
/// <param name="name">The name of symbol.</param>
/// <returns>parseNodePtr.</returns>
parse_node_ptr id_node(const char* name)
{
    return id_node_common(name, type_id);
}

/// <summary>
/// Identifier specified name.
/// </summary>
/// <param name="name">The name of symbol.</param>
/// <returns>parseNodePtr.</returns>
parse_node_ptr label_node(char* name)
{
    if (name[0] != '@' && name[0] != '-' && name[0] != '+')
        last_label = name;
    else if (last_label != nullptr && name[0] == '@') {
        return id_node_common(format_local_sym(name, last_label), type_label);
    }
    return id_node_common(name, type_label);
}

/// <summary>
/// Macro Identifier specified name.
/// </summary>
/// <param name="name">The id of symbol.</param>
/// <returns>parseNodePtr.</returns>
parse_node_ptr macro_id_node(const char* name)
{
    return id_node_common(name, type_macro_id);
}

parse_node_ptr print_state_node(const int op)
{
    const parse_node_ptr p = allocate_node();
    p->type = type_print;
    p->pr.print_state = op;

    return p;
}

/// <summary>
/// Macro execution node
/// </summary>
/// <param name="name">macro to execute.</param>
/// <param name="macro_params">macro parameters.</param>
/// <returns>parseNodePtr.</returns>
parse_node_ptr macro_expand_node(const char* name, const parse_node_ptr macro_params)
{
    // Must be done first
    const parse_node_ptr macro_node_ptr = macro_id_node(name);

    /* MALLOC node */
    const parse_node_ptr p = allocate_node();

    /* copy information */
    p->type = type_macro_ex;
    p->macro.macro = macro_node_ptr;
    p->macro.name = const_cast<char*>(STRDUP(name));
    p->macro.macro_params = macro_params;

    return p;
}

/// <summary>
/// adds a data node BYTE, WORD, STRING
/// </summary>
/// <param name="data_node_size">size of data.</param>
/// <param name="data">data.</param>
/// <returns>parseNodePtr.</returns>
parse_node_ptr data_node(const int data_node_size, const parse_node_ptr data)
{
    /* MALLOC node */
    const parse_node_ptr p = allocate_node();

    /* copy information */
    p->type = type_data;
    p->data.size = data_node_size;
    p->data.data = data;

    return p;
}

/// <summary>
/// Operator.
/// </summary>
/// <param name="opr">The operator.</param>
/// <param name="number_of_ops">The number of ops.</param>
/// <param name="...">...</param>
/// <returns>parseNodePtr.</returns>
parse_node_ptr operator_node(const int opr, const int number_of_ops, ...)
{
    // ReSharper disable once CppTooWideScope 
    va_list ap;
    parse_node_ptr p = allocate_node();

    /* copy information */
    p->type = type_opr;
    p->opr.opr = opr;

    if (number_of_ops > 0) {
        va_start(ap, number_of_ops);
        for (int i = 0; i < number_of_ops; i++)
            (p->operands).push_back(va_arg(ap, parse_node_ptr));
        va_end(ap);
    }

    return p;
}

/// <summary>
/// Opcodes
/// </summary>
/// <param name="op">The operator.</param>
/// <param name="mode">The mode.</param>
/// <param name="number_of_ops">The number of ops.</param>
/// <param name="..."></param>
/// <returns>parseNode *.</returns>
parse_node_ptr opcode_node(const int op, int mode, const int number_of_ops, ...)
{
    va_list ap;
    int index;

    const parse_node_ptr p = allocate_node();

    p->type = type_op_code;

    // a branch will look like absolute addressing so we check to
    // see if it really relative
    if (get_op_code(op, r) != -1) {
        mode = r;
    }

    /* copy information */
    p->opcode.mode = mode;
    p->opcode.instruction = op;

    int val = 0;
    if (number_of_ops > 0) {
        va_start(ap, number_of_ops);
        for (index = 0; index < number_of_ops; index++) {
            p->operands.push_back(va_arg(ap, parse_node_ptr));
            val = expand_node(p->operands[index]);
        }
        va_end(ap);
    }
    const bool has_uninitialized = has_uninitialized_symbol(p);

    /* Take care of ASL A   etc */
    if (p->opcode.mode == a && number_of_ops > 0) {
        const parse_node_ptr pp = p->operands[0];
        // ReSharper disable once CppDeprecatedEntity
        if (pp && pp->type == type_id && stricmp(pp->id.name, "A") == 0) {
            mode = A;
            // p->operands.pop_back();
            p->opcode.mode = mode;
        }
    }

    int code = get_op_code(op, mode);
    int new_mode = mode;
    if (((val & ~0xFF) == 0 && has_uninitialized == false) || code == -1) {
        switch (mode) {
            case r:     /* relative             */
            case A:     /* Accumulator          */
            case I:     /* immediate            */
            case i:     /* implied              */
            case zp:    /* zero page            */
            case zpi:   /* zero page indirect   */
            case zpx:   /* zero page x          */
            case zpy:   /* zero page y          */
            case izx:   /* zero page indirect x */
            case izy:   /* zero page indirect y */
                break;

            case a:     /* absolute             */
                new_mode = zp;
                break;

            case aix:   /* absolute indirect x  */
                new_mode = izx;
                break;

            case ax:    /* absolute x           */
                new_mode = zpx;
                break;

            case ay:    /* absolute y           */
                new_mode = zpy;
                break;

            case ind:   /* absolute indirect    */
                new_mode = zpi;
                break;

            default:
                break;
        }
    }
    if (new_mode != mode) {
        const int temp_code = get_op_code(op, new_mode);
        if (temp_code != -1) {
            p->opcode.mode = new_mode;
            code = temp_code;
        }
    }

    for (auto& op : p->operands) {
        expand_node(op);
    }

    p->opcode.opcode = code;
    if (code < 0) {
        error(error_invalid_opcode_or_mode);
    }

    return p;
}

// This will not nodes from remove from p->ops.
// The problem is an entry in p->ops is also in the first level of vector
// That means if it freed it must be done
// in all places in the tree at once or it will cause a crash.
// There may be multiple entries for the same node
void remove_parse_node(const parse_node_ptr p)
{
    if (p == nullptr)
        return;

    for (auto it = parse_nodes.begin(); it != parse_nodes.end(); ) {
        if (*it == p) {
            it = parse_nodes.erase(it);  // Remove element and update iterator
            break;
        }
        else {
            ++it;  // Increment iterator only if not erasing
        }
    }

    free_parse_node(p);
}

/// <summary>
/// Frees the tree.
/// </summary>
void free_parse_tree(void)
{
    while (parse_nodes.size() > 0) {
        remove_parse_node(parse_nodes.front());
    }
}

//
// Print indent
//
void print_indent(std::stringstream& file)
{
    print_tab(file, print_nest_level);
}

void print_tab(std::stringstream& file, int tab)
{
    file << std::setw(tab * 4) << std::setfill(' ') << "";
}

int is_valid_parse_tree()
{
    for (auto& node : parse_nodes) {
        if (!is_valid_parse_node(node))
            return 0;
    }
    return ~0;
}

int is_valid_parse_node(const parse_node_ptr p)
{
    if (p == nullptr) return 0;

    if (p->type <= type_unknown || p->type >= type_last)
        return 0;

    for (auto& op : p->operands) {
        if (!is_valid_parse_node(op))
            return 0;
    }

    return ~0;
}

// 
// FREE a node
//
void free_parse_node(const parse_node_ptr p)
{
    switch (p->type) {
        case type_macro_id:
        case type_id:
            if (p->id.name != nullptr && is_name_valid(p->id.name)) {
                FREE(p->id.name);
                p->id.name = nullptr;
            }
            break;

        case type_str:
            FREE(p->str.value);
            p->str.value = nullptr;
            break;

        case type_head_node:
        case type_con:
        case type_label:
        case type_macro_ex:
        case type_opr:
        case type_op_code:
        case type_data:
        case type_unknown:
        case type_print:
        case type_last:
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            break;

    }
    p->operands.clear();
    parsenode_allocator.deallocate(p, 1);
}

int is_name_valid(char* name)
{
    if (name == nullptr) return 0;

    for (; *name; name++) {
        const char c = *name;
        if ((c != '+' && c != '-' && c != '@') &&
            ((c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                c == '_' || c == '\\'))
            continue;

        return 0;
    }
    return 1;
}

inline void print_label(std::stringstream& str_stream, const char* label)
{
    const int col1_label_width = 15;
    str_stream << std::left << std::setfill(' ') << std::setw(col1_label_width) << label;
}

inline void print_value(std::stringstream& str_stream, const char* value)
{
    str_stream << std::left << std::setfill(' ') << std::setw(0) << value;
}

inline void print_value(std::stringstream& str_stream, bool value)
{
    str_stream << std::left << std::setfill(' ') << std::setw(0) << value;
}

inline void print_hex_value(std::stringstream& str_stream, void* value)
{
    str_stream << std::left << std::setw(0) << std::setfill(' ') << "" <<
        "0x" << std::right << std::setfill('0') << std::setw(8) << std::uppercase << std::hex << value;
}

inline void print_hex_value(std::stringstream& str_stream, int value)
{
    print_2byte(str_stream, value);
}

inline void print_char(std::stringstream& str_stream, char op)
{
    if (isprint(op)) {
        str_stream << "'" << op << "'";
    }
    else {
        print_byte(str_stream, op);
    }
}

void print_node(parse_node_ptr p, std::stringstream& str_stream)
{
    print_indent(str_stream);
    if (print_nest_level > 0)
        str_stream << "CHILD ";
    str_stream << "NODE [line " << yylineno << "]" << std::endl;
    print_nest_level++;

    print_tab(str_stream, print_nest_level + 1);
    print_label(str_stream, "type");

    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    // ReSharper disable once CppIncompleteSwitchStatement
    switch (p->type)  // NOLINT(clang-diagnostic-switch)
    {
        case type_unknown:
            print_value(str_stream, "unknown");
            str_stream << std::endl;
            break;

        case type_id:
        case type_macro_id:
        case type_label:
            switch (p->type)  // NOLINT(clang-diagnostic-switch-enum)
            {
                case type_id:
                    print_value(str_stream, "type_id");
                    break;

                case type_label:
                    print_value(str_stream, "type_label");
                    break;

                case type_macro_id:
                    print_value(str_stream, "type_macro_id");
                    break;

            }
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "name");
            print_value(str_stream, p->id.name);
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "symbol_ptr");

            if (p->id.symbol_ptr == nullptr) {
                print_value(str_stream, "(nil)");
            }
            else {
                print_hex_value(str_stream, p->id.symbol_ptr);
            }
            str_stream << std::endl;

            if (p->id.symbol_ptr) {
                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "fullname");
                print_value(str_stream, p->id.symbol_ptr->fullname);
                str_stream << std::endl;

                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "is_initialized");
                print_value(str_stream, (p->id.symbol_ptr)->is_initialized);
                str_stream << std::endl;

                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "value");
                print_value(str_stream, p->id.symbol_ptr->value);
                str_stream << std::endl;

                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "ismacroname");
                print_value(str_stream, p->id.symbol_ptr->is_macro_name);
                str_stream << std::endl;

                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "ismacroparam");
                print_value(str_stream, p->id.symbol_ptr->is_macro_param);
                str_stream << std::endl;

                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "isplusminus");
                print_value(str_stream, p->id.symbol_ptr->is_plus_minus);
                str_stream << std::endl;

                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "isvar");
                print_value(str_stream, p->id.symbol_ptr->is_var);
                str_stream << std::endl;

                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "macroNode");
                print_hex_value(str_stream, p->id.symbol_ptr->macro_node);
                str_stream << std::endl;

                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "scope");
                print_value(str_stream, p->id.symbol_ptr->scope ? p->id.symbol_ptr->scope : "NULL");
                str_stream << std::endl;

                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "name");
                print_value(str_stream, p->id.symbol_ptr->name);
                str_stream << std::endl;
            }
            break;

        case type_macro_ex:
            str_stream << std::endl;
            break;

        case type_opr:
            print_value(str_stream, "typeOpr");
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "opr");
            print_value(str_stream, op_lookup[p->opr.opr].c_str());
            str_stream << std::endl;
            break;

        case type_op_code:
            print_value(str_stream, "typeOpCode");
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "instruction");
            print_value(str_stream, instruction_to_string(p->opcode.instruction));
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "mode");
            print_value(str_stream, mode_to_string(p->opcode.mode));
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "opCode");
            print_byte(str_stream, p->opcode.opcode);
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "PC");
            print_value(str_stream, p->opcode.program_counter);
            str_stream << std::endl;
            break;

        case type_con:
            print_value(str_stream, "typeCon");
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "IsPC");
            print_value(str_stream, p->con.is_program_counter);
            str_stream << std::endl;

            if (p->con.is_program_counter) {
                print_tab(str_stream, print_nest_level + 1);
                print_label(str_stream, "PC");
                print_hex_value(str_stream, program_counter);
                str_stream << std::endl;
            }

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "value");
            print_hex_value(str_stream, p->con.value);
            str_stream << std::endl;
            break;

        case type_data:
            print_value(str_stream, "typeData");
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "size");
            print_hex_value(str_stream, p->data.size);
            str_stream << std::endl;
            print_node(static_cast<parse_node_ptr>(p->data.data), str_stream);
            break;

        case type_str:
            print_value(str_stream, "typeStr");
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "allocated");
            print_value(str_stream, p->str.allocated);
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "len");
            print_hex_value(str_stream, p->str.len);
            str_stream << std::endl;

            print_tab(str_stream, print_nest_level + 1);
            print_label(str_stream, "value");
            print_value(str_stream, p->str.value);
            str_stream << std::endl;
            break;
    }
    for (auto& op : p->operands)
        print_node(op, str_stream);
    print_nest_level--;
}

//
// Print a node
//
void print_node(parse_node_ptr p, std::ostream& file)
{
    std::stringstream str_stream;
    print_node(p, str_stream);
    file << str_stream.str();
}

void print_node(parse_node_ptr p, std::ofstream& file)
{
    std::stringstream str_stream;
    print_node(p, str_stream);
    file << str_stream.str();
}

// 
// Generate a list node entry
// based on node
//
// This should only be called on the FinalPass
// when all symbols/macros are resolved
//
// return negative value on an error
// return 0 on success
int generate_list_node(const parse_node_ptr p)
{
    int bytes = 0;
    int op = 0;
    int output_length = 0;
    int len = 0;
    char* string_start = nullptr;
    char* str = nullptr;
    unsigned char hi = 0;
    unsigned char lo = 0;
    int pc = program_counter;
    std::stringstream str_stream;

    // passing NULL will cause an empty node which forces a the
    // current file to listed even if no code is generated
    if (p == nullptr) {
        if (add_list(current_file_name, yylineno, "") == nullptr) {
            error(error_creating_list_node);
            return -1;
        }
        return 0;
    }

    switch (p->type) {
        // raw bytes
        default:
            print_word(str_stream, pc & 0xFFFF);
            str_stream << ": ";
            // expand the node
            op = expand_node(p);
            hi = HIBYTE(op);
            lo = LOBYTE(op);

            if (op < 0 && data_size == 1) {
                hi = 0;
            }
            // check for overflow
            if (hi != 0 && data_size < 2) {
                // This error is reported in generate_output
                // error(error_value_out_of_range);
                return -1;
            }

            // output bytes
            print_byte(str_stream, lo);
            if (data_size > 1) {
                str_stream << ' ';
                print_byte(str_stream, hi);
            }
            // padd to next field
            len = SRC_LST_INDENT - static_cast<int>(str_stream.str().size()) + 4;
            str_stream << std::setw(len) << std::setfill(' ') << ".db ";

            // add .db bytes
            print_byte(str_stream, lo);
            if (data_size > 1) {
                str_stream << ", ";
                print_byte(str_stream, hi);
            }
            // create the list node
            if (add_list(current_file_name, yylineno, str_stream.str().c_str()) == nullptr) {
                error(error_creating_list_node);
                return -1;
            }
            break;

        case type_print:
            if (p->pr.print_state != print_list_state) {
                print_list_state = false;
                auto node = add_list(current_file_name, yylineno, "");
                if (node == nullptr) {
                    error(error_creating_list_node);
                    return -1;
                }
                node->list_directive = true;
                print_list_state = p->pr.print_state;
            }
            return 1;

        case type_str:
            str = p->str.value;
            output_length = p->str.len;
            if (data_size == 2 && output_length % 2 != 0) {
                output_length++;
            }
            while (output_length) {
                str_stream.str("");
                string_start = str;
                print_word(str_stream, pc & 0xFFFF);
                str_stream << ":";
                bytes = MIN(3, output_length);
                output_length -= bytes;
                pc += bytes;

                for (auto i = 0; i < bytes; ++i) {
                    str_stream << ' ';
                    print_byte(str_stream, static_cast<int>(*str++) & 0xFF);
                }

                len = SRC_LST_INDENT - static_cast<int>(str_stream.str().size()) + 4;
                str_stream << std::setw(len) << std::setfill(' ') << ".db ";

                print_char(str_stream, string_start[0]);
                for (len = 1; len < bytes; len++) {
                    str_stream << ", ";
                    print_char(str_stream, string_start[len]);
                }
                if (add_list(current_file_name, yylineno, str_stream.str().c_str()) == nullptr) {
                    error(error_creating_list_node);
                    return -1;
                }
            }
            break;

        case type_op_code:
            print_word(str_stream, p->opcode.program_counter);
            str_stream << ": ";

            print_byte(str_stream, p->opcode.opcode);

            // get number of bytes used by opcode
            // must be 0, 1 or 2
            bytes = get_op_byte_count(p);
            if (bytes > 0) {
                op = expand_node(p->operands[0]);

                // if relative mode check range
                if (p->opcode.mode == r) {
                    op -= (p->opcode.program_counter + 2);
                    if (op > 128 || op < -127) {
                        // This error is reported in generate_output
                        error(error_branch_out_of_range);
                        return -1;
                    }
                    op &= 0xFF;
                }
                // get the hi and lo byte
                hi = HIBYTE(op);
                lo = LOBYTE(op);

                // output the extra bytes
                str_stream << ' ';
                print_byte(str_stream, lo);
                if (bytes > 1) {
                    str_stream << ' ';
                    print_byte(str_stream, hi);
                }
            }

            // padd to next field
            len = SRC_LST_INDENT - static_cast<int>(str_stream.str().size()) + 3;
            str_stream << std::setw(len) << std::setfill(' ');

            str_stream << instruction_to_string(p->opcode.instruction);
            if (bytes > 0 && (p->operands.size() > 0)) {
                str_stream << ' ';
                for (auto& operand : p->operands) {
                    expand_node(operand);
                }
                op = expand_node(p->operands[0]);

                switch (p->opcode.mode) {
                    case A:
                    case i:
                    default:
                        break;

                    case I:      // immediate
                        str_stream << '#';
                        print_byte(str_stream, op);
                        break;

                    case zp:     // zero page
                        print_byte(str_stream, op);
                        break;

                    case zpi:    // zero page indirect
                        str_stream << '(';
                        print_byte(str_stream, op);
                        str_stream << ')';
                        break;

                    case zpx:    // zero page x
                        print_byte(str_stream, op);
                        str_stream << ",x";
                        break;

                    case zpy:    // zero page y
                        print_byte(str_stream, op);
                        str_stream << ",y";
                        break;

                    case izx:   // zero page indirect x
                        str_stream << '(';
                        print_byte(str_stream, op);
                        str_stream << ",x)";
                        break;

                    case izy:   // zero page indirect y
                        str_stream << '(';
                        print_byte(str_stream, op);
                        str_stream << "),y";
                        break;

                    case a:      // absolute
                        print_2byte(str_stream, op);
                        break;

                    case aix:    // absolute indirect x
                        str_stream << '(';
                        print_2byte(str_stream, op);
                        str_stream << ",x)";
                        break;

                    case ax:     // absolute x
                        print_2byte(str_stream, op);
                        str_stream << ",x";
                        break;

                    case ay:     // absolute y
                        print_2byte(str_stream, op);
                        str_stream << ",y";
                        break;

                    case ind:    // absolute indirect
                        str_stream << '(';
                        print_2byte(str_stream, op);
                        str_stream << ')';
                        break;

                    case r:      // relative
                        print_2byte(str_stream, op);
                        break;

                }
            }
            if (add_list(current_file_name, yylineno, str_stream.str().c_str()) == nullptr) {
                error(error_creating_list_node);
                return -1;
            }

            break;
    }
    return 0;
}
