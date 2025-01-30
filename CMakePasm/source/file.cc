// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppZeroConstantCanBeReplacedWithNullptr
// ReSharper disable CppPointerConversionDropsQualifiers
// ReSharper disable CppClangTidyClangDiagnosticIncompatiblePointerTypesDiscardsQualifiers
#include "file.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#include "error.h"
#include "flex.h"
#include "memory.h"
#include "node.h"
#include "pasm.h"

#pragma warning(disable:4996 4090)

enum {
    path_max = 4096
};


static char path[path_max] = { 0 };

void open_file_stream(std::ofstream& file, const char* filename, const std::ios::openmode mode)
{
    file.open(filename, mode);
    if (file.is_open())
        return;

    if (directories == nullptr) return;

    for (char* directory = directories; *directory != 0;) {
        char* p = path;
        int len = 0;
        while (*directory != 0 && *directory != ';' && len < (path_max - 1)) {
            ++len;
            *p++ = *directory++;
        }
        *p = 0;
        if (len + strlen(filename) + 2 >= path_max) {
            error2(error_path_name_too_long, path);
            exit(-1);  // NOLINT(concurrency-mt-unsafe)
        }
        strcat(path, filename);
        file.open(path, mode);
        if (file.is_open())
            return;

        if (*directory == ';')
            directory++;

        while (*directory == ' ' || *directory == '\t')
            directory++;
    }
}

/**
 * \brief open a file. If directories is not NULL it will search them in order
 * \param file name of file to open
 * \param mode mode to open file with
 * \return open file or -1
 */
FILE* open_file(const char* file, const char* mode)
{
    if (file == NULL) {
        return NULL;
    }

    FILE* fd = fopen(file, mode);
    if (fd != NULL) return fd;

    if (directories == NULL) return NULL;

    for (char* dir = directories; *dir != 0;) {
        char* p = path;
        int len = 0;
        while (*dir != 0 && *dir != ';' && len < (path_max - 1)) {
            ++len;
            *p++ = *dir++;
        }
        *p = 0;
        if (len + strlen(file) + 2 >= path_max) {
            error2(error_path_name_too_long, path);
            exit(-1);  // NOLINT(concurrency-mt-unsafe)
        }
        strcat(path, file);
        fd = fopen(path, mode);
        if (fd != NULL)
            return fd;

        if (*dir == ';')
            dir++;

        while (*dir == ' ' || *dir == '\t')
            dir++;
    }
    return NULL;
}

/**
 * \brief read a file into a file line linked list
 * \param file_name file to read
 * \return head node of the file line linked list
 */
file_line_node* read_file_lines(const char* file_name)
{
    assert(file_name != NULL);

    int line = 1;
    file_line_node* head_file_node = NULL;
    file_line_node* file_node = NULL;
    FILE* file = open_file(file_name, "r");
    if (file == NULL) {
        error(error_opening_list_file);
        return NULL;
    }
    while (!feof(file)) {
        internal_buffer[0] = 0;
        if (fgets(internal_buffer, max_line_len, file)) {
            if (file_node == NULL) {
                // ReSharper disable once CppCStyleCast
                file_node = (file_line_node*)MALLOC(sizeof(file_line_node));
                head_file_node = file_node;
            }
            else {
                // ReSharper disable once CppCStyleCast
                file_node->next = (file_line_node*)MALLOC(sizeof(file_line_node));
                file_node = file_node->next;
            }
            if (file_node == NULL) {
                error(error_out_of_memory);
                return NULL;
            }
            memset(file_node, 0, sizeof(file_line_node));
            // ReSharper disable once CppDeprecatedEntity
            file_node->line_content = (char*)STRDUP(internal_buffer);
            file_node->line_number = line++;
        }
    }
    fclose(file);  // NOLINT(cert-err33-c)
    return head_file_node;
}

/**
 * \brief FREE memory allocated from read_file_lines
 * \param file_lines file_lines head node
 */
void free_file_lines(file_line_node* file_lines)
{
    file_line_node* temp = file_lines;
    while (temp != NULL) {
        file_line_node* next = temp->next;
        FREE(temp->line_content);
        FREE(temp);
        temp = next;
    }
}

/**
 * \brief open an include file
 * \param file name of file
 * \return 0 on success
 */
int open_include_file(char* file)
{
    if (final_pass) generate_list_node(NULL);

    file_line_stack_entry file_line = { current_file_name, yylineno };
    file_stack.push(file_line);

    current_file_name = file;

    yyin = open_file(file, "r");
    yylineno = 0;

    if (final_pass) generate_list_node(NULL);

    if (!yyin) {
        if (file != NULL) {
            error2(error_cant_open_include_file, file);
        }
        else {
            error(error_cant_open_include_file);
        }
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }
    yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
    return 0;
}
