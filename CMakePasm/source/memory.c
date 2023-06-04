// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppZeroConstantCanBeReplacedWithNullptr
// ReSharper disable CppClangTidyConcurrencyMtUnsafe
// ReSharper disable CppClangTidyClangDiagnosticImplicitInt
// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticShorten64To32
// ReSharper disable CppClangTidyClangDiagnosticDeprecatedDeclarations
// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppPointerConversionDropsQualifiers
// ReSharper disable CppClangTidyClangDiagnosticIncompatiblePointerTypesDiscardsQualifiers

// ReSharper disable CppClangTidyCertErr33C
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "dictionary.h"
#include "error.h"
#include "memory.h"

#include "node.h"
#include "pasm.h"
#include "stacks.h"

#pragma warning(disable: 4090 4996)
extern bool track_malloc;

typedef struct
{
    size_t size;
    char* function;
    int line;
    void* memory;
    int times_deleted;
} memory_entry;

size_t total_allocated_bytes = 0;

bool track_malloc = true;

// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
static dictionary_ptr alloc_dict = NULL;

void* pasm_malloc(const size_t size, const char* function, const int line)
{
    if (!track_malloc) return malloc(size);

    const bool temp = track_malloc;
    track_malloc = false;

    if (alloc_dict == NULL)
    {
        alloc_dict = dict_create(sizeof(memory_entry), -1);
    }

    memory_entry entry = { size, strdup(function), line, malloc(size), 0 };

    dict_insert(&alloc_dict, entry.memory, &entry);
    total_allocated_bytes += entry.size;

    track_malloc = temp;
    return entry.memory;
}

void* pasm_realloc(void* memory, const size_t size, const char* function, const int line)
{
    if (!track_malloc) return realloc(memory, size);

    const bool temp = track_malloc;
    track_malloc = false;

    if (alloc_dict == NULL)
    {
        alloc_dict = dict_create(sizeof(memory_entry), -1);
    }

    memory_entry* old_entry = dict_search(alloc_dict, memory);
    memory_entry entry;
    if (old_entry)
    {
        total_allocated_bytes -= old_entry->size;
        void* temp_ptr = realloc(old_entry->memory, size);
        if (temp_ptr != NULL)
        {
            old_entry->memory = temp_ptr;
            old_entry->size = size;
        }
        entry = *old_entry;
    }
    else
    {
        entry.memory = malloc(size);
        entry.size = size;
        entry.function = strdup(function);
        entry.line = line;
        entry.times_deleted = 0;

        dict_insert(&alloc_dict, entry.memory, &entry);
    }
    total_allocated_bytes += entry.size;
    track_malloc = temp;
    return entry.memory;
}


void pasm_free(void* memory)
{
    if (!track_malloc)
    {
        free(memory);
        return;
    }

    const bool temp = track_malloc;
    track_malloc = false;

    if (alloc_dict == NULL)
    {
        error(error_free_without_malloc);
        exit(-1);
    }
    memory_entry* entry = dict_search(alloc_dict, memory);
    if (entry)
    {
        if (entry->times_deleted == 0)
        {
            total_allocated_bytes -= entry->size;
            entry->times_deleted++;
            dict_delete(alloc_dict, entry);
        }
        else
        { 
            error(error_free_unknown_pointer);
        }
    }
    else
    {
        error(error_free_unknown_pointer);
        // exit(-1);
    }
    track_malloc = temp;
}

const char* pasm_strdup(const char* str, const char* function, const int line)
{
    const size_t len = strlen(str);
    char* dup = pasm_malloc(len + 1, function, line);
    strcpy(dup, str);
    return dup;
}

int print_memory(const element_ptr e, FILE* file)
{
    const memory_entry* me = e->value;
    if (stricmp(me->function, "allocate_node") == 0 && me->size == sizeof(parse_node))
    {
        const parse_node_ptr p = me->memory;
        if (!p->allocated)
            return 0;
    }

    fprintf(file, "%s Line %d\n", me->function, me->line);
    fprintf(file, "size %zu\n", me->size);
    fprintf(file, "%p\n", me->memory);

    return 0;
}

#ifndef _WIN32
int stricmp(char const *a, char const *b) 
{
  while (*b) {
    int d = tolower(*a) - tolower(*b);
    if (d) 
    {
        return d;
    } 
    a++;
    b++;
  } 
  return tolower(*a);
}
#endif

void dump_memory(FILE* file)
{
    dump_dictionary(alloc_dict, print_memory, console);
}
