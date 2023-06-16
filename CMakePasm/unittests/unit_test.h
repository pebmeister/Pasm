#include "dictionary.h"
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
#include "stacks.h"
#include "str.h"
#include "sym.h"

#include "pasm.tab.h"

typedef struct ops
{
    instruction in;
    int number_of_modes;
    int* modes;
    int* opcodes;
} ops, * ops_ptr;

extern void initialize();
extern void destroy();
extern char* escape_string(const char* str);
extern size_t char_to_w_string(std::string s, std::wstring& ws);
extern void execute_text(const char* text, const unsigned char* expected, size_t count, const char* expected_text);

extern ops_ptr cpu_6502_illegal_ops;
extern ops_ptr cpu_65c02_ops;
extern ops_ptr cpu_6502_ops;


#define LB(x)       ((x)&0xFF)
#define HB(x)       (LB((x)>>8))
#define WORD(x)     LB(x),HB(x)

#define _countof(a) (sizeof((a))/sizeof(*(a)))

#define EXPECT_NULL(x)          EXPECT_EQ((x), nullptr)
#define EXPECT_NOT_NULL(x)      EXPECT_NE((x), nullptr)

