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
#include <map>

#include "error.h"
#include "memory.h"

#include "node.h"
#include "pasm.h"

#pragma warning(disable: 4090 4996)
extern bool track_malloc;

typedef struct {
    size_t size;
    char* function;
    int line;
    void* memory;
    int times_deleted;
} memory_entry;

size_t total_allocated_bytes = 0;

bool track_malloc = true;

// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
std::map<void*, memory_entry*> alloc_dict;
std::allocator<memory_entry> memory_entry_allocator;

void* pasm_malloc(const size_t size, const char* function, const int line)
{
    if (!track_malloc) return malloc(size);

    const bool temp = track_malloc;
    track_malloc = false;

    auto entry = memory_entry_allocator.allocate(1);
    entry->function = (char*)STRDUP(function);
    entry->line = line;
    entry->size = size;
    entry->memory = malloc(size);
    entry->times_deleted = 0;
    alloc_dict[entry->memory] = entry;

    total_allocated_bytes += entry->size;

    track_malloc = temp;
    return entry->memory;
}

void* pasm_realloc(void* memory, const size_t size, const char* function, const int line)
{
    if (!track_malloc) return realloc(memory, size);

    const bool temp = track_malloc;
    track_malloc = false;
    void* old_memory = nullptr;
    auto alloc_search = alloc_dict.find(memory);
    if (alloc_search != alloc_dict.end()) {
        auto old_entry = alloc_dict[memory];
        old_memory = old_entry->memory;
        total_allocated_bytes -= old_entry->size;
        alloc_dict.erase(alloc_search);
    }
    auto entry = memory_entry_allocator.allocate(1);
    entry->function = (char*)STRDUP(function);
    entry->line = line;
    entry->size = size;
    entry->memory = realloc(old_memory, size);
    entry->times_deleted = 0;
    alloc_dict[entry->memory] = entry;

    total_allocated_bytes += entry->size;

    track_malloc = temp;
    return entry->memory;
}

void pasm_free(void* memory)
{
    if (!track_malloc) {
        free(memory);
        return;
    }

    const bool temp = track_malloc;
    track_malloc = false;

    auto search_alloc = alloc_dict.find(memory);
    if (search_alloc != alloc_dict.end()) {
        auto entry = alloc_dict[memory];
        if (entry->times_deleted == 0) {
            total_allocated_bytes -= entry->size;
            entry->times_deleted++;
            memory_entry_allocator.destroy(entry);
            alloc_dict.erase(entry);
        }
        else {
            error(error_free_unknown_pointer);
        }
    }
    else {
        error(error_free_unknown_pointer);
        // exit(-1);
    }
    track_malloc = temp;
}

const char* pasm_strdup(const char* str, const char* function, const int line)
{
    const size_t len = strlen(str);
    char* dup = static_cast<char*>(pasm_malloc(len + 1, function, line));
    strcpy(dup, str);
    return dup;
}

int print_memory(memory_entry* me, FILE* file)
{
    if (stricmp(me->function, "allocate_node") == 0 && me->size == sizeof(parse_node)) {
        const parse_node_ptr p = static_cast<parse_node_ptr>(me->memory);
        if (!p->allocated)
            return 0;
    }

    fprintf(file, "%s Line %d\n", me->function, me->line);
    fprintf(file, "size %zu\n", me->size);
    fprintf(file, "%p\n", me->memory);

    return 0;
}

#ifndef _WIN32
int stricmp(char const* a, char const* b)
{
    while (*b) {
        int d = tolower(*a) - tolower(*b);
        if (d) {
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
    for (const auto& [key, value] : alloc_dict) {
        print_memory(value, console);
    }
}
