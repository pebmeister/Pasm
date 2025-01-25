#pragma once

#include "node.h"

extern int expand_node(parse_node_ptr p);
extern void reset_macro_dict(void);
extern char* last_label;
extern char* current_scope;
extern void dump_op_table(void);
extern void dump_table_dictionary(void);
extern int is_uninitialized_symbol(parse_node_ptr p);
extern bool has_uninitialized_symbol(parse_node_ptr p);
extern int get_op_byte_count(parse_node_ptr p);

extern int sym_value_changed;
extern int print_list_state;
extern int end_expansion;

typedef struct macro_dict_entry {
public:
    int times_executed;
} macro_dict_entry;

extern std::allocator<macro_dict_entry> macro_entry_allocator;
extern std::map<std::string, macro_dict_entry*> macro_dict;
