#pragma once

#include "node.h"

extern int generate_output(FILE* file, parse_node_ptr p);
extern int get_op_code_byte_count(parse_node_ptr p);
extern void generate_output_reset(void);

extern int total_bytes_written;

