// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppClangTidyConcurrencyMtUnsafe
#include <fstream>
#include <map>
#include <vector>

#include "pasm.h"
#include "sym.h"

std::stack<file_line_stack_entry> file_stack;
std::stack<int*> ifdef_stack;
std::vector<symbol_table_ptr> changed_sym_list;

std::ofstream log_file;

FILE* sym_file = nullptr;
FILE* output_file = nullptr;
FILE* list_file = nullptr;
FILE* console = nullptr;
FILE* console_error = nullptr;

bool final_pass = false;
bool ignore_warnings = false;
bool org_origin_specified = false;
bool origin_specified = false;

bool verbose = false;

char** input_files = nullptr;
char* internal_buffer = nullptr;
char* current_file_name = nullptr;
char* directories = nullptr;
char* output_file_name = nullptr;
char* sym_file_name = nullptr;
char* list_file_name = nullptr;
char* log_file_name = nullptr;

int in_macro_definition = 0;
int output_file_format = bin;
int pass = 0;
int program_counter = 0;
int data_size = 0;
int input_file_count = 0;
extern int plus_symbol_table_index;
extern int minus_symbol_table_index;
extern int plus_symbol_table_size;
extern int minus_symbol_table_size;

void init_globals()
{
    while (file_stack.size() > 0) {
        file_stack.pop();
    }
    while (ifdef_stack.size() > 0) {
        ifdef_stack.pop();
    }
    while (macro_params_stack.size() > 0) {
        macro_params_stack.pop();
    }

    changed_sym_list.clear();

    if (log_file.is_open())
        log_file.close();

    sym_file = nullptr;
    output_file = nullptr;
    list_file = nullptr;
    console = nullptr;
    console_error = nullptr;
    final_pass = false;
    ignore_warnings = false;
    origin_specified = false;
    org_origin_specified = false;
    verbose = false;

    input_files = nullptr;
    internal_buffer = nullptr;
    current_file_name = nullptr;
    directories = nullptr;
    output_file_name = nullptr;
    sym_file_name = nullptr;
    list_file_name = nullptr;
    log_file_name = nullptr;
    minus_symbol_table = nullptr;
    minus_symbol_table_index = 0;

    plus_symbol_table_index = 0;
    plus_symbol_table_size = 0;
    plus_symbol_table = NULL;

    minus_symbol_table_index = 0;
    minus_symbol_table_size = 0;
    minus_symbol_table = NULL;

    in_macro_definition = 0;
    output_file_format = bin;
    pass = 0;
    program_counter = 0;
    data_size = 0;
    input_file_count = 0;
}