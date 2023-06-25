// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppClangTidyConcurrencyMtUnsafe
// ReSharper disable CppClangTidyCertErr33C
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "expand.h"
#include "file.h"
#include "flex.h"
#include "genlist.h"
#include "genoutput.h"
#include "memory.h"
#include "pasm.h"
#include "pasm.tab.h"
#include "stacks.h"

const int max_passes = 20;

int program_origin_default = 0x1000;

/**
 * \brief Reset the Lexer for each pass
 */
void reset_lex(void)
{
    error_count = 0;

    last_label = nullptr;

    sym_value_changed = 0;

    // reset the Program Counter
    program_counter = program_origin_default;

    // reset End flag
    end_expansion = 0;

    // reset Org flag
    origin_specified = false;

    if (internal_buffer == nullptr)
    {
        internal_buffer = (char*)MALLOC(max_line_len);
        if (internal_buffer == nullptr)
        {
            error(error_out_of_memory);
            exit(-1);
        }
    }

    // reset the the head node
    current_node = head_node = allocate_node(0);
    if (current_node == nullptr)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    head_node->type = type_head_node;

    reset_macro_dict();

    // initialize expander
    init_expander();

    if (changed_sym_stack == nullptr)
    {
        changed_sym_stack = create_stack(sizeof(symbol_table));
    }
    else
    {
        changed_sym_stack->clear(changed_sym_stack->instance);
    }
    if (file_stack == nullptr)
    {
        file_stack = create_stack(sizeof(file_line_stack_entry));
    }
    else
    {
        file_stack->clear(file_stack->instance);
    }
    if (ifdef_stack == nullptr)
    {
        ifdef_stack = create_stack(sizeof(int));
    }
    else
    {
        ifdef_stack->clear(ifdef_stack->instance);
    }
    in_macro_definition = 0;
}


/**
 * \brief Parse one pass iterating through all files
 */
void parse_pass(void)
{
    reset_lex();

    // loop through each input line_content
    for (int in_file_index = 0; in_file_index < input_file_count; in_file_index++)
    {
        // initialize line number and set line_content name
        yylineno = 0;

        current_file_name = input_files[in_file_index];
        yyin = open_file(current_file_name, "r");
        if (yyin == nullptr)
        {
            error2(error_cant_open_input_file, current_file_name);
            exit(-1);
        }

        if (log_file != nullptr)
        {
            fprintf(log_file, "Current File %s\n", current_file_name);
        }
        if (verbose)
        {
            fprintf(console, "Current File %s\n", current_file_name);
        }

        // seek to start
        fseek(yyin, 0, SEEK_SET);

        // reset parser
        yyrestart(yyin);

        // parse the line_content
        yyparse();

        // close the line_content
        fclose(yyin);

        yyin = nullptr;
    }

    // free the parse tree
    free_parse_tree();

    if (current_scope)
    {
        FREE(current_scope);
        current_scope = nullptr;
    }
}

int assemble(void)
{
    error_count = 0;

    delete_list_table();
    delete_file_lines();

    if (verbose)
    {
        fprintf(console, "\n");
    }

    // initialize line number and set line_content name
    yylineno = 0;
    final_pass = false;

    pass = 1;

    if (log_file_name != nullptr)
    {
        log_file = open_file(log_file_name, "w");
        if (log_file == nullptr)
        {
            error(error_opening_log_file);
            return -1;
        }
    }

    int clean_pass_count = 0;
    do
    {
        if (verbose)
        {
            fprintf(console, "Pass %d\n", pass);
        }

        parse_pass();

        const int unresolved_count = unresolved_symbol_count();
        if (error_count == 0 && sym_value_changed == 0 && unresolved_count == 0)
            clean_pass_count++;
        else
            clean_pass_count = 0;

        if (verbose && clean_pass_count == 0 && pass > 1)
        {
            fprintf(console, "%d unresolved symbols %d symbols value changed\n\n", unresolved_count, sym_value_changed);
            if (unresolved_count > 0)
            {
                fprintf(console, "Unresolved symbols:\n");
                dump_unresolved_symbols(console);
            }
            if (sym_value_changed > 0)
            {
                fprintf(console, "changed symbols:\n");
                dump_changed_symbols(console);
            }
        }
        pass++;
    } while (error_count == 0 && pass < max_passes && clean_pass_count < 1);

    if (pass >= max_passes)
    {
        if (sym_value_changed > 0)
        {
            fprintf(console, "check symbols:\n");
            dump_changed_symbols(console);
        }
        error(error_maximum_number_of_passes_exceeded);
        return -1;
    }

    if (error_count > 0)
    {
        fprintf(console_error, "%d error(s)\n", error_count);
        return -1;
    }

    if (verbose)
        fprintf(console, "Final Pass\n");

    // set final pass flag
    final_pass = true;

    if (output_file_name != nullptr)
    {
        output_file = fopen(output_file_name, "wb");
        if (output_file == nullptr)
        {
            error(error_opening_output_file);
            return -1;
        }
    }

    parse_pass();

    // close output line_content
    if (output_file != nullptr)
    {
        fclose(output_file);
        fprintf(console, "\n%d bytes written to %s\n\n", total_bytes_written, output_file_name);
    }

    // generate symbol line_content
    if (sym_file_name != nullptr)
    {
        sym_file = fopen(sym_file_name, "w");
        if (sym_file == nullptr)
        {
            error(error_opening_symbol_file);
            return -1;
        }

        // display the symbols
        dump_symbols(sym_file);

        fclose(sym_file);
    }


    // generate the list file
    if (list_file_name != nullptr)
    {
        list_file = fopen(list_file_name, "w");
        if (list_file == nullptr)
        {
            error(error_opening_list_file);
        }
        else
        {
            generate_list_file(list_file);
            fclose(list_file);
        }
    }

    // generate verbose output
    if (verbose)
    {
        dump_symbols(console);
        reset_file_lines();
        generate_list_file(console);
    }

    delete_list_table();

    return 0;
}

