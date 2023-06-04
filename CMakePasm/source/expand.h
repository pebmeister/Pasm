#pragma once

#include "node.h"

#ifdef __cplusplus
extern "C"
{
#endif
extern void init_expander(void);
extern void destroy_expander(void);
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
extern dictionary_ptr macro_dict;

#ifdef __cplusplus
}
#endif
