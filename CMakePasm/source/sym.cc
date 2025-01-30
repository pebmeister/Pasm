// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppZeroConstantCanBeReplacedWithnullptr
// ReSharper disable CppClangTidyCertErr33C
// ReSharper disable CppClangTidyConcurrencyMtUnsafe
// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyClangDiagnosticShorten64To32
// ReSharper disable CppPointerConversionDropsQualifiers
// ReSharper disable CppClangTidyClangDiagnosticIncompatiblePointerTypesDiscardsQualifiers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#include "error.h"
#include "expand.h"
#include "memory.h"
#include "pasm.h"
#include "sym.h"

#include <flex.h>
bool is_local_sym(const char* name);

#pragma warning(disable: 4996 4267 4090 6386)

std::string ToKey(char* key);
static void sanitize_symbol(symbol_table_ptr symbol);
static int get_max_macro_parameter(void);

std::allocator<symbol_table> symbol_table_allocator;
std::allocator<plus_minus_sym> plus_minus_sym_allocator;

std::map<std::string, symbol_table*> symbol_dictionary; 
std::stack<macro_stack_entry*> macro_params_stack;

// must be less than PLUS_MINUS_TABLE_SIZE
#define GROW_THRESHOLD 100
#define PLUS_MINUS_TABLE_SIZE 5000

int plus_symbol_table_index = 0;
int plus_symbol_table_size = 0;
plus_minus_sym * plus_symbol_table = nullptr;
std::allocator<macro_stack_entry> macro_stack_entry_allocator;

int minus_symbol_table_index = 0;
int minus_symbol_table_size = 0;
plus_minus_sym* minus_symbol_table = nullptr;

static int compare_plus_minus_sym_value(const plus_minus_sym* a, const char* file, int line);
static int compare_plus_minus_sym(const plus_minus_sym* a, const plus_minus_sym* b);
static int plus_minus_sym_cmp_function(const void* a, const void* b);

int compare_plus_minus_sym(const plus_minus_sym* a, const plus_minus_sym* b)
{
    const int result = stricmp(a->file, b->file);
    if (result != 0) return result;
    return a->line - b->line;
}

int plus_minus_sym_cmp_function(const void* a, const void* b)
{
    return compare_plus_minus_sym((plus_minus_sym*)a, (plus_minus_sym*)b);
}

int compare_plus_minus_sym_value(const plus_minus_sym* a, const char* file, const int line)
{
    const plus_minus_sym b = { file, line, 0 };
    return compare_plus_minus_sym(a, &b);
}

symbol_table_ptr add_plus_minus_sym(char* name)
{
    const auto blank = "";

    if (last_label == nullptr)
        last_label = const_cast<char*>(&blank[0]);

    auto sym = symbol_table_allocator.allocate(1);
    memset(sym, 0, sizeof(symbol_table));

    char* temp_section = current_scope;

    sym->is_plus_minus = true;
    if (current_scope != nullptr)
        sym->scope = (char*)STRDUP(current_scope);

    sym->name = format_local_plus_minus_sym(name, last_label);

    if (sym->scope != nullptr) {
        sym->fullname = static_cast<char*>(MALLOC(strlen(sym->scope) + strlen(sym->name) + 2));
        sprintf(sym->fullname, "%s.%s", sym->scope, sym->name);
    }
    else
        sym->fullname = (char*)STRDUP(sym->name);

    current_scope = temp_section;
    
    auto key = ToKey(sym->fullname);
    symbol_dictionary[key] = sym;

    sanitize_symbol(sym);

    return sym;
}

bool is_local_sym(const char* name)
{
    for (auto nm = name; *nm; nm++) {
        if (*nm == '@')
            return true;
    }

    return false;
}

symbol_table_ptr add_symbol(char* name)
{
    if (name == nullptr) return nullptr;
    if (stricmp(name, "A") == 0) return nullptr;

    if (name[0] == '+' || name[0] == '-')
        return add_plus_minus_sym(name);

    auto sym = symbol_table_allocator.allocate(1);
    memset(sym, 0, sizeof(symbol_table));

    char* temp_section = current_scope;

    if (strstr(name, "."))
        current_scope = nullptr;

    if (name[0] == '.') {
        name++;
    }
    sym->is_macro_param = name[0] == '\\';
    sym->is_local = name[0] == '@';
    if (sym->is_local && last_label != nullptr) {
        name = format_local_sym(name, last_label);
    }

    size_t len = strlen(name);
    if (name[len - 1] == ':')
        name[len - 1] = 0;

    symbol_table_ptr tmp_ptr = look_up_symbol(name);
    if (tmp_ptr != nullptr) {
        symbol_table_allocator.destroy(sym);
        current_scope = temp_section;
        return tmp_ptr;
    }

    sym->name = (char*)STRDUP(name);
    if (sym->name == nullptr) {
        error(error_out_of_memory);
        exit(-1);
    }

    if (current_scope) {
        sym->scope = (char*)STRDUP(current_scope);
        if (sym->scope == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }
    }
    if (sym->scope) {
        len = strlen(sym->name) + strlen(sym->scope) + 2;
        sym->fullname = static_cast<char*>(MALLOC(len));
        if (sym->fullname == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }
        sprintf(sym->fullname, "%s.%s", sym->scope, sym->name);
    }
    else {
        sym->fullname = (char*)STRDUP(sym->name);
        if (sym->fullname == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }
    }

    if (sym->name[0] == '@') {
        sym->is_local = true;
        sym->is_initialized = true;
        sym->value = 0;
    }

    current_scope = temp_section;

    auto key = ToKey(sym->fullname);
    symbol_dictionary[key] = sym;

    sanitize_symbol(sym);

    return sym;
}

symbol_table_ptr look_up_macro_param(const int param_number)
{
    char macro_param_name[sizeof(int) + 5] = {0};

    sprintf(macro_param_name, "\\%d", param_number);
    auto found = symbol_dictionary.find(macro_param_name) != symbol_dictionary.end();
    if (!found)
        return nullptr;
    return symbol_dictionary[macro_param_name];
}

/// <summary>
/// Get SymbolTable for symbol.
/// </summary>
/// <param name="name">The name.</param>
/// <returns>int.</returns>
symbol_table_ptr look_up_symbol(const char* name)
{

    if (current_scope) {
        const int len = static_cast<int>(strlen(current_scope)) + static_cast<int>(strlen(name)) + 2;
        char* temp_name = static_cast<char*>(MALLOC(len));
        if (temp_name == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }
        sprintf(temp_name, "%s.%s", current_scope, name);
        // ReSharper disable once CppLocalVariableMayBeConst

        auto key = ToKey(temp_name);
        auto found = symbol_dictionary.find(key) != symbol_dictionary.end();
        FREE(temp_name);
        if (found) {
            auto sym = symbol_dictionary[key];
            sym->times_accessed++;
            return sym;
        }
    }

    // ReSharper disable once CppLocalVariableMayBeConst
    auto key = ToKey((char*)name);
    auto found = symbol_dictionary.find(key) != symbol_dictionary.end();
    if (!found)
        return nullptr;

    auto sym = symbol_dictionary[key];
    sym->times_accessed++;
    return sym;
}

symbol_table_ptr set_symbol_value(const symbol_table_ptr sym, const int value)
{
    if ((sym != nullptr) && (sym->value != value)) {
        if (((value & ~0xFFFF) != 0) && !sym->is_var) {
            error(error_value_out_of_range);
            return nullptr;
        }
        sym->value = value;
        if (!sym->is_macro_param && !sym->is_var) {
            sym_value_changed++;
            changed_sym_list.push_back(sym);
        }
    }
    return sym;
}

int find_plus_symbol_definition(const char* file, const int line)
{
    if (plus_symbol_table_index <= 0) return -1;

    for (int index = 0; index < plus_symbol_table_index; ++index) {
        if (compare_plus_minus_sym_value(&plus_symbol_table[index], file, line) == 0) {
            return index;
        }
    }
    return -1;
}

int find_minus_symbol_definition(const char* file, const int line)
{
    if (minus_symbol_table_index <= 0) return -1;

    for (int index = 0; index < minus_symbol_table_index; ++index) {
        if (compare_plus_minus_sym_value(&minus_symbol_table[index], file, line) == 0) {
            return index;
        }
    }
    return -1;
}

int find_minus_symbol(const int depth, const char* file, const int line)
{
    if (minus_symbol_table_index <= 0) return -1;
    int found_index = -1;
    for (int index = 0; index < minus_symbol_table_index; ++index) {
        if (minus_symbol_table[index].file == nullptr)
            continue;

        if (stricmp(file, minus_symbol_table[index].file) != 0)
            continue;

        if (minus_symbol_table[index].line <= line) {
            found_index = index;
        }
    }
    if (found_index >= 0) {
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
    for (int index = 0; index < plus_symbol_table_index; ++index) {
        if (stricmp(file, plus_symbol_table[index].file) != 0)
            continue;

        if (plus_symbol_table[index].line > line) {
            found_index = index;
            break;
        }
    }
    if (found_index >= 0) {
        found_index = found_index - 1 + depth;
        if (found_index >= 0 || found_index < plus_symbol_table_index)
            return found_index;
    }
    return -1;
}

void add_minus_symbol(const char* file, const int line, const int value)
{
    if (macro_params_stack.size() > 0) {
        error(error_plus_sym_not_allowed_in_macro);
        return;
    }

    if (minus_symbol_table_size - minus_symbol_table_index < GROW_THRESHOLD) {
        minus_symbol_table_size += PLUS_MINUS_TABLE_SIZE;
        auto* temp_ptr = static_cast<struct plus_minus_sym*>(
            REALLOC(minus_symbol_table, sizeof(struct plus_minus_sym) * minus_symbol_table_size));
        if (temp_ptr == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }
        char* memory_clear_location = (char*)temp_ptr;
        memory_clear_location += sizeof(plus_minus_sym) * (minus_symbol_table_size - PLUS_MINUS_TABLE_SIZE);
        memset(memory_clear_location, 0, sizeof(plus_minus_sym) * PLUS_MINUS_TABLE_SIZE);

        minus_symbol_table = temp_ptr;
    }

    minus_symbol_table[minus_symbol_table_index].file = (char*)STRDUP(file);
    minus_symbol_table[minus_symbol_table_index].line = line;
    minus_symbol_table[minus_symbol_table_index].value = value;
    ++minus_symbol_table_index;

    qsort(minus_symbol_table, minus_symbol_table_index, sizeof(plus_minus_sym), plus_minus_sym_cmp_function);
}

void add_plus_symbol(const char* file, const int line, const int value)
{
    if (macro_params_stack.size() > 0) {
        error(error_plus_sym_not_allowed_in_macro);
        return;
    }

    if (plus_symbol_table_size - plus_symbol_table_index < GROW_THRESHOLD) {
        plus_symbol_table_size += PLUS_MINUS_TABLE_SIZE;
        auto* temp_ptr = static_cast<struct plus_minus_sym*>(REALLOC(plus_symbol_table, sizeof(struct plus_minus_sym) * plus_symbol_table_size));
        if (temp_ptr == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }
        plus_symbol_table = temp_ptr;
    }

    plus_symbol_table[plus_symbol_table_index].file = (char*)STRDUP(file);
    plus_symbol_table[plus_symbol_table_index].line = line;
    plus_symbol_table[plus_symbol_table_index].value = value;
    ++plus_symbol_table_index;

    qsort(plus_symbol_table, plus_symbol_table_index, sizeof(plus_minus_sym), plus_minus_sym_cmp_function);
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
    const int max_macro = get_max_macro_parameter();

    auto macro_stack_param_entry = macro_stack_entry_allocator.allocate(1);
    macro_stack_param_entry->num_nodes = 0;
    macro_stack_param_entry->values = nullptr;

    if (max_macro < 0) {
        macro_stack_param_entry->num_nodes = 0;
    }
    else {
        macro_stack_param_entry->num_nodes = max_macro + 1;
        macro_stack_param_entry->values = static_cast<int*>(MALLOC(((unsigned long long)max_macro + 1) * sizeof(long)));

        macro_stack_param_entry->values[0] = 0;
        for (int index = 1; index <= max_macro; index++) { 
            // ReSharper disable once CppLocalVariableMayBeConst
            symbol_table_ptr tmp_ptr = look_up_macro_param(index);
            if (tmp_ptr != nullptr) {
                macro_stack_param_entry->values[index] = tmp_ptr->value;
            }
            else {
                error(error_macro_parameter_not_found);
                return;
            }
        }
    }

    macro_params_stack.push(macro_stack_param_entry);
}

void pop_macro_params(void)
{
    if (macro_params_stack.size() < 1) {
        error(error_macro_parameter_under_flow);
        exit(-1);
    }

    auto* macro_stack_param_entry_ptr = macro_params_stack.top();
    macro_params_stack.pop();

    for (int index = 1; index < macro_stack_param_entry_ptr->num_nodes; index++) {
        const symbol_table_ptr tmp_ptr = look_up_macro_param(index);
        if (tmp_ptr != nullptr) {
            set_symbol_value(tmp_ptr, macro_stack_param_entry_ptr->values[index]);
        }
        else {
            error(error_macro_parameter_not_found);
            return;
        }
    }
    if (macro_stack_param_entry_ptr->num_nodes > 0) {
        FREE(macro_stack_param_entry_ptr->values);
        macro_stack_param_entry_ptr->values = nullptr;
    }
    macro_stack_param_entry_ptr->num_nodes = 0;
    macro_stack_entry_allocator.destroy(macro_stack_param_entry_ptr);
}

void sanitize_symbol(const symbol_table_ptr symbol)
{
    char* combined_section;

    if (symbol->is_macro_param) {
        if (symbol->scope) {
            FREE(symbol->scope);
            symbol->scope = nullptr;
            return;
        }
    }

    if (strstr(symbol->name, ".") == nullptr)
        return;

    int len = static_cast<int>(strlen(symbol->name));
    while (symbol->name[len] != '.')
        len--;
    symbol->name[len] = 0;
    char* temp_section = (char*)STRDUP(symbol->name);
    char* temp_name = (char*)STRDUP(&symbol->name[len + 1]);
    if (temp_name == nullptr) {
        error(error_out_of_memory);
        exit(-1);
    }
    if (symbol->name)
        FREE(symbol->name);

    symbol->name = temp_name;
    if (symbol->scope) {
        len = static_cast<int>(strlen(symbol->scope)) + static_cast<int>(strlen(temp_section)) + 2;
        combined_section = static_cast<char*>(MALLOC(len));
        if (combined_section == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }
        sprintf(combined_section, "%s.%s", symbol->scope, temp_section);
        FREE(symbol->scope);
        symbol->scope = combined_section;
    }
    else {
        len = static_cast<int>(strlen(temp_section)) + 1;
        combined_section = static_cast<char*>(MALLOC(len));
        if (combined_section == nullptr) {
            error(error_out_of_memory);
            exit(-1);
        }
        strcpy(combined_section, temp_section);
        symbol->scope = combined_section;
    }
    FREE(temp_section);
}

char* format_local_plus_minus_sym(char* name, char* label)
{
    const auto blank = "";
    if (label == nullptr)
        label = const_cast<char*>(&blank[0]);

    char number_buffer[20] = { 0 };

    sprintf(number_buffer, "%d", yylineno);

    const int new_len = strlen(last_label) + 1 + strlen(name) + 1 + strlen(number_buffer) + 1 + 2 + 2;  // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)

    char* new_name = static_cast<char*>(MALLOC(new_len));
    if (new_name) {
        sprintf(new_name, "%s__%s__%s", label, number_buffer, name);
    }
    name = (char*)STRDUP(new_name);
    FREE(new_name);
    return name;
}

char* format_local_sym(char* name, const char* label)
{
    const int new_len = strlen(last_label) + 1 + strlen(name) + 1 + 2;  // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
    char* new_name = static_cast<char*>(MALLOC(new_len));
    if (new_name) {
        sprintf(new_name, "%s__%s", label, name);
    }
    name = (char*)STRDUP(new_name);
    FREE(new_name);
    return name;
}

int print_symbol(symbol_table_ptr sym, FILE* file)
{
    fprintf(file, "\n");
    fprintf(file, "    name:           %s\n", sym->name);
    fprintf(file, "    name:           %s\n", sym->scope);
    fprintf(file, "    fullname:       %s\n", sym->fullname);
    fprintf(file, "    value:          %d\n", sym->value);
    fprintf(file, "    macro_node:     %p\n", sym->macro_node);
    fprintf(file, "    is_label:       %s\n", sym->is_label ? "true" : "false");
    fprintf(file, "    is_macro_name:  %s\n", sym->is_macro_name ? "true" : "false");
    fprintf(file, "    is_initialized: %s\n", sym->is_initialized ? "true" : "false");
    fprintf(file, "    is_local:       %s\n", sym->is_local ? "true" : "false");
    fprintf(file, "    is_var:         %s\n", sym->is_var ? "true" : "false");
    fprintf(file, "    is_plus_minus:  %s\n", sym->is_plus_minus ? "true" : "false");
    fprintf(file, "\n");
    return 0;
}

typedef struct sym_entry
{
    char* name;
    int value;
} sym_entry;

std::allocator<sym_entry> sym_entry_allocator;

int symbol_compare(const void*a, const void* b)
{
    const sym_entry* aa = (sym_entry*)a;
    const sym_entry* bb = (sym_entry*)b;

    return aa->value - bb->value; 
}

typedef bool (*filter)(symbol_table* sym);
static void dump_symbols_common(filter f, FILE* file)
{
    const int sz = symbol_dictionary.size();
    if (sz == 0) return;

    int count = 0;
    auto* entry_array = sym_entry_allocator.allocate(sz);
    if (entry_array == nullptr) {
        error(error_out_of_memory);
        exit(-1);
    }

    for (const auto& [key, value] : symbol_dictionary) {
        if (f(value)) {
            entry_array[count].name = value->fullname;
            entry_array[count].value = value->value;
            count++;
        }
    }

    qsort(entry_array, count, sizeof(sym_entry), symbol_compare);

    auto current_entry = entry_array;
    int column = 1;
    for (int i = 0; i < count; ++i) {
        const int columns = 5;
        const char* name = current_entry->name;
        const int value = current_entry->value;
        fprintf(file, "%15s $%4.4X  ", name, value);
        if (column == columns) {
            fprintf(file, "\n");
            column = 1;
        }
        else {
            column++;
        }
        current_entry++;
    }
    fprintf(file, "\n");

    sym_entry_allocator.destroy(entry_array);
}

static bool all_sym_filter(symbol_table* sym) 
{
    return !sym->is_plus_minus && sym->times_accessed > pass + 1 && !sym->is_var && !sym->is_macro_param && !sym->is_macro_name;
}

static bool unresolved_filter(symbol_table* sym)
{
    return all_sym_filter(sym) && (sym->is_initialized == false);
}

void dump_symbols(FILE* file)
{
    dump_symbols_common(all_sym_filter, file);
}

void dump_unresolved_symbols(FILE* file)
{
    dump_symbols_common(unresolved_filter, file);
}

int unresolved_symbol_count(void)
{
    int count = 0;

    for (const auto& [key, value] : symbol_dictionary) {
        if (unresolved_filter(value)) {
            count++;
        }
    }
    return count;
}

void dump_changed_symbols(FILE* file)
{
    int column = 1;
    for (auto& changd_sym : changed_sym_list) {

        const int columns = 5;
        const char* name = changd_sym->name;
        const int value = changd_sym->value;
        fprintf(file, "%15s $%4.4X  ", name, value);
        if (column == columns) {
            fprintf(file, "\n");
            column = 1;
        }
        else {
            column++;
        }
    }
    fprintf(file, "\n");
    fprintf(file, "\n");
}

std::string ToKey(char* key)
{
    std::string out;
    while (*key) {
        out += toupper(*key++);
    }
    return out;
}
