#pragma once
#include <stdbool.h>
#include "stacks.h"

enum
{
    max_line_len = 2048
};

enum
{
    none,
    bin,
    c64
};

typedef struct file_line_stack_entry
{
    char* file;
    int line;
} file_line_stack_entry;

extern void reset_lex(void);
extern void parse_pass(void);
extern int assemble(void);

// Used in flex generated line_content
extern bool final_pass;
extern bool ignore_warnings;
extern bool origin_specified;
extern bool verbose;

extern int data_size;
extern int output_file_format;
extern int in_macro_definition;
extern int pass;
extern int program_counter;
extern int input_file_count;
extern int program_origin_default;

extern stack_ptr file_stack;
extern stack_ptr ifdef_stack;
extern stack_ptr changed_sym_stack;

extern char* internal_buffer;
extern char* current_file_name;
extern char* directories;
extern char** input_files;
extern char* output_file_name;
extern char* sym_file_name;
extern char* list_file_name;
extern char* log_file_name;

extern FILE* console;
extern FILE* console_error;