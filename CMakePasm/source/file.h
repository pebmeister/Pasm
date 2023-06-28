#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>

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
extern void open_file_stream(std::ofstream& file, const char* filename, const std::ios::openmode mode);
extern int open_include_file(char* file);

extern std::ofstream log_file;
extern FILE* sym_file;
extern FILE* output_file;
extern FILE* list_file;


