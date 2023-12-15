#pragma once
#include <stdio.h>

#include "file.h"

typedef struct list_table
{
    int line;
    char* filename;
    char* output;
    int print_state;
    bool list_directive;
    struct list_table* next;
} list_table, * list_table_ptr;

typedef struct file_entry
{
    char* filename;
    file_line_node* lines;
    struct file_entry* next;
} file_entry;

extern void delete_file_lines(void);
extern  void delete_list_table(void);
extern void generate_list_file(FILE* list_file);
extern void reset_file_lines(void);
extern list_table_ptr add_list(const char* file, int line, const char* output);

extern int print_list_state;
extern list_table_ptr list_head;

#define SRC_LST_INDENT  25
