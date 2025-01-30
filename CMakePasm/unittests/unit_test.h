#include <iostream>
#include <fstream>

#include "error.h"
#include "expand.h"
#include "file.h"
#include "flex.h"
#include "genoutput.h"
#include "genlist.h"
#include "hash.h"
#include "memory.h"
#include "node.h"
#include "opcodes.h"
#include "pasm.h"
#include "str.h"
#include "sym.h"

#include "pasm.tab.h"

extern void print_node(parse_node_ptr p, std::ofstream& file);

typedef struct ops
{
    instruction in;
    int number_of_modes;
    int* modes;
    int* opcodes;
} ops, * ops_ptr;

extern void init_globals();
extern void initialize();
extern void destroy();
extern char* escape_string(const char* str);
extern size_t char_to_w_string(std::string s, std::wstring& ws);
extern void execute_text(std::string& text, const unsigned char* expected, const size_t count, std::string expected_text);

extern ops_ptr cpu_6502_illegal_ops;
extern ops_ptr cpu_65c02_ops;
extern ops_ptr cpu_6502_ops;


#define LB(x)       ((x)&0xFF)
#define HB(x)       (LB((x)>>8))
#define WORD(x)     LB(x),HB(x)

#ifndef _countof
#define _countof(a) (sizeof((a))/sizeof(*(a)))
#endif

#define EXPECT_NULL(x)          EXPECT_EQ((x), nullptr)
#define EXPECT_NOT_NULL(x)      EXPECT_NE((x), nullptr)

