// ReSharper disable CppClangTidyConcurrencyMtUnsafe
// ReSharper disable CppPointerConversionDropsQualifiers
// ReSharper disable CppClangTidyCertErr33C
// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyClangDiagnosticShadow
// ReSharper disable CppClangTidyBugproneSuspiciousStringCompare
// ReSharper disable CppClangTidyClangDiagnosticFormatNonliteral
#include "genlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "file.h"
#include "memory.h"
#include "pasm.h"

#pragma warning(disable: 4090 4996)

list_table_ptr list_head = NULL;
file_entry* source_file_list = NULL;

file_line_node* get_file_line(char* file, const int line)
{
    file_entry* current_file_entry = source_file_list;
    file_entry* prev = NULL;

    for (; current_file_entry != NULL && stricmp(current_file_entry->filename, file) != 0; current_file_entry = current_file_entry->next)
        prev = current_file_entry;

    if (!current_file_entry)
    {
        current_file_entry = (file_entry*)MALLOC(sizeof(file_entry));
        if (current_file_entry)
        {
            memset(current_file_entry, 0, sizeof(file_entry));
            current_file_entry->filename = file;
            current_file_entry->lines = read_file_lines(file);
            if (prev)
                prev->next = current_file_entry;
        }
    }
    if (current_file_entry)
    {
        if (source_file_list == NULL)
            source_file_list = current_file_entry;

        if (stricmp(current_file_entry->filename, file) == 0)
        {
            file_line_node* line_entry = current_file_entry->lines;
            for (; line_entry && line_entry->line_number < line; line_entry = line_entry->next)
                ;
            return line_entry;
        }
    }
    error(error_opening_list_file);
    return NULL;
}

const char* source_indent_format = "%-45s%s";

void delete_list_table(void)
{
    list_table_ptr list_node = list_head;
    while (list_node)
    {
        const list_table_ptr next = list_node->next;
        FREE(list_node->output);
        FREE(list_node->filename);
        memset(list_node, 0, sizeof(list_table));
        FREE(list_node);
        list_node = next;
    }
    list_head = NULL;
}

void generate_list_file(FILE* list_file)
{
    list_table_ptr list_node = list_head;
    current_file_name = NULL;

    int state = 1;  // NOLINT(clang-diagnostic-implicit-int)
    fprintf(list_file, "\n");

    current_file_name = list_head->filename;
    fprintf(list_file, "Processing %s\n", current_file_name);
    for (int line = 1; line < list_head->line; ++line)
    {
        internal_buffer[0] = 0;
        file_line_node* source_list = get_file_line(list_node->filename, line);
        if (source_list->displayed == 0)
        {
            if (state)
            {
                sprintf(internal_buffer, source_indent_format, "", source_list->line_content);
                fputs(internal_buffer, list_file);
            }
            source_list->displayed++;
        }
    }

    // loop through all list  nodes
    for (; list_node; list_node = list_node->next)
    {
        if (current_file_name == NULL || stricmp(current_file_name, list_node->filename))
        {
            current_file_name = list_node->filename;
            fprintf(list_file, "Processing %s\n", current_file_name);
        }

        if (list_node->print_state != state)
        {
            state = list_node->print_state;
            continue;
        }

        if (list_node->print_state == 0)
            continue;

        const int start_line = list_node->line;
        int end_line = start_line;
        if (list_node->next && stricmp(list_node->filename, list_node->next->filename) == 0 && list_node->next->line > list_node->line)
            end_line = list_node->next->line - 1;

        file_line_node* source_list = get_file_line(list_node->filename, start_line);
        if (source_list == NULL)
            continue;

        internal_buffer[0] = 0;
        if (source_list->displayed == 0)
        {
            if (state)
            {
                sprintf(internal_buffer, source_indent_format, list_node->output, source_list->line_content);
            }
            source_list->displayed++;
        }
        else
        {
            if (state)
            {
                sprintf(internal_buffer, source_indent_format, list_node->output, "\n");
            }
        }
        fputs(internal_buffer, list_file);

        for (source_list = source_list->next; source_list && source_list->line_number <= end_line; source_list = source_list->next)
        {
            internal_buffer[0] = 0;
            sprintf(internal_buffer, source_indent_format, "","");
            fputs(internal_buffer, list_file);
            if (source_list->displayed == 0)
            {
                if (state)
                {
                    fprintf(list_file, "%s", source_list->line_content);
                }
                source_list->displayed++;
            }
            else
            {
                if (state)
                {
                    fputs("\n", list_file);
                }
            }
        }
    }
}

/**
 * \brief Reset displayed field in source_file_list tree
 */
void reset_file_lines(void)
{
    for (const file_entry * file_entry = source_file_list; file_entry != NULL; file_entry = file_entry->next)
    {
        for (file_line_node* file_line_ptr = file_entry->lines; file_line_ptr != NULL; file_line_ptr = file_line_ptr->next)
        {
            file_line_ptr->displayed = 0;
        }
    }
}

void delete_file_lines(void)
{
    source_file_list = NULL;
}

/**
 * \brief add an entry to the table list
 * \param file name of file to add
 * \param line line number to add
 * \param output table list entry of added file line
 * \return new entry
 */
list_table_ptr add_list(const char* file, int line, const char* output)
{
    // yylineno seems to be 1 off.
    --line;

    // ReSharper disable once CppLocalVariableMayBeConst
    list_table_ptr var_ptr = MALLOC(sizeof(list_table));
    if (var_ptr == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    memset(var_ptr, 0, sizeof(list_table));
    var_ptr->print_state = print_list_state;

    var_ptr->filename = STRDUP(file);
    if (var_ptr->filename == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    var_ptr->line = line + 1;
    var_ptr->output = STRDUP(output);
    if (var_ptr->output == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    list_table_ptr tmp_ptr = list_head;

    if (list_head == NULL)
    {
        list_head = var_ptr;
        return var_ptr;
    }

    while (tmp_ptr->next != NULL)
        tmp_ptr = tmp_ptr->next;

    tmp_ptr->next = var_ptr;

    return var_ptr;
}
