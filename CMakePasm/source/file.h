#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct file_line_node
{
    char* line_content;
    int line_number;
    int displayed;
    struct file_line_node* next;
} file_line_node;

extern file_line_node* read_file_lines(const char* file_name);
extern void free_file_lines(file_line_node*);
extern FILE* open_file(const char* file, const char* mode);
extern int open_include_file(char* file);

#ifdef __cplusplus
}
#endif
