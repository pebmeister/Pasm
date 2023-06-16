// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppZeroConstantCanBeReplacedWithNullptr
// ReSharper disable CppClangTidyCertErr33C
// ReSharper disable CppClangTidyConcurrencyMtUnsafe
// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyClangDiagnosticShorten64To32
// ReSharper disable CppPointerConversionDropsQualifiers
// ReSharper disable CppClangTidyClangDiagnosticIncompatiblePointerTypesDiscardsQualifiers

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary.h"
#include "error.h"
#include "expand.h"
#include "memory.h"
#include "pasm.h"
#include "stacks.h"
#include "sym.h"

#pragma warning(disable: 4996 4267 4090 6386)

typedef struct  macro_stack_entry
{
    int num_nodes;
    int* values;
} macro_stack_entry;

static void sanitize_symbol(symbol_table_ptr symbol);
static int get_max_macro_parameter(void);

dictionary_ptr symbol_dictionary = NULL; 
stack_ptr macro_params_stack = NULL;

// must be less than PLUS_MINUS_TABLE_SIZE
#define GROW_THRESHOLD 100
#define PLUS_MINUS_TABLE_SIZE 1000

int plus_symbol_table_index = 0;
int plus_symbol_table_size = 0;
struct plus_minus_sym * plus_symbol_table = NULL;

int minus_symbol_table_index = 0;
int minus_symbol_table_size = 0;
struct plus_minus_sym* minus_symbol_table = NULL;

static int compare_plus_minus_sym_value(const struct plus_minus_sym* a, const char* file, int line);
static int compare_plus_minus_sym(const struct plus_minus_sym* a, const struct plus_minus_sym* b);
static int plus_minus_sym_cmp_function(const void* a, const void* b);

int compare_plus_minus_sym(const struct plus_minus_sym* a, const struct plus_minus_sym* b)
{
    const int result = stricmp(a->file, b->file);
    if (result != 0) return result;
    return a->line - b->line;
}

int plus_minus_sym_cmp_function(const void* a, const void* b)
{
    return compare_plus_minus_sym((struct plus_minus_sym*)a, (struct plus_minus_sym*)b);
}

int compare_plus_minus_sym_value(const struct plus_minus_sym* a, const char* file, const int line)
{
    const struct plus_minus_sym b = { file, line, 0 };
    return compare_plus_minus_sym(a, &b);
}

symbol_table_ptr add_symbol(char* name)
{
    if (name == NULL) return NULL;
    if (in_macro_definition > 0) return NULL;
    if (stricmp(name, "A") == 0) return NULL;

    symbol_table sym = { 0 };
    char* temp_section = current_scope;

    if (symbol_dictionary == NULL)
    {
        symbol_dictionary = dict_create(sizeof(symbol_table), 0);
        if (symbol_dictionary == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
    }

    if (strstr(name, "."))
        current_scope = NULL;

    if (name[0] == '.')
    {
        name++;
    }
    sym.is_macro_param = name[0] == '\\';
    sym.is_local = name[0] == '@';
    if (sym.is_local && last_label != NULL)
    {
        name = format_local_sym(name, last_label);
    }

    size_t len = strlen(name);
    if (name[len - 1] == ':')
        name[len - 1] = 0;

    symbol_table_ptr tmp_ptr = look_up_symbol(name);
    if (tmp_ptr != NULL)
    {
        current_scope = temp_section;
        return tmp_ptr;
    }

    sym.name = STRDUP(name);
    if (sym.name == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }

    if (current_scope)
    {
        sym.scope = STRDUP(current_scope);
        if (sym.scope == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
    }
    if (sym.scope)
    {
        len = strlen(sym.name) + strlen(sym.scope) + 2;
        sym.fullname = (char*) MALLOC(len);
        if (sym.fullname == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
        sprintf(sym.fullname, "%s.%s", sym.scope, sym.name);
    }
    else
    {
        sym.fullname = STRDUP(sym.name);
        if (sym.fullname == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
    }

    if (sym.name[0] == '@')
    {
        sym.is_initialized = true;
        sym.value = 0;
    }

    current_scope = temp_section;
    tmp_ptr = (symbol_table_ptr)dict_insert(&symbol_dictionary, sym.fullname, &sym);
    sanitize_symbol(tmp_ptr);

    return tmp_ptr;
}

symbol_table_ptr look_up_macro_param(const int param_number)
{
    char macro_param_name[sizeof(int) + 5] = {0};

    sprintf(macro_param_name, "\\%d", param_number);
    return (symbol_table_ptr) dict_search(symbol_dictionary, macro_param_name);
}

/// <summary>
/// Get SymbolTable for symbol.
/// </summary>
/// <param name="name">The name.</param>
/// <returns>int.</returns>
symbol_table_ptr look_up_symbol(const char* name)
{ 
    if (symbol_dictionary == NULL)
    {
        symbol_dictionary = dict_create(sizeof(symbol_table), 0);
        if (symbol_dictionary == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
        return NULL;
    }

    if (current_scope)
    {
        const int len = (int)strlen(current_scope) + (int)strlen(name) + 2;
        char* temp_name = (char*)MALLOC(len);
        if (temp_name == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
        sprintf(temp_name, "%s.%s", current_scope, name);
        // ReSharper disable once CppLocalVariableMayBeConst
        symbol_table_ptr tmp_ptr = (symbol_table_ptr)dict_search(symbol_dictionary, temp_name);
        FREE(temp_name);
        if (tmp_ptr)
        {
            tmp_ptr->times_accessed++;
            return tmp_ptr;
        }
    }

    // ReSharper disable once CppLocalVariableMayBeConst
    symbol_table_ptr tmp_ptr = (symbol_table_ptr)dict_search(symbol_dictionary, name);
    if (tmp_ptr)
        tmp_ptr->times_accessed++;
    return tmp_ptr;
} 

symbol_table_ptr set_symbol_value(const symbol_table_ptr sym, const int value)
{
    if ((sym != NULL) && (sym->value != value))
    {
        if (((value & ~0xFFFF) != 0) && !sym->is_var)
        {
            error(error_value_out_of_range);
            return NULL;
        }
        sym->value = value;
        if (!sym->is_macro_param && !sym->is_var)
        {
            sym_value_changed++;
            if (changed_sym_stack == NULL)
            {
                changed_sym_stack = create_stack(sizeof(symbol_table));
            }
            changed_sym_stack->push(changed_sym_stack->instance, sym);
        }
    }
    return sym;
}

int find_plus_symbol_definition(const char* file, const int line)
{
    if (plus_symbol_table_index <= 0) return -1;

    for (int index = 0; index < plus_symbol_table_index; ++index)
    {
        if (compare_plus_minus_sym_value(&plus_symbol_table[index], file, line) == 0)
        {
            return index;
        }
    }
    return -1;
}

int find_minus_symbol_definition(const char* file, const int line)
{
    if (minus_symbol_table_index <= 0) return -1;

    for (int index = 0; index < minus_symbol_table_index; ++index)
    {
        if (compare_plus_minus_sym_value(&minus_symbol_table[index], file, line) == 0)
        {
            return index;
        }
    }
    return -1;
}

int find_minus_symbol(const int depth, const char* file, const int line)
{
    if (minus_symbol_table_index <= 0) return -1;
    int found_index = -1;
    for (int index = 0; index < minus_symbol_table_index; ++index)
    {
        if (minus_symbol_table[index].file == NULL)
            continue;

        if (stricmp(file, minus_symbol_table[index].file) != 0)
            continue;

        if (minus_symbol_table[index].line <= line)
        {
            found_index = index;
        }
    }
    if (found_index >= 0)
    {
        found_index = found_index + 1 - depth;
        if (found_index >= 0 || found_index < minus_symbol_table_index)
            return found_index;
    }
    return -1;
}

int find_plus_symbol(const int depth, const char* file, const int line)
{
    if (plus_symbol_table_index <= 0) return -1;
    int found_index = -1;
    for (int index = 0; index < plus_symbol_table_index; ++index)
    {
        if (stricmp(file, plus_symbol_table[index].file) != 0)
            continue;

        if (plus_symbol_table[index].line > line)
        {
            found_index = index;
            break;
        }
    }
    if (found_index >= 0)
    {
        found_index = found_index -1 + depth;
        if (found_index >= 0 || found_index < plus_symbol_table_index)
            return found_index;
    }
    return -1;
}

void add_minus_symbol(const char* file, const int line, const int value)
{
    if (macro_params_stack != NULL && macro_params_stack->instance->index >= 0)
    {
        error(error_plus_sym_not_allowed_in_macro);
        return;
    }

    if (minus_symbol_table_size - minus_symbol_table_index < GROW_THRESHOLD)
    {
        minus_symbol_table_size += PLUS_MINUS_TABLE_SIZE;
        struct plus_minus_sym* temp_ptr = (struct plus_minus_sym*)REALLOC(minus_symbol_table, sizeof(struct plus_minus_sym) * minus_symbol_table_size);
        if (temp_ptr == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
        char* memory_clear_location = (char*)temp_ptr;
        memory_clear_location += sizeof(struct plus_minus_sym) * (minus_symbol_table_size - PLUS_MINUS_TABLE_SIZE);
        memset(memory_clear_location, 0, sizeof(struct plus_minus_sym) * PLUS_MINUS_TABLE_SIZE);

        minus_symbol_table = temp_ptr;
    }

    minus_symbol_table[minus_symbol_table_index].file = STRDUP(file);
    minus_symbol_table[minus_symbol_table_index].line = line;
    minus_symbol_table[minus_symbol_table_index].value = value;
    ++minus_symbol_table_index;

    qsort(minus_symbol_table, minus_symbol_table_index, sizeof(struct plus_minus_sym), plus_minus_sym_cmp_function);
}

void add_plus_symbol(const char* file, const int line, const int value)
{
    if (macro_params_stack != NULL && macro_params_stack->instance->index >= 0)
    {
        error(error_plus_sym_not_allowed_in_macro);
        return;
    }

    if (plus_symbol_table_size - plus_symbol_table_index < GROW_THRESHOLD)
    {
        plus_symbol_table_size += PLUS_MINUS_TABLE_SIZE;
        struct plus_minus_sym* temp_ptr = (struct plus_minus_sym*)REALLOC(plus_symbol_table, sizeof(struct plus_minus_sym) * plus_symbol_table_size);
        if (temp_ptr == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
        plus_symbol_table = temp_ptr;
    }

    plus_symbol_table[plus_symbol_table_index].file = STRDUP(file);
    plus_symbol_table[plus_symbol_table_index].line = line;
    plus_symbol_table[plus_symbol_table_index].value = value;
    ++plus_symbol_table_index;

    qsort(plus_symbol_table, plus_symbol_table_index, sizeof(struct plus_minus_sym), plus_minus_sym_cmp_function);
}

/// <summary>
/// Gets the maximum macro parameter.
/// </summary>
/// <returns>int.</returns>
int get_max_macro_parameter(void)
{
    return max_macro_param;
}

void push_macro_params(void)
{
    if (macro_params_stack == NULL)
    {
        macro_params_stack = create_stack(sizeof(macro_stack_entry));
    }
    const int max_macro = get_max_macro_parameter();

    macro_stack_entry macro_stack_param_entry = { 0, NULL };
    
    if (max_macro < 0)
    {
        macro_stack_param_entry.num_nodes = 0;
    }
    else
    {
        macro_stack_param_entry.num_nodes = max_macro + 1;
        macro_stack_param_entry.values = (int*) MALLOC(((unsigned long long)max_macro + 1) * sizeof(long));

        macro_stack_param_entry.values[0] = 0;
        for (int index = 1; index <= max_macro; index++)
        {
            // ReSharper disable once CppLocalVariableMayBeConst
            symbol_table_ptr tmp_ptr = look_up_macro_param(index);
            if (tmp_ptr != NULL)
            {
                macro_stack_param_entry.values[index] = tmp_ptr->value;
            }
            else
            {
                error(error_macro_parameter_not_found);
                return;
            }
        }
    }

    macro_params_stack->push(macro_params_stack->instance, &macro_stack_param_entry);
}

void pop_macro_params(void)
{
    if (macro_params_stack == NULL || macro_params_stack->instance->index < 0)
    {
        error(error_macro_parameter_under_flow);
        exit(-1);
    }

    macro_stack_entry* macro_stack_param_entry_ptr = (macro_stack_entry*)macro_params_stack->pop(macro_params_stack->instance);

    for (int index = 1; index < macro_stack_param_entry_ptr->num_nodes; index++)
    {
        const symbol_table_ptr tmp_ptr = look_up_macro_param(index);
        if (tmp_ptr != NULL)
        {
            set_symbol_value(tmp_ptr, macro_stack_param_entry_ptr->values[index]);
        }
        else
        {
            error(error_macro_parameter_not_found);
            return;
        }
    }
    if (macro_stack_param_entry_ptr->num_nodes > 0)
    {
        FREE(macro_stack_param_entry_ptr->values);
        macro_stack_param_entry_ptr->values = NULL;
    }
    macro_stack_param_entry_ptr->num_nodes = 0;

}

void sanitize_symbol(const symbol_table_ptr symbol)
{
    char* combined_section;

    if (symbol->is_macro_param)
    {
        if (symbol->scope)
        {
            FREE(symbol->scope);
            symbol->scope = NULL;
            return;
        }
    }

    if (strstr(symbol->name, ".") == NULL)
        return;

    int len = (int)strlen(symbol->name);
    while (symbol->name[len] != '.')
        len--;
    symbol->name[len] = 0;
    char* temp_section = STRDUP(symbol->name);
    char* temp_name = STRDUP(&symbol->name[len + 1]);
    if (temp_name == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    if (symbol->name)
        FREE(symbol->name);

    symbol->name = temp_name;
    if (symbol->scope)
    {
        len = (int)strlen(symbol->scope) + (int)strlen(temp_section) + 2;
        combined_section = (char*)MALLOC(len);
        if (combined_section == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
        sprintf(combined_section, "%s.%s", symbol->scope, temp_section);
        FREE(symbol->scope);
        symbol->scope = combined_section;
    }
    else
    {
        len = (int)strlen(temp_section) + 1;
        combined_section = (char*)MALLOC(len);
        if (combined_section == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
        strcpy(combined_section, temp_section);
        symbol->scope = combined_section;
    }
    FREE(temp_section);
}

char* format_local_sym(char* name, const char* label)
{
    const int new_len = strlen(last_label) + 1 + strlen(name) + 1 + 2;  // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
    char* new_name = (char*)MALLOC(new_len);
    if (new_name)
    {
        sprintf(new_name, "%s__%s", label, name);
    }
    name = STRDUP(new_name);
    FREE(new_name);
    return name;
}

int print_symbol(const element_ptr e, FILE* file)
{
    const symbol_table_ptr sym = (symbol_table_ptr) e->value;
    fprintf(file, "\n");
    fprintf(file, "    name:           %s\n", sym->name);
    fprintf(file, "    fullname:       %s\n", sym->fullname);
    fprintf(file, "    value:          %d\n", sym->value);
    fprintf(file, "    macro_node:     %p\n", sym->macro_node);
    fprintf(file, "    is_label:       %s\n", sym->is_label ? "true" : "false");
    fprintf(file, "    is_macro_name:  %s\n", sym->is_macro_name ? "true" : "false");
    fprintf(file, "    is_initialized: %s\n", sym->is_initialized ? "true" : "false");
    fprintf(file, "    is_local:       %s\n", sym->is_local ? "true" : "false");
    fprintf(file, "    is_var:         %s\n", sym->is_var ? "true" : "false");
    fprintf(file, "    is_minus:       %s\n", sym->is_minus ? "true" : "false");
    fprintf(file, "\n");
    return 0;
}

typedef struct sym_entry
{
    char* name;
    int value;
} sym_entry;

int symbol_compare(const void*a, const void* b)
{
    const sym_entry* aa = (sym_entry*)a;
    const sym_entry* bb = (sym_entry*)b;

    return aa->value - bb->value; 
}

void dump_symbols(FILE* file)
{
    if (symbol_dictionary == NULL)
        return;

    const int sz = symbol_dictionary->number_elements;
    int count = 0;
    sym_entry* entry_array = (sym_entry*)MALLOC(sz * sizeof(sym_entry));
    if (entry_array == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    sym_entry* current_entry = entry_array;
    for (int index = 0; index < symbol_dictionary->size; index++)
    {
        for (element_ptr element = symbol_dictionary->table[index]; element != 0; element = element->next)
        {
            const symbol_table_ptr sym = (symbol_table_ptr)element->value;
            if (sym->is_var == false && sym->is_macro_param == false && sym->is_macro_name == false)
            {
                current_entry->name = (char*)element->key;
                current_entry->value = sym->value;
                current_entry++;
                count++;
            }
        }
    }

    qsort(entry_array, count, sizeof(sym_entry), symbol_compare);

    current_entry = entry_array;
    int column = 1;
    for (int i = 0; i < count; ++i)
    {
        const int columns = 5;
        const char* name = current_entry->name;
        const int value = current_entry->value;
        fprintf(file, "%15s $%4.4X  ", name, value);
        if (column == columns)
        {
            fprintf(file, "\n");
            column = 1;
        }
        else
        {
            column++;
        }
        current_entry++; 
    }
    fprintf(file, "\n");

    FREE(entry_array);
}

void dump_changed_symbols(FILE* file)
{
    int column = 1;
    for (int i = 0; i <= changed_sym_stack->instance->index; ++i)
    {
        const symbol_table_ptr current_entry = (symbol_table_ptr) changed_sym_stack->item_at(changed_sym_stack->instance, i);

        const int columns = 5;
        const char* name = current_entry->name;
        const int value = current_entry->value;
        fprintf(file, "%15s $%4.4X  ", name, value);
        if (column == columns)
        {
            fprintf(file, "\n");
            column = 1;
        }
        else
        {
            column++;
        }
    }
    fprintf(file, "\n");
    fprintf(file, "\n");

}

void dump_unresolved_symbols(FILE* file)
{
    if (symbol_dictionary == NULL)
        return;

    const int sz = symbol_dictionary->number_elements;
    int count = 0;
    sym_entry* entry_array = (sym_entry*)MALLOC(sz * sizeof(sym_entry));
    if (entry_array == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    sym_entry* current_entry = entry_array;
    for (int index = 0; index < symbol_dictionary->size; index++)
    {
        for (element_ptr element = symbol_dictionary->table[index]; element != 0; element = element->next)
        {
            const symbol_table_ptr sym = (symbol_table_ptr)element->value;
            if (sym->is_var == false  && sym->is_initialized == false)
            {
                current_entry->name = (char*)element->key;
                current_entry->value = sym->value;
                current_entry++;
                count++;
            }
        }
    }

    qsort(entry_array, count, sizeof(sym_entry), symbol_compare);

    current_entry = entry_array;
    int column = 1;
    for (int i = 0; i < count; ++i)
    {
        const int columns = 5;
        const char* name = current_entry->name;
        const int value = current_entry->value;
        fprintf(file, "%15s $%4.4X  ", name, value);
        if (column == columns)
        {
            fprintf(file, "\n");
            column = 1;
        }
        else
        {
            column++;
        }
        current_entry++;
    }
    fprintf(file, "\n");

    FREE(entry_array);
}

int unresolved_symbol_count(void)
{
    int count = 0;

    if (symbol_dictionary == NULL)
        return 0;

    for (int index = 0; index < symbol_dictionary->size; index++)
    {
        for (element_ptr elem = symbol_dictionary->table[index]; elem; elem = elem->next)
        {
            const symbol_table_ptr sym = (symbol_table_ptr) elem->value;

            if (sym->is_initialized == false && sym->is_macro_name == false &&
                sym->is_var == false && sym->is_macro_param == false)
            {
                count++;
            }
        }
    }

    return count;
}
