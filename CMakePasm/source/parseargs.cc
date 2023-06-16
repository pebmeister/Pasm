// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyClangDiagnosticDeprecatedDeclarations
// ReSharper disable CppClangTidyConcurrencyMtUnsafe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "expand.h"
#include "memory.h"
#include "opcodes.h"
#include "parseargs.h"

#include "flex.h"
#include "pasm.h"
#include "sym.h"

void usage(void);

#pragma warning(disable: 4996)
void usage(void)
{
    fprintf(console, "Usage:\n");
    fprintf(console, "  pasm [-o outputfile] [-d sym val] [-dir directory] [-s symbolfile] [-l listfile] [-log logfile] [-i] [-v] [-c64] [-65c02 | -6502] [-?] [-nowarn] inputfile1 inputfile2 ...\n\n");
    fprintf(console, "         -o outputfile   specifies output file\n");
    fprintf(console, "         -d sym value    defines a symbol with a value. This may be specified more than one for multiple symbols\n");
    fprintf(console, "         -dir directory  specifies directories to search for source files and include files separated by ';'\n");
    fprintf(console, "         -s symbolfile   specifies symbol file\n");
    fprintf(console, "         -l listfile     specifies listing file\n");
    fprintf(console, "         -log logfile    specifies log file\n");
    fprintf(console, "         -i              allow illegal instructions\n");
    fprintf(console, "         -v              verbose mode.\n");
    fprintf(console, "         -c64            Commodore program format. Write load address in first 2 bytes of output file\n");
    fprintf(console, "         -6502           use 6502 instruction set (default)\n");
    fprintf(console, "         -65C02          use 65C02 instruction set\n");
    fprintf(console, "         -nowarn         turn off warnings\n");
    fprintf(console, "         -?              print this help\n");
}

int parse_arguments(const int argc, char* argv[])
{
    console = stdout;
    console_error = stderr;
    yyout = console;

    for (int arg = 0; arg < argc; arg++)
    {
        fprintf(console, "%s ", argv[arg]);
    }
    fprintf(console, "\n");

    bool illegal_specified = false;
    bool cpu_specified = false;

    input_files = (char**)MALLOC(sizeof(char*) * argc);
    int arg_index = 1;

    if (input_files == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }

    cpu_mode = cpu_6502;

    // Init Ex 
    init_expander();

    while (arg_index < argc)
    {
        // help
        if (stricmp(argv[arg_index], "-?") == 0 || stricmp(argv[arg_index], "/?") == 0
            || (stricmp(argv[arg_index], "-h") == 0 || stricmp(argv[arg_index], "/h") == 0))
        {
            usage();
            exit(0);
        }

        // get output file
        if (stricmp(argv[arg_index], "-o") == 0 || stricmp(argv[arg_index], "/o") == 0)
        {
            arg_index++;
            if (arg_index >= argc)
            {
                error(error_missing_output_file);
                usage();
                exit(-1);
            }
            if (output_file_name != NULL)
            {
                error(error_output_file_specified_more_than_once);
                usage();
                exit(-1);
            }
            output_file_name = argv[arg_index];
            arg_index++;
            continue;
        }

        // get symbol file
        if (stricmp(argv[arg_index], "-s") == 0 || stricmp(argv[arg_index], "/s") == 0)
        {
            arg_index++;
            if (arg_index >= argc)
            {
                error(error_missing_symbol_file);
                usage();
                exit(-1);
            }
            if (sym_file_name != NULL)
            {
                error(error_symbol_file_specified_more_than_once);
                usage();
                exit(-1);
            }
            sym_file_name = argv[arg_index];
            arg_index++;
            continue;
        }

        // directory paths separated by ;
        if (stricmp(argv[arg_index], "-dir") == 0 || stricmp(argv[arg_index], "/dir") == 0)
        {
            arg_index++;
            if (arg_index >= argc)
            {
                error(error_missing_parameter);
                usage();
                exit(-1);
            }
            if (directories != NULL)
            {
                error(error_symbol_file_specified_more_than_once);
                usage();
                exit(-1);
            }
            directories = argv[arg_index];
            arg_index++;
            continue;
        }

        // log file
        if (stricmp(argv[arg_index], "-log") == 0 || stricmp(argv[arg_index], "/log") == 0)
        {
            arg_index++;
            if (arg_index >= argc)
            {
                error(error_missing_log_file);
                usage();
                exit(-1);
            }
            if (log_file_name != NULL)
            {
                error(error_log_file_specified_more_than_once);
                usage();
                exit(-1);
            }
            log_file_name = argv[arg_index];
            arg_index++;
            continue;
        }

        // list file
        if (stricmp(argv[arg_index], "-l") == 0 || stricmp(argv[arg_index], "/l") == 0)
        {
            arg_index++;
            if (arg_index >= argc)
            {
                error(error_missing_list_file);
                usage();
                exit(-1);
            }
            if (list_file_name != NULL)
            {
                error(error_list_file_specified_more_than_once);
                usage();
                exit(-1);
            }
            list_file_name = argv[arg_index];
            arg_index++;
            continue;
        }

        // C64 file mode - write load address at start of file
        if (stricmp(argv[arg_index], "-c64") == 0 || stricmp(argv[arg_index], "/c64") == 0)
        {
            if (output_file_format == c64)
            {
                error(error_c64_output_format_specified_more_than_once);
                usage();
                exit(-1);
            }
            output_file_format = c64;
            arg_index++;
            continue;
        }

        // verbose
        if (stricmp(argv[arg_index], "-v") == 0 || stricmp(argv[arg_index], "/v") == 0)
        {
            if (verbose)
            {
                error(error_verbose_specified_more_than_once);
                usage();
                exit(-1);
            }

            verbose = 1;
            arg_index++;
            continue;
        }

        // 65c02 opcodes enabled
        if (stricmp(argv[arg_index], "-65c02") == 0 || stricmp(argv[arg_index], "/65c02") == 0)
        {
            if (cpu_specified)
            {
                error(error_instruction_set_specified_more_than_once);
                usage();
                exit(-1);
            }
            cpu_specified = true;
            cpu_mode = cpu_65C02;
            arg_index++;
            continue;
        }

        // 6502 opcodes specified
        if (stricmp(argv[arg_index], "-6502") == 0 || stricmp(argv[arg_index], "/6502") == 0)
        {
            if (cpu_specified)
            {
                error(error_instruction_set_specified_more_than_once);
                usage();
                exit(-1);
            }
            cpu_specified = true;
            cpu_mode = cpu_6502;
            arg_index++;
            continue;
        }

        // illegal opcodes allowed specified
        if (stricmp(argv[arg_index], "-i") == 0 || stricmp(argv[arg_index], "/i") == 0)
        {
            if (illegal_specified)
            {
                error(error_illegal_opcodes_specified_more_than_once);
                usage();
                exit(-1);
            }
            illegal_specified = true;
            allow_illegal_op_codes = true;
            arg_index++;
            continue;
        }

        // no warnings
        if (stricmp(argv[arg_index], "-nowarn") == 0 || stricmp(argv[arg_index], "/nowarn") == 0)
        {
            if (ignore_warnings)
            {
                error(error_ignore_warnings_specified_more_than_once);
                usage();
                exit(-1);
            }
            ignore_warnings = true;
            arg_index++;
            continue;
        }

        // define symbol
        if (stricmp(argv[arg_index], "-d") == 0 || stricmp(argv[arg_index], "/d") == 0)
        {
            arg_index++;
            if (arg_index >= argc)
            {
                error(error_invalid_parameters);
                usage();
                exit(-1);
            }
            char* sym_name = argv[arg_index];

            arg_index++;
            if (arg_index >= argc)
            {
                error(error_invalid_parameters);
                usage();
                exit(-1);
            }
            const char* val = argv[arg_index];

            int sym_val = 0;

            if (val[0] == '$')
            {
                sym_val = (int)strtol(val + 1, NULL, 16);
            }
            else if (val[0] == '%')
            {
                sym_val = (int)strtol(val + 1, NULL, 2);
            }
            else if (strlen(val) > 2)
            {
                if (val[0] == '0' && val[1] == 'x')
                {
                    sym_val = (int)strtol(val + 2, NULL, 16);
                }
                else if (val[0] == '0' && val[1] == 'b')
                {
                    sym_val = (int)strtol(val + 2, NULL, 2);
                }
            }
            else
            {
                sym_val = (int)strtol(val, NULL, 10);
            }

            // ReSharper disable once CppLocalVariableMayBeConst
            symbol_table_ptr sym = add_symbol(sym_name);
            if (sym != NULL)
            {
                set_symbol_value(sym, sym_val);
                sym->is_initialized = true;
            }
            arg_index++;
            continue;
        }

        // add to the input file list
        input_files[input_file_count++] = argv[arg_index];
        arg_index++;
    }

    if (input_file_count == 0)
    {
        error(error_no_input_file_specified);
        usage();
        exit(- 1);
    }

    return input_file_count;
}
