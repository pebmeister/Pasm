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
#include "node.h"

#include <ctype.h>
#include <fstream>
#include <iomanip>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

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

static int is_name_valid(char* name);

/// <summary>
/// The head node
/// </summary>
parse_node_ptr head_node = nullptr;

/// <summary>
/// The current node
/// </summary>
parse_node_ptr current_node = nullptr;

/// <summary>
/// The print nest level
/// </summary>
int print_nest_level = 0;

/// <summary>
/// Allocates the node.
/// </summary>
/// <returns>parseNode *.</returns>
parse_node_ptr allocate_node(const int number_of_ops)
{
    parse_node_ptr p;
    size_t size = sizeof(parse_node);
    if ((p = static_cast<parse_node_ptr>(MALLOC(size))) == nullptr) {
        error(error_out_of_memory);
        exit(1);  // NOLINT(concurrency-mt-unsafe)
    }
    memset(p, 0, size);

    if (current_node != nullptr) {
        p->prev = current_node;
        current_node->next = p;
    }
    current_node = p;
    p->number_of_ops = number_of_ops;
    p->allocated = true;
    if (number_of_ops > 0) {
        size = number_of_ops * sizeof(parse_node_ptr);
        if ((p->op = static_cast<parse_node**>(MALLOC(size))) == nullptr) {
            error(error_out_of_memory);
            exit(-1);  // NOLINT(concurrency-mt-unsafe)
        }
        memset(p->op, 0, size);
    }
    if (head_node == nullptr) {
        head_node = p;
    }
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
    const parse_node_ptr p = allocate_node(0);
    if (p == nullptr) {
        error(error_out_of_memory);
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }
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
    const parse_node_ptr p = allocate_node(0);

    char* str = const_cast<char*>(STRDUP(value));

    if (str == nullptr || p == nullptr) {
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
    parse_node_ptr p = allocate_node(0);

    if (p == nullptr) {
        error(error_out_of_memory);
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }

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
    const parse_node_ptr p = allocate_node(0);
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
    const parse_node_ptr p = allocate_node(0);

    if (p == nullptr || macro_node_ptr == nullptr) {
        error(error_out_of_memory);
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }

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
    const parse_node_ptr p = allocate_node(0);

    if (p == nullptr) {
        error(error_out_of_memory);
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }

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

    /* MALLOC node */
    const parse_node_ptr p = allocate_node(number_of_ops);
    if (p == nullptr) {
        error(error_out_of_memory);
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }

    /* copy information */
    p->type = type_opr;
    p->opr.opr = opr;

    if (number_of_ops > 0) {
        va_start(ap, number_of_ops);
        for (int i = 0; i < number_of_ops; i++)
            p->op[i] = va_arg(ap, parse_node_ptr);
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

    const parse_node_ptr p = allocate_node(number_of_ops);

    if (p == nullptr) {
        error(error_out_of_memory);
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }

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
            p->op[index] = va_arg(ap, parse_node_ptr);
            val = expand_node(p->op[index]);
        }
        va_end(ap);
    }
    const bool has_uninitialized = has_uninitialized_symbol(p);


    /* Take care of ASL A   etc */
    if (p->opcode.mode == a && number_of_ops > 0) {
        const parse_node_ptr pp = p->op[0];
        // ReSharper disable once CppDeprecatedEntity
        if (pp && pp->type == type_id && stricmp(pp->id.name, "A") == 0) {
            mode = A;
            p->number_of_ops--;
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

    for (index = 0; index < number_of_ops; index++) {
        if (p->op[index])
            expand_node(p->op[index]);
    }

    p->opcode.opcode = code;
    if (code < 0) {
        error(error_invalid_opcode_or_mode);
    }

    return p;
}


// This will not nodes from remove from p->ops.
// The problem is an entry in p->ops is also in the first level op tree
// That means if it freed it must be done
// in all places in the tree at once or it will cause a crash.
// There may be multiple entries for the same node
void remove_parse_node(const parse_node_ptr p)
{
    if (p == nullptr)
        return;

    const parse_node_ptr prev = p->prev;
    const parse_node_ptr next = p->next;

    if (prev)
        prev->next = next;
    if (next)
        next->prev = prev;
    if (p == head_node)
        head_node = next;

    free_parse_node(p);
}

/// <summary>
/// Frees the tree.
/// </summary>
void free_parse_tree(void)
{
    for (parse_node_ptr p = head_node; p != nullptr;) {
        const parse_node_ptr next = p->next;
        remove_parse_node(p);
        p = next;
    }
    head_node = current_node = nullptr;
}

//
// Print indent
//
void print_indent(std::ostream& file)
{
    for (int index = 0; index < print_nest_level; index++)
        file << "    ";
}
void print_indent(std::ofstream& file)
{
    for (int index = 0; index < print_nest_level; index++)
        file << "    ";
}

int is_valid_parse_tree(void)
{
    for (parse_node_ptr p = head_node; p; p = p->next) {
        if (!is_valid_parse_node(p))
            return 0;
    }
    return ~0;
}

int is_valid_parse_node(const parse_node_ptr p)
{
    if (p == nullptr) return 0;

    if (p->type <= type_unknown || p->type >= type_last)
        return 0;

    for (int i = 0; i < p->number_of_ops; i++) {
        if (!is_valid_parse_node(p->op[i]))
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
    if (p->number_of_ops > 0 && p->op) {
        FREE(p->op);
        p->op = nullptr;
        p->number_of_ops = 0;
    }
    p->allocated = false;
    FREE(p);
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

//
// Print a node
//
void print_node(parse_node_ptr p, std::ostream& file)
{
    // ReSharper disable once CppTooWideScope
    int index;

    print_indent(file);
    if (print_nest_level > 0)
        file << "CHILD ";
    file << "NODE [line " << yylineno << "]" << std::endl;
    print_nest_level++;

    print_indent(file);
    file << "allocated " << p->allocated << std::endl;

    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    // ReSharper disable once CppIncompleteSwitchStatement
    switch (p->type)  // NOLINT(clang-diagnostic-switch)
    {
        case type_unknown:
            print_indent(file);
            file << "type type_unknown" << std::endl;
            print_indent(file);
            break;

        case type_id:
        case type_macro_id:
        case type_label:
            print_indent(file);
            switch (p->type)  // NOLINT(clang-diagnostic-switch-enum)
            {
                case type_id:
                    file << "type type_id" << std::endl;
                    break;

                case type_label:
                    file << "type type_label" << std::endl;
                    break;

                case type_macro_id:
                    file << "type type_macro_id" << std::endl;
                    break;

                default:
                    break;
            }
            print_indent(file);
            file << "name " << p->id.name << std::endl;
            print_indent(file);
            if (p->id.symbol_ptr == nullptr) {
                file << "i    (nil)" << std::endl;
            }
            else {
                file << "i    0x" << std::setfill('0') << std::setw(8) << std::hex << p->id.symbol_ptr << std::endl;
            }
            if (p->id.symbol_ptr) {
                print_indent(file);
                file << "     fullname     " << p->id.symbol_ptr->fullname << std::endl;
                print_indent(file);
                file << "     is_initialized  " << p->id.symbol_ptr->is_initialized << std::endl;
                print_indent(file);
                file << "     value        " << p->id.symbol_ptr->value << std::endl;
                print_indent(file);
                file << "     ismacroname  " << p->id.symbol_ptr->is_macro_name << std::endl;
                print_indent(file);
                file << "     ismacroparam " << p->id.symbol_ptr->is_macro_param << std::endl;
                print_indent(file);
                file << "     isplusminus  " << p->id.symbol_ptr->is_plus_minus << std::endl;
                print_indent(file);
                file << "     isvar        " << p->id.symbol_ptr->is_var << std::endl;
                print_indent(file);
                file << "     macroNode    " << std::setfill('0') << std::setw(8) << std::hex << p->id.symbol_ptr->macro_node << std::endl;
                print_indent(file);
                file << "     scope      " << (p->id.symbol_ptr->scope ? p->id.symbol_ptr->scope : "NULL") << std::endl;
                print_indent(file);
                file << "     name         " << p->id.symbol_ptr->name << std::endl;
            }
            break;

        case type_macro_ex:
            break;

        case type_opr:
            print_indent(file);
            file << "type typeOpr" << std::endl;
            switch (p->opr.opr) {
                case INC:
                    print_indent(file);
                    file << "opr INC" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case LOAD:
                    print_indent(file);
                    file << "opr LOAD" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case LOBYTE:
                    print_indent(file);
                    file << "opr LOBYTE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case HIBYTE:
                    print_indent(file);
                    file << "opr HIBYTE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case PCASSIGN:
                    print_indent(file);
                    file << "opr PCASSIGN" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case ORG:
                    print_indent(file);
                    file << "opr ORG" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case EXPRLIST:
                    print_indent(file);
                    file << "opr EXPRLIST" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case MACRO:
                    print_indent(file);
                    file << "opr MACRO" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case WHILE:
                    print_indent(file);
                    file << "opr WHILE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case REPEAT:
                    print_indent(file);
                    file << "opr REPEAT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case SECTION:
                    print_indent(file);
                    file << "opr SECTION" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case ENDSECTION:
                    print_indent(file);
                    file << "opr ENDSECTION" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case DO:
                    print_indent(file);
                    file << "opr DO" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case FOR:
                    print_indent(file);
                    file << "opr FOR" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case REGX:
                    print_indent(file);
                    file << "opr REGX" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case REGY:
                    print_indent(file);
                    file << "opr REGY" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case IF:
                    print_indent(file);
                    file << "opr IF" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case PRINT:
                    print_indent(file);
                    file << "opr PRINT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case DS:
                    print_indent(file);
                    file << "opr DS" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case STATEMENT:
                    print_indent(file);
                    file << "opr STATEMENT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case END:
                    print_indent(file);
                    file << "opr END" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case EQU:
                    print_indent(file);
                    file << "opr EQU" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '=':
                    print_indent(file);
                    file << "opr '='" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case UMINUS:
                    print_indent(file);
                    file << "opr UMINUS" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '~':
                    print_indent(file);
                    file << "opr '~'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '+':
                    print_indent(file);
                    file << "opr '+'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '-':
                    print_indent(file);
                    file << "opr '-'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '*':
                    print_indent(file);
                    file << "opr '*'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '/':
                    print_indent(file);
                    file << "opr '/'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case BIT_OR:
                    print_indent(file);
                    file << "opr BIT_OR" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case BIT_AND:
                    print_indent(file);
                    file << "opr BIT_AND" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '^':
                    print_indent(file);
                    file << "opr '^'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '<':
                    print_indent(file);
                    file << "opr '<'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '>':
                    print_indent(file);
                    file << "opr '>'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case EQ:
                    print_indent(file);
                    file << "opr EQ" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case NE:
                    print_indent(file);
                    file << "opr NE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case GE:
                    print_indent(file);
                    file << "opr GE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case LE:
                    print_indent(file);
                    file << "opr LE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case OR:
                    print_indent(file);
                    file << "opr OR" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case AND:
                    print_indent(file);
                    file << "opr AND" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case NOT:
                    print_indent(file);
                    file << "opr NOT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case SHIFT_LEFT:
                    print_indent(file);
                    file << "opr SHIFT_LEFT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case SHIFT_RIGHT:
                    print_indent(file);
                    file << "opr SHIFT_RIGHT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                default:
                    break;
            }
            break;

        case type_op_code:
            print_indent(file);
            file << "type typeOpCode" << std::endl;

            print_indent(file);
            file << "instruction   " << instruction_to_string(p->opcode.instruction) << std::endl;

            print_indent(file);
            file << "mode          " << mode_to_string(p->opcode.mode) << std::endl;

            print_indent(file);
            file << "opCode        " << std::hex << std::setw(2) << std::uppercase << std::setfill('0') << p->opcode.opcode << std::endl;

            print_indent(file);
            file << "PC            0x" << std::hex << std::setw(8) << std::setfill('0') << p->opcode.program_counter << std::endl;
            for (index = 0; index < p->number_of_ops; index++)
                print_node(p->op[index], file);
            break;

        case type_con:
            print_indent(file);
            file << "type typeCon" << std::endl;

            print_indent(file);
            file << "IsPC  " << p->con.is_program_counter << std::endl;

            if (p->con.is_program_counter)
                file << "     PC = " << std::setw(8) << std::hex << std::setfill('0') << program_counter << std::endl;

            print_indent(file);
            file << "value 0x" << std::setw(8) << std::hex << std::setfill('0') << p->con.value << std::endl;

            for (index = 0; index < p->number_of_ops; index++)
                print_node(p->op[index], file);
            break;

        case type_data:
            print_indent(file);
            file << "type typeData" << std::endl;

            print_indent(file);
            file << "size " << p->data.size << std::endl;

            print_node(static_cast<parse_node_ptr>(p->data.data), file);
            for (index = 0; index < p->number_of_ops; index++)
                print_node(p->op[index], file);
            break;

        case type_str:
            print_indent(file);
            file << "type typeStr" << std::endl;

            print_indent(file);
            file << "allocated  " << p->str.allocated << std::endl;

            print_indent(file);
            file << "len        0x" << std::setw(8) << std::hex << std::setfill('0') << p->str.len << std::endl;

            print_indent(file);
            file << "value " << p->str.value << std::endl;
            for (index = 0; index < p->number_of_ops; index++)
                print_node(p->op[index], file);
            break;
    }

    print_nest_level--;
}

void print_node(parse_node_ptr p, std::ofstream& file)
{
    // ReSharper disable once CppTooWideScope
    int index;

    print_indent(file);
    if (print_nest_level > 0)
        file << "CHILD ";
    file << "NODE [line " << yylineno << "]" << std::endl;
    print_nest_level++;

    print_indent(file);
    file << "allocated " << p->allocated << std::endl;

    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    // ReSharper disable once CppIncompleteSwitchStatement
    switch (p->type)  // NOLINT(clang-diagnostic-switch)
    {
        case type_unknown:
            print_indent(file);
            file << "type type_unknown" << std::endl;
            print_indent(file);
            break;

        case type_id:
        case type_macro_id:
        case type_label:
            print_indent(file);
            switch (p->type)  // NOLINT(clang-diagnostic-switch-enum)
            {
                case type_id:
                    file << "type type_id" << std::endl;
                    break;

                case type_label:
                    file << "type type_label" << std::endl;
                    break;

                case type_macro_id:
                    file << "type type_macro_id" << std::endl;
                    break;

                default:
                    break;
            }
            print_indent(file);
            file << "name " << p->id.name << std::endl;
            print_indent(file);
            if (p->id.symbol_ptr == nullptr) {
                file << "i    (nil)" << std::endl;
            }
            else {
                file << "i    0x" << std::setfill('0') << std::setw(8) << std::hex << p->id.symbol_ptr << std::endl;
            }
            if (p->id.symbol_ptr) {
                print_indent(file);
                file << "     fullname     " << p->id.symbol_ptr->fullname << std::endl;
                print_indent(file);
                file << "     is_initialized  " << p->id.symbol_ptr->is_initialized << std::endl;
                print_indent(file);
                file << "     value        " << p->id.symbol_ptr->value << std::endl;
                print_indent(file);
                file << "     ismacroname  " << p->id.symbol_ptr->is_macro_name << std::endl;
                print_indent(file);
                file << "     ismacroparam " << p->id.symbol_ptr->is_macro_param << std::endl;
                print_indent(file);
                file << "     isplusminus  " << p->id.symbol_ptr->is_plus_minus << std::endl;
                print_indent(file);
                file << "     isvar        " << p->id.symbol_ptr->is_var << std::endl;
                print_indent(file);
                file << "     macroNode    " << std::setfill('0') << std::setw(8) << std::hex << p->id.symbol_ptr->macro_node << std::endl;
                print_indent(file);
                file << "     scope      " << (p->id.symbol_ptr->scope ? p->id.symbol_ptr->scope : "NULL") << std::endl;
                print_indent(file);
                file << "     name         " << p->id.symbol_ptr->name << std::endl;
            }
            break;

        case type_macro_ex:
            break;

        case type_opr:
            print_indent(file);
            file << "type typeOpr" << std::endl;
            switch (p->opr.opr) {
                case INC:
                    print_indent(file);
                    file << "opr INC" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case LOAD:
                    print_indent(file);
                    file << "opr LOAD" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case LOBYTE:
                    print_indent(file);
                    file << "opr LOBYTE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case HIBYTE:
                    print_indent(file);
                    file << "opr HIBYTE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case PCASSIGN:
                    print_indent(file);
                    file << "opr PCASSIGN" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case ORG:
                    print_indent(file);
                    file << "opr ORG" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case EXPRLIST:
                    print_indent(file);
                    file << "opr EXPRLIST" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case MACRO:
                    print_indent(file);
                    file << "opr MACRO" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case WHILE:
                    print_indent(file);
                    file << "opr WHILE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case REPEAT:
                    print_indent(file);
                    file << "opr REPEAT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case SECTION:
                    print_indent(file);
                    file << "opr SECTION" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case ENDSECTION:
                    print_indent(file);
                    file << "opr ENDSECTION" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case DO:
                    print_indent(file);
                    file << "opr DO" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case FOR:
                    print_indent(file);
                    file << "opr FOR" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case REGX:
                    print_indent(file);
                    file << "opr REGX" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case REGY:
                    print_indent(file);
                    file << "opr REGY" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case IF:
                    print_indent(file);
                    file << "opr IF" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case PRINT:
                    print_indent(file);
                    file << "opr PRINT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case DS:
                    print_indent(file);
                    file << "opr DS" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case STATEMENT:
                    print_indent(file);
                    file << "opr STATEMENT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case END:
                    print_indent(file);
                    file << "opr END" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case EQU:
                    print_indent(file);
                    file << "opr EQU" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '=':
                    print_indent(file);
                    file << "opr '='" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case UMINUS:
                    print_indent(file);
                    file << "opr UMINUS" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '~':
                    print_indent(file);
                    file << "opr '~'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '+':
                    print_indent(file);
                    file << "opr '+'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '-':
                    print_indent(file);
                    file << "opr '-'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '*':
                    print_indent(file);
                    file << "opr '*'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '/':
                    print_indent(file);
                    file << "opr '/'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case BIT_OR:
                    print_indent(file);
                    file << "opr BIT_OR" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case BIT_AND:
                    print_indent(file);
                    file << "opr BIT_AND" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '^':
                    print_indent(file);
                    file << "opr '^'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '<':
                    print_indent(file);
                    file << "opr '<'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case '>':
                    print_indent(file);
                    file << "opr '>'" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case EQ:
                    print_indent(file);
                    file << "opr EQ" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case NE:
                    print_indent(file);
                    file << "opr NE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case GE:
                    print_indent(file);
                    file << "opr GE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case LE:
                    print_indent(file);
                    file << "opr LE" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case OR:
                    print_indent(file);
                    file << "opr OR" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case AND:
                    print_indent(file);
                    file << "opr AND" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case NOT:
                    print_indent(file);
                    file << "opr NOT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case SHIFT_LEFT:
                    print_indent(file);
                    file << "opr SHIFT_LEFT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                case SHIFT_RIGHT:
                    print_indent(file);
                    file << "opr SHIFT_RIGHT" << std::endl;
                    for (index = 0; index < p->number_of_ops; index++)
                        print_node(p->op[index], file);
                    break;

                default:
                    break;
            }
            break;

        case type_op_code:
            print_indent(file);
            file << "type typeOpCode" << std::endl;

            print_indent(file);
            file << "instruction   " << instruction_to_string(p->opcode.instruction) << std::endl;

            print_indent(file);
            file << "mode          " << mode_to_string(p->opcode.mode) << std::endl;

            print_indent(file);
            file << "opCode        " << std::hex << std::setw(2) << std::uppercase << std::setfill('0') << p->opcode.opcode << std::endl;

            print_indent(file);
            file << "PC            0x" << std::hex << std::setw(8) << std::setfill('0') << p->opcode.program_counter << std::endl;
            for (index = 0; index < p->number_of_ops; index++)
                print_node(p->op[index], file);
            break;

        case type_con:
            print_indent(file);
            file << "type typeCon" << std::endl;

            print_indent(file);
            file << "IsPC  " << p->con.is_program_counter << std::endl;

            if (p->con.is_program_counter)
                file << "     PC = " << std::setw(8) << std::hex << std::setfill('0') << program_counter << std::endl;

            print_indent(file);
            file << "value 0x" << std::setw(8) << std::hex << std::setfill('0') << p->con.value << std::endl;

            for (index = 0; index < p->number_of_ops; index++)
                print_node(p->op[index], file);
            break;

        case type_data:
            print_indent(file);
            file << "type typeData" << std::endl;

            print_indent(file);
            file << "size " << p->data.size << std::endl;

            print_node(static_cast<parse_node_ptr>(p->data.data), file);
            for (index = 0; index < p->number_of_ops; index++)
                print_node(p->op[index], file);
            break;

        case type_str:
            print_indent(file);
            file << "type typeStr" << std::endl;

            print_indent(file);
            file << "allocated  " << p->str.allocated << std::endl;

            print_indent(file);
            file << "len        0x" << std::setw(8) << std::hex << std::setfill('0') << p->str.len << std::endl;

            print_indent(file);
            file << "value " << p->str.value << std::endl;
            for (index = 0; index < p->number_of_ops; index++)
                print_node(p->op[index], file);
            break;
    }

    print_nest_level--;
}
// 
// Generate a list node entry
// based on node
//
// This should only be called on the FinalPass
// when all symbols/macros are resolved
int generate_list_node(const parse_node_ptr p)
{
    int col = 0;
    int bytes = 0;
    int len;
    char* current_buffer = static_cast<char*>(MALLOC(10LLU * 1024));
    char* current_ptr = current_buffer;
    *current_buffer = 0;
    memset(current_buffer, 0, 10LLU * 1024);

    // passing NULL will cause an empty node which forces a the
    // current file to listed even if no code is generated
    if (p == nullptr) {
        if (add_list(current_file_name, yylineno, "") == nullptr) {
            error(error_creating_list_node);
        }
        FREE(current_buffer);
        return 0;
    }

    if (p->type == type_print) {
        if (p->pr.print_state != print_list_state) {
            print_list_state = false;
            auto node = add_list(current_file_name, yylineno, "");
            node->list_directive = true;
            print_list_state = p->pr.print_state;
        }
        FREE(current_buffer);
        return 1;
    }

    // get the program counter
    // if the node is an opcode get the PC from the opcode itself
    // otherwise use PC
    int op = p->type == type_op_code ? p->opcode.program_counter & 0xFFFF : program_counter & 0xFFFF;
    sprintf(current_ptr, "$%02X%02X:", ((op) >> 8), op & 0xFF);
    current_ptr += strlen(current_ptr);

    // generate a list node for a string
    if (p->type == type_str) {
        const char* string_start = nullptr;
        char* str = p->str.value;
        int output_length = p->str.len;
        int pad = 0;
        while (output_length--) {
            // output the PC on column zero
            if (col == 0) {
                string_start = str;
                current_ptr = current_buffer;
                op = (program_counter + bytes) & 0xFFFF;
                sprintf(current_ptr, "$%02X%02X:", ((op) >> 8), op & 0xFF);
                current_ptr += strlen(current_ptr);
            }

            col++;
            sprintf(current_ptr, " $%02X", *str & 0xFF);
            current_ptr += strlen(current_ptr);
            str++;
            bytes++;

            // at column 3 create a node and reset to column 0
            if (col == 3) {
                len = static_cast<int>(strlen(current_buffer));
                while (len++ < SRC_LST_INDENT) {
                    *current_ptr++ = ' ';
                }
                *current_ptr = 0;
                sprintf(current_ptr, ".db ");
                current_ptr += strlen(current_ptr);
                for (len = 0; len < col; len++) {
                    if (len > 0) {
                        sprintf(current_ptr, ", ");
                        current_ptr += strlen(current_ptr);
                    }
                    if (isprint(string_start[len])) {
                        sprintf(current_ptr, "'%c'", string_start[len]);
                    }
                    else {
                        sprintf(current_ptr, "$%02X", string_start[len] & 0xFF);
                    }
                    current_ptr += strlen(current_ptr);
                }

                if (add_list(current_file_name, yylineno, current_buffer) == nullptr) {
                    error(error_creating_list_node);
                    return 0;
                }
                col = 0;
            }
        }

        // if using a string with WORD then pad to a word boundary
        if (data_size == 2 && p->str.len % 2) {
            col++;
            sprintf(current_ptr, " $%02X", *str & 0xFF);
            current_ptr += strlen(current_ptr);
            pad = 1;
        }

        // output final node
        if (col != 0) {
            len = static_cast<int>(strlen(current_buffer));
            while (len++ < SRC_LST_INDENT) {
                *current_ptr++ = ' ';
            }
            sprintf(current_ptr, ".db ");
            current_ptr += strlen(current_ptr);
            for (len = 0; len < col; len++) {
                if (len > 0) {
                    sprintf(current_ptr, ", ");
                    current_ptr += strlen(current_ptr);
                }
                if (len == col - 1 && pad) {
                    sprintf(current_ptr, "$%02X", string_start[len]);
                }
                else {
                    sprintf(current_ptr, "'%c'", string_start[len]);
                }
                current_ptr += strlen(current_ptr);
            }

            if (add_list(current_file_name, yylineno, current_buffer) == nullptr) {
                error(error_creating_list_node);
                exit(-1);  // NOLINT(concurrency-mt-unsafe)
            }
        }
        FREE(current_buffer);
        return bytes;
    }

    // output numeric value
    // DataSize contain number of bytes to output
    if (p->type != type_op_code) {
        // expand the node
        op = expand_node(p);

        unsigned char hi = static_cast<unsigned char>((op & 0xFF00) >> 8);
        const unsigned char lo = static_cast<unsigned char>(op & 0xFF);

        if (op < 0 && data_size == 1) {
            hi = 0;
        }
        // check for overflow
        if (hi != 0 && data_size < 2) {
            FREE(current_buffer);
            // This error is reported in generate_output
            fprintf(console, "%s\n", current_buffer);
            return 0;
        }

        // see if we can append to last node
        list_table_ptr current_list_ptr = list_head;
        if (current_list_ptr) {
            while (current_list_ptr->next) {
                current_list_ptr = current_list_ptr->next;
            }
        }

        // output lo byte
        sprintf(current_ptr, " $%02X", lo);
        current_ptr += strlen(current_ptr);

        // output hi byte
        if (data_size > 1) {
            sprintf(current_ptr, " $%02X", hi);
            current_ptr += strlen(current_ptr);
        }
        len = static_cast<int>(strlen(current_buffer));
        while (len++ < SRC_LST_INDENT) {
            *current_ptr++ = ' ';
        }
        sprintf(current_ptr, ".db ");
        current_ptr += strlen(current_ptr);
        sprintf(current_ptr, "$%02X", lo);
        current_ptr += strlen(current_ptr);
        if (data_size > 1 || (data_size == 0 && hi != 0)) {
            sprintf(current_ptr, ", $%02X", hi);
            // ReSharper disable once CppAssignedValueIsNeverUsed
            current_ptr += strlen(current_ptr);
        }
        // add the node
        if (add_list(current_file_name, yylineno, current_buffer) == nullptr) {
            error(error_creating_list_node);
            exit(-1);  // NOLINT(concurrency-mt-unsafe)
        }
        FREE(current_buffer);
        return 0;
    }

    // output an opcode list node

    // get the opcode
    sprintf(current_ptr, " $%02X", p->opcode.opcode);
    current_ptr += strlen(current_ptr);

    // get number of bytes used by opcode
    bytes = get_op_byte_count(p);
    if (bytes > 0) {
        // expand the data
        op = expand_node(p->op[0]);

        // if relative mode check range
        if (p->opcode.mode == r) {
            op -= (p->opcode.program_counter + 2);
            if (op > 128 || op < -127) {
                // This error is reported in generate_output
                fprintf(console, "%s\n", current_buffer);
                return 0;
            }
            op &= 0xFF;
        }

        // get the hi and lo byte
        const unsigned char hi = static_cast<unsigned char>((op & 0xFF00) >> 8);
        const unsigned char lo = static_cast<unsigned char>(op & 0xFF);

        // output the lo byte
        sprintf(current_ptr, " $%02X", lo);
        current_ptr += strlen(current_ptr);

        // output hi byte if needed
        if (bytes > 1) {
            sprintf(current_ptr, " $%02X", hi);
            current_ptr += strlen(current_ptr);
        }
    }

    col = static_cast<int>(strlen(current_buffer));
    while (col++ < SRC_LST_INDENT) {
        *current_ptr++ = ' ';
    }
    sprintf(current_ptr, "%s ", instruction_to_string(p->opcode.instruction));
    current_ptr += strlen(current_ptr);
    if (bytes > 0 && (p->number_of_ops > 0)) {
        op = expand_node(p->op[0]);
        if (p->number_of_ops > 1) {
            expand_node(p->op[1]);
        }
        // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
        switch (p->opcode.mode) {
            case i:      /* implied *           */
            case A:      /* Accumulator         */
                break;

            case I:      /* immediate           */
                sprintf(current_ptr, "#$%02X ", op);
                break;

            case zp:     /* zero page            */
                sprintf(current_ptr, "$%02X ", op);
                break;

            case zpi:    /* zero page indirect   */
                sprintf(current_ptr, "($%02X) ", op);
                break;

            case zpx:    /* zero page x          */
                sprintf(current_ptr, "$%02X,x ", op);
                break;

            case zpy:    /* zero page y          */
                sprintf(current_ptr, "$%02X,y ", op);
                break;

            case izx:   /* zero page indirect x */
                sprintf(current_ptr, "($%02X,x) ", op);
                break;

            case izy:   /* zero page indirect y */
                sprintf(current_ptr, "($%02X),y ", op);
                break;

            case a:      /* absolute             */
                sprintf(current_ptr, "$%04X ", op);
                break;

            case aix:    /* absolute indirect x  */  // NOLINT(bugprone-branch-clone)
                sprintf(current_ptr, "$%04X,x ", op);
                break;

            case ax:     /* absolute x           */
                sprintf(current_ptr, "$%04X,x ", op);
                break;

            case ay:     /* absolute y           */
                sprintf(current_ptr, "$%04X,y ", op);
                break;

            case ind:    /* absolute indirect    */
                sprintf(current_ptr, "($%04X) ", op);
                break;

            case r:      /* relative             */
                sprintf(current_ptr, "$%04X ", op);
                break;

        }
    }

    // add the node
    if (add_list(current_file_name, yylineno, current_buffer) == nullptr) {
        FREE(current_buffer);
        error(error_creating_list_node);
        return 0;
    }
    FREE(current_buffer);
    return bytes + 1;
}
