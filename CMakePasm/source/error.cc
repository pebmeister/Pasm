// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppClangTidyCertErr33C
#include <assert.h>  // NOLINT(modernize-deprecated-headers)
#include <string.h>
#include <map>

#include "error.h"
#include "flex.h"
#include "pasm.h"

// number of errors
int error_count = 0;
int warning_count = 0;

// error strings
std::map<int, std::string> error_lookup =
{
    { error_out_of_memory, "Out of Memory."},
    { error_source_string_null, "Source string NULL."},
    { error_unrecognized_escape_sequence, "Unrecognized escape sequence."},
    { error_cant_open_include_file, "Can't open include file"},
    { error_no_input_file_specified, "No input file specified."},
    { error_cant_open_input_file, "Can't open input file."},
    { error_opening_list_file, "Error opening list list file."},
    { error_invalid_opcode_or_mode, "Invalid opcode or mode."},
    { error_writing_output_file, "Error writing to output file."},
    { error_value_out_of_range, "Value out of range."},
    { error_missing_parameter, "Missing parameter."},
    { error_adding_symbol, "Error adding symbol."},
    { error_for_reg_cant_be_nested, "FOR_REG can't be nested."},
    { error_end_section_without_section, "END_SECTION without section."},
    { error_branch_out_of_range, "Branch out of range, creating jmp."},
    { error_org_specified_more_than_once, "ORG specified more than once."},
    { error_initializing_variable, "Error setting symbol value."},
    { error_unknown_node_type, "Internal error: Unknown node type."},
    { error_unknown_operator_type, "Internal error: Unknown operator type."},
    { error_divide_by_zero, "Divide by zero."},
    { error_infinite_loop_detected, "Infinite loop detected."},
    { error_expected_next, "Error expected NEXT."},
    { error_free_without_malloc, "Error free called without alloc."},
    { error_free_unknown_pointer, "Error free unknown pointer."},
    { error_macro_parameter_not_found, "Macro Parameter not found."},
    { error_macro_parameter_under_flow, "Macro parameter underflow."},
    { error_plus_sym_not_allowed_in_macro, "Plus and Minus symbols not allowed in macro."},
    { error_plus_sym_overflow, "Plus symbol overflow."},
    { error_missing_output_file, "Error missing output file."},
    { error_output_file_specified_more_than_once, "Output file specified more than once."},
    { error_missing_symbol_file, "Error missing symbol file name."},
    { error_symbol_file_specified_more_than_once, "Symbol file specified more than once."},
    { error_invalid_parameters, "Invalid parameters specified."},
    { error_ignore_warnings_specified_more_than_once, "Ignore warnings specified more than once."},
    { error_instruction_set_specified_more_than_once, "Instruction set specified more than once."},
    { error_illegal_opcodes_specified_more_than_once, "Allow illegal opcodes specified more than once."},
    { error_missing_log_file, "Error missing log file name."},
    { error_log_file_specified_more_than_once, "Logfile specified more than once."},
    { error_c64_output_format_specified_more_than_once, "Output file format specified more than once."},
    { error_verbose_specified_more_than_once, "Verbose specified more than once."},
    { error_missing_list_file, "Error missing list file."},
    { error_list_file_specified_more_than_once, "List file specified more than once."},
    { error_opening_log_file, "Error creating log file."},
    { error_maximum_number_of_passes_exceeded, "Maximum number of passes exceeded."},
    { error_opening_symbol_file, "Error opening symbol file."},
    { error_opening_output_file, "Error opening output file."},
    { error_creating_list_node, "Error creating list node."},
    { error_path_name_too_long, "Error path name too long."},
};

/**
 * \brief Print an error on the screen
 * \param err_num number of the error
 */
void error(const int err_num)
{
    assert(err_num > error_start && err_num < error_max);
    yyerror((error_lookup[err_num]).c_str());
}

/**
 * \brief Print a warning on the screen
 * \param err_num number of the error
 */
void warn(const int err_num)
{
    assert(err_num > error_start && err_num < error_max);
    yymessage((error_lookup[err_num]).c_str());
    warning_count++;
}

/**
 * \brief Print an error on the screen
 * \param err_num number of the error
 * \param parameter string to print with error
 */
void error(const int err_num, const char* parameter)
{
    assert(err_num > error_start && err_num < error_max);
    yyerror((error_lookup[(int)err_num] + " " + parameter).c_str());
}

/// <summary>
/// message routine.
/// </summary>
/// <param name="s">The s.</param>
void yymessage(const char* s)
{
    if (yyin && current_file_name != NULL && internal_buffer != NULL) {
        int current_line;

        fprintf(console_error, "%s File %s near line %d\n", s, current_file_name, yylineno + 1);

        const long current_position = ftell(yyin);
        fseek(yyin, 0, SEEK_SET);
        for (current_line = 1; current_line < yylineno - 2; current_line++)
            fgets(internal_buffer, max_line_len, yyin);

        for (; current_line < yylineno + 3; current_line++) {
            if (!feof(yyin)) {
                *internal_buffer = 0;
                fgets(internal_buffer, max_line_len, yyin);
                fprintf(console_error, "%-5d  ", current_line);
                fputs(internal_buffer, console_error);
            }
        }
        fputs("\n", console_error);
        fseek(yyin, current_position, SEEK_SET);
    }
    else
        fprintf(console_error, "%s\n", s);
}

/// <summary>
/// Error routine.
/// </summary>
/// <param name="s">The error string.</param>
void yyerror(const char* s)
{
    yymessage(s);
    error_count++;
}
