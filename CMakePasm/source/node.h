#pragma once
#include "sym.h"

// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

typedef enum {
    type_unknown = 0, type_head_node = 1, type_con, type_label, type_id, type_macro_id, type_macro_ex,
    type_opr, type_op_code, type_data, type_str, type_print, type_last
} node_type_enum;

typedef enum
{
    data_string = 0,
    data_byte = 1,
    data_word = 2,
} data_type_expand;

/* constants */
typedef struct
{
    int value;                      /* value of constant */
    int is_program_counter;         /* TRUE if value is PC */
} constant_node_struct;

typedef struct
{
    char* value;                    /* value of string */
    char* allocated;			    /* allocated string */
    int len;
} string_node_struct;

/* identifiers */
typedef struct
{
    char* name;
    symbol_table_ptr symbol_ptr;          /* symbol entry */
} id_node_struct;

/* operators */
typedef struct
{
    int opr;                    /* operator */
} operator_node_struct;

/* opcodes */
typedef struct
{
    int instruction;            /* untranslated instruction */
    int opcode;                 /* opcode */
    int mode;                   /* mode */
    int program_counter;        /* program counter */
} opcode_node_struct;

/* macro execution node */
typedef struct
{
    void* macro;
    void* macro_params;
    char* name;
} macro_node_struct;

/* data definition node */
typedef struct
{
    int size;                       /* 1 = byte 2 = word, 0 = string */
    void* data;
} data_node_struct;

typedef struct
{
    int print_state;
} print_state_node_struct;

/* nodes union */
typedef struct parse_node
{
    node_type_enum type;            /* type of node */
    bool allocated;
    union
    {
        constant_node_struct con;   /* constants            */
        id_node_struct id;          /* identifiers          */
        operator_node_struct opr;   /* operators            */
        opcode_node_struct opcode;  /* opcodes              */
        macro_node_struct macro;    /* macro execution      */
        data_node_struct data;      /* numeric data node    */
        string_node_struct str;		/* string node          */
        print_state_node_struct pr; /* print node           */
    };
    int number_of_ops;              /* number of operands   */
    struct parse_node** op;         /* operands             */
    struct parse_node* next;        /* next node in tree    */
    struct parse_node* prev;        /* previous node        */
} parse_node, * parse_node_ptr;


/* prototypes */
extern parse_node_ptr operator_node(int opr, int number_of_ops, ...);
extern parse_node_ptr opcode_node(int op, int mode, int number_of_ops, ...);
extern parse_node_ptr id_node(const char* name);
extern parse_node_ptr label_node(char* name);
extern parse_node_ptr macro_id_node(const char* name);
extern parse_node_ptr constant_node(int value, int is_program_counter);
extern parse_node_ptr string_node(const char* value);
extern parse_node_ptr macro_expand_node(const char* name, parse_node_ptr macro_params);
extern parse_node_ptr data_node(int data_node_size, parse_node_ptr data);
extern parse_node_ptr print_state_node(int op);

extern parse_node_ptr allocate_node(int number_of_ops);
extern int is_valid_parse_node(parse_node_ptr p);
extern int is_valid_parse_tree(void);
extern void remove_parse_node(parse_node_ptr p);
extern void free_parse_node(parse_node_ptr p);
extern void free_parse_tree(void);
extern int generate_list_node(const parse_node_ptr p);

extern parse_node_ptr head_node;
extern parse_node_ptr current_node;
