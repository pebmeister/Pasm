// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppClangTidyCertErr33C
#include <assert.h>  // NOLINT(modernize-deprecated-headers)
#include <string.h>

#include "error.h"
#include "flex.h"
#include "pasm.h"

// number of errors
int error_count = 0;
int warning_count = 0;

// error strings
// order must be same as defined in error.h
const char* errors[] =
{
    "",
    "Out of Memory.",
    "Source string NULL.",
    "Unrecognized escape sequence.",
    "Can't open include file",
    "No input file specified.",
    "Can't open input file.",
    "Error opening list list file.",
    "Invalid opcode or mode.",
    "Error writing to output file.",
    "Value out of range.",
    "Missing parameter.",
    "Error adding symbol.",
    "FOR_REG can't be nested.",
    "END_SECTION without section.",
    "Branch out of range, creating jmp.",
    "ORG specified more than once.",
    "Error setting symbol value.",
    "Internal error: Unknown node type.",
    "Internal error: Unknown operator type.",
    "Divide by zero.",
    "Infinite loop detected.",
    "Error expected NEXT.",
    "Error free called without alloc.",
    "Error free unknown pointer.",
    "Macro Parameter not found.",
    "Macro parameter underflow.",
    "Plus and Minus symbols not allowed in macro.",
    "Plus symbol overflow.",
    "Error missing output file.",
    "Output file specified more than once.",
    "Error missing symbol file name.",
    "Symbol file specified more than once.",
    "Invalid parameters specified.",
    "Ignore warnings specified more than once.",
    "Instruction set specified more than once.",
    "Allow illegal opcodes specified more than once.",
    "Error missing log file name.",
    "Logfile specified more than once.",
    "Output file format specified more than once.",
    "Verbose specified more than once.",
    "Error missing list file.",
    "List file specified more than once.",
    "Error creating log file.",
    "Maximum number of passes exceeded.",
    "Error opening symbol file.",
    "Error opening output file.",
    "Error creating list node.",
    "Error path name too long.",
};

/**
 * \brief Print an error on the screen
 * \param err_num number of the error
 */
void error(const int err_num)
{
    assert(err_num > error_start && err_num < error_max);
    yyerror(errors[err_num]);
}

/**
 * \brief Print a warning on the screen
 * \param err_num number of the error
 */
void warn(const int err_num)
{
    assert(err_num > error_start && err_num < error_max);
    yymessage(errors[err_num]);
    warning_count++;
}

/**
 * \brief Print an error on the screen
 * \param err_num number of the error
 * \param parameter string to print with error
 */
void error2(const int err_num, const char* parameter)
{
    assert(err_num > error_start && err_num < error_max);
    assert(internal_buffer != NULL);
    if (strlen(errors[err_num]) + strlen(parameter) + 2 < max_line_len)
    {
        sprintf(internal_buffer, "%s %s", errors[err_num], parameter);  // NOLINT(cert-err33-c)
    }
    else
    {
        error(err_num);
    }
    yyerror(internal_buffer);
}

/// <summary>
/// message routine.
/// </summary>
/// <param name="s">The s.</param>
void yymessage(const char* s)
{
    if (yyin && current_file_name != NULL && internal_buffer != NULL)
    {
        int current_line;

        fprintf(console_error, "%s File %s near line %d\n", s, current_file_name, yylineno + 1);

        const long current_position = ftell(yyin);
        fseek(yyin, 0, SEEK_SET);
        for (current_line = 1; current_line < yylineno - 2; current_line++)
            fgets(internal_buffer, max_line_len, yyin);

        for (; current_line < yylineno + 3; current_line++)
        {
            if (!feof(yyin))
            {
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
