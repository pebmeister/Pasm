// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppClangTidyConcurrencyMtUnsafe
#include <stdio.h>
#include <stdlib.h>

#include "pasm.h"
#include "stacks.h"

stack_ptr file_stack = NULL;
stack_ptr ifdef_stack = NULL;
stack_ptr changed_sym_stack = NULL;

FILE* log_file = NULL;
FILE* sym_file = NULL;
FILE* output_file = NULL;
FILE* list_file = NULL;
FILE* console = NULL;
FILE* console_error = NULL;
bool final_pass = false;
bool ignore_warnings = false;
bool origin_specified = false;
bool verbose = false;

char** input_files = NULL;
char* internal_buffer = NULL;
char* current_file_name = NULL;
char* directories = NULL;
char* output_file_name = NULL;
char* sym_file_name = NULL;
char* list_file_name = NULL;
char* log_file_name = NULL;

int in_macro_definition = 0;
int output_file_format = bin;
int pass = 0;
int program_counter = 0;
int data_size = 0;
int input_file_count = 0;

void init_globals()
{
    file_stack = NULL;
    ifdef_stack = NULL;
    changed_sym_stack = NULL;

    log_file = NULL;
    sym_file = NULL;
    output_file = NULL;
    list_file = NULL;
    console = NULL;
    console_error = NULL;
    final_pass = false;
    ignore_warnings = false;
    origin_specified = false;
    verbose = false;

    input_files = NULL;
    internal_buffer = NULL;
    current_file_name = NULL;
    directories = NULL;
    output_file_name = NULL;
    sym_file_name = NULL;
    list_file_name = NULL;
    log_file_name = NULL;

    in_macro_definition = 0;
    output_file_format = bin;
    pass = 0;
    program_counter = 0;
    data_size = 0;
    input_file_count = 0;
}