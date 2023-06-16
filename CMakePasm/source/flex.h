#pragma once
#include <stdio.h>

// flex bison functions and variables
#ifndef YY_TYPEDEF_YY_SIZE_T
#define YY_TYPEDEF_YY_SIZE_T
    typedef size_t yy_size_t;
#endif

/* copied from generated flex line_content */
/* Size of default input buffer. */
#ifndef YY_BUF_SIZE
#ifdef __ia64__
/* On IA-64, the buffer size is 16k, not 8k.
 * Moreover, YY_BUF_SIZE is 2*YY_READ_BUF_SIZE in the general case.
 * Ditto for the __ia64__ case accordingly.
 */
#define YY_BUF_SIZE 32768
#else
#define YY_BUF_SIZE 16384
#endif /* __ia64__ */
#endif

#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
    typedef struct yy_buffer_state* YY_BUFFER_STATE;
#endif

extern int yylex();
extern void yyerror(const char* s);
extern void yymessage(const char* s);
extern void yytrace(const char* s);
extern void* yyalloc(yy_size_t);
extern void* yyrealloc(void*, size_t);
extern void yyfree(void*);
extern void yyrestart(FILE* input_file);
extern int yylineno;
extern FILE* yyin;
extern void yypush_buffer_state(YY_BUFFER_STATE new_buffer);
extern YY_BUFFER_STATE yy_create_buffer(FILE* file, int size);

extern FILE* yyout;

