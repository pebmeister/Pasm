#pragma once


#ifdef __cplusplus
extern "C"
{
#endif
#include <stddef.h>
#include <stdio.h>

extern void pasm_free(void* memory);
extern void* pasm_malloc(size_t size, const char* function, int line);
extern void* pasm_realloc(void* memory, size_t size, const char* function, int line);
extern const char* pasm_strdup(const char* str, const char* function, int line);
extern void dump_memory(FILE* file);
#ifndef _WIN32
extern int stricmp(char const *a, char const *b);
#endif

#ifdef _DEBUG
#define MALLOC(s)       pasm_malloc(s, __FUNCTION__, __LINE__)
#define REALLOC(s,sz)   pasm_realloc(s, sz, __FUNCTION__, __LINE__)
#define FREE(p)         pasm_free(p)
#define STRDUP(s)       pasm_strdup(s, __FUNCTION__, __LINE__)
#else
#define MALLOC(s)       malloc((s))
#define REALLOC(s,sz)   realloc((s),(sz))
#define FREE(p)         free((p))
#define STRDUP(s)       strdup((s))
#endif

extern size_t total_allocated_bytes;

#ifdef __cplusplus
}
#endif
