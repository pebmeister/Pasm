#pragma once
#include <stdbool.h>
#include <string>
#include <map>

#include "stacks.h"

typedef struct plus_minus_sym
{
    const char* file;
    int line;
    int value;
} plus_minus_symbol;

typedef struct symbol_table
{
    int value;
    int times_accessed;
    bool is_initialized;
    bool is_macro_name;
    bool is_macro_param;
    bool is_label;
    bool is_local;
    bool is_plus_minus;
    bool is_var;
    char* scope;
    char* name;
    char* fullname;
    void* macro_node;
} symbol_table, * symbol_table_ptr;

extern int print_symbol(symbol_table_ptr sym, FILE* file);

extern int find_minus_symbol(int depth, const char* file, int line);
extern int find_plus_symbol(int depth, const char* file, int line);
extern symbol_table_ptr look_up_symbol(const char*);
extern char* format_local_sym(char* name, const char* label);
extern char* format_local_plus_minus_sym(char* name, char* label);
extern symbol_table_ptr add_symbol(char* name);
extern symbol_table_ptr set_symbol_value(symbol_table_ptr sym, int value);
extern void add_minus_symbol(const char* file, int line, int value);
extern void add_plus_symbol(const char* file, int line, int value);
extern int find_minus_symbol_definition(const char* file, int line);
extern int find_plus_symbol_definition(const char* file, int line);
extern void push_macro_params(void);
extern void pop_macro_params(void);
extern int unresolved_symbol_count(void);
extern void dump_symbols(FILE* file);
extern void dump_unresolved_symbols(FILE* file);
extern void dump_changed_symbols(FILE* file);

extern plus_minus_symbol* minus_symbol_table;
extern plus_minus_symbol* plus_symbol_table;
extern std::map<std::string, symbol_table_ptr> symbol_dictionary;
extern stack_ptr macro_params_stack;
extern int max_macro_param;

