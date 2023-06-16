// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppClangTidyCertErr33C
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "error.h"
#include "expand.h"
#include "genoutput.h"
#include "node.h"
#include "opcodes.h"
#include "pasm.h"

// number of byte written to output file
int total_bytes_written = 0;
static int byte_delta = 0;
static int origin = 0;

int write_byte(FILE* file, unsigned char ch);
int write_word(FILE* file, int word);
int write_string(FILE* file, const char* str);

/**
 * \brief reset output variables
 */
void generate_output_reset(void)
{
    total_bytes_written = 0;
    byte_delta = 0;
    origin = 0;
    origin_specified = 0;
}

int write_data(FILE* file, const int data)
{
    if (data_size == 1)
        // ReSharper disable once CppRedundantCastExpression
        return write_byte(file, (char)data);
    if (data_size == 2)
        return write_word(file, data);

    return -1;
}

int write_byte(FILE* file, const unsigned char ch)
{
    // ReSharper disable once CppRedundantCastExpression
    const int bytes_written = (int) fwrite(&ch, 1, 1, file);
    if (bytes_written < 1)
    {
        error(error_writing_output_file);
        return 0;
    }
    return bytes_written;
}

int write_word(FILE* file, const int word)
{
    const unsigned char hi = (word & 0xFF00) >> 8;
    const unsigned char lo = word & 0xFF;

    int bytes_written = write_byte(file, lo);
    if (bytes_written > 0)
    {
        bytes_written += write_byte(file, hi);
    }
    return bytes_written;
}

int write_string(FILE* file, const char* str)
{
    const size_t len = strlen(str);
    // ReSharper disable once CppRedundantCastExpression
    const int bytes_written = (int)fwrite(str, 1, len, file);
    if (bytes_written < (int) len)
    {
        error(error_writing_output_file);
        return 0;
    }
    return bytes_written;
}

int write_header(FILE* file)
{
    int bytes_written = 0;
    origin = program_counter;
    byte_delta = 0;

    // write header if c64
    if (output_file_format == c64)
    {
        bytes_written = write_word(output_file, origin);
        total_bytes_written += (int)bytes_written;

        if (total_bytes_written < 2)
        {
            return -1;
        }
        byte_delta = 2;
    }
    return bytes_written;
}

int write_program_counter_pad(FILE* file, const int delta)
{
    while (delta + byte_delta > total_bytes_written)
    {
        const int bytes_written = write_byte(output_file, 0);
        total_bytes_written += (int)bytes_written;
        if (bytes_written == 0)
        {
            return -1;
        }
    }
    return 0;
}

int write_string_node(FILE* file, const parse_node_ptr p)
{
    int bytes_written = write_string(output_file, p->str.value);
    total_bytes_written += (int)bytes_written;
    if (bytes_written < p->str.len)
    {
        return -1;
    }

    // if using a string with WORD then pad to a word boundary
    if (data_size == 2 && bytes_written % 2)
    {
        bytes_written = write_byte(output_file, 0);
        total_bytes_written += (int)bytes_written;
        if (bytes_written == 0)
        {
            return -1;
        }
    }
    return bytes_written;
}

int write_data_node(FILE* file, const parse_node_ptr p)
{
    // expand_node the expression
    const int op = expand_node(p);
    const int overflow = (op & ~0xFFFF) != 0;
    const int large_value = (op & ~0xFF) != 0;
    int bytes_written;

    if (overflow && op < 0)
    {
        if (data_size != 1)
        {
            if (-(op) > 0xFFFE)
            {
                error(error_value_out_of_range);
                return 0;
            }
            bytes_written = write_word(output_file, op);
            total_bytes_written += (int)bytes_written;
        }
        else
        {
            if (-(op) > 0xFE)
            {
                error(error_value_out_of_range);
                return 0;
            }
            bytes_written = write_byte(output_file, op & 0xFF);
            total_bytes_written += (int)bytes_written;
        }
    }
    else
    {
        if ((data_size == 1 && large_value) || overflow)
        {
            error(error_value_out_of_range);
            return 0;
        }
        if (data_size == 1 || (data_size == 0 && !large_value))
        {
            bytes_written = write_byte(output_file, op);  // NOLINT(clang-diagnostic-implicit-int-conversion)
            total_bytes_written += (int)bytes_written;
        }
        else
        {
            bytes_written = write_word(output_file, op);
            total_bytes_written += (int)bytes_written;
        }
    }
    if (bytes_written < 1)
    {
        return 0;
    }
    return  bytes_written;
}

int write_opcode_node(FILE* file, const parse_node_ptr p)
{
    // the node is an opcode
    // get the number of bytes needed
    // based on addressing mode of opcode
    const int bytes = get_op_code_byte_count(p);

    // write the opcode byte
    int bytes_written = write_byte(output_file, p->opcode.opcode);  // NOLINT(clang-diagnostic-implicit-int-conversion)
    total_bytes_written += (int)bytes_written;
    if (bytes_written < 1)
    {
        error(error_writing_output_file);
        return 0;
    }

    if (bytes > 0)
    {
        // expand_node the data bytes
        int op = expand_node(p->op[0]);
        if (p->opcode.mode == r)
        {
            // make sure a branch is in range
            op -= (p->opcode.program_counter + 2);

            if (op > 128 || op < -127)
            {
                error(error_value_out_of_range);
                return 0;
            }
            op &= 0xFF;
        }

        data_size = bytes;
        bytes_written = write_data(output_file, op);
        total_bytes_written += (int)bytes_written;
        if (bytes_written < 1)
        {
            error(error_writing_output_file);
            return 0;
        }
    }

    return bytes_written;
}

/// <summary>
/// Generate output to the object file
/// based on node
///
/// This should only be called on the FinalPass
/// when all symbols/macros are resolved
/// </summary>
/// <param name="file"></param>
/// <param name="p">The parseNode.</param>
/// <returns>int.</returns>
int generate_output(FILE* file, const parse_node_ptr p)
{
    int bytes_written;
    const int start_bytes = total_bytes_written;

    // if there is no output file exit
    if (file == NULL || !origin_specified)
        return 0;

    // header
    if (total_bytes_written == 0)
    {
        bytes_written = write_header(file);
        if (bytes_written < 0)
        {
            return 0;
        }
    }

    // program counter pad
    // need if user does
    // .ORG XXXX
    // * = $XXXX + N
    // it fills the memory from previous program counter
    // to the new program counter with 0's
    const int delta = program_counter - origin;
    bytes_written = write_program_counter_pad(output_file, delta);
    if (bytes_written < 0)
    {
        return 0;
    }

    if (p == NULL)
        return total_bytes_written - start_bytes;

    switch (p->type)  // NOLINT(clang-diagnostic-switch-enum)
    {
    case type_str:
        bytes_written = write_string_node(output_file, p);
        break;

    case type_op_code:
        bytes_written = write_opcode_node(output_file, p);
        break;

    default:
        bytes_written = write_data_node(output_file, p);
        break;
    }

    if (bytes_written < 0)
    {
        return 0;
    }
    return total_bytes_written - start_bytes;
}

/// <summary>
/// Gets the op byte count.
/// </summary>
/// <param name="p">The parseNode.</param>
/// <returns>int.</returns>
int get_op_code_byte_count(const parse_node_ptr p)
{
    int byte_count = 0;

    if (p->type != type_op_code)
        return byte_count;

    switch (p->opcode.mode)
    {
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
    case izx:   /* zero page indirect x */
    case izy:   /* zero page indirect y */
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
