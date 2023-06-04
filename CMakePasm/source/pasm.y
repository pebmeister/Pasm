%no-lines
%expect 2
%glr-parser
%expect-rr 27

%{

// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

#pragma warning(disable:4065 4996)

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "expand.h"
#include "flex.h"
#include "opcodes.h"
#include "pasm.h"
#include "pasm.tab.h"
#include "node.h" 
#include "sym.h" 

%}

%union 
{
    int iValue;                 /* integer value */
    char* strValue;      		/* string */
    char* sIndex;               /* symbol table pointer */
    struct parse_node *nPtr;    /* node pointer */
};

%token <iValue> INTEGER
%token <iValue> OPCODE
%token <sIndex> SYMBOL MACROPARAM
%token <strValue> STRING_LITERAL

%token WHILE ENDDO REPEAT UNTIL IF PRINT PRINTALL EQU ORG PCASSIGN 
%token END DO MACRO ENDMACRO ENDIF WEND STATEMENT EXPRLIST STR
%token FOR NEXT TO DOWNTO STEP NOT
%token EOL BYTE WORD LOBYTE HIBYTE DS INC LOAD
%token REGX REGY VAR  FILL PRINTON PRINTOFF
%token SECTION ENDSECTION
 
%nonassoc ELSE UMINUS '~'

%left SHIFT_LEFT SHIFT_RIGHT
%left OR AND GE LE EQ NE '>' '<' 
%left BIT_OR BIT_AND '^'
%left '+' '-'
%left '*' '/'

%type <nPtr> stmt_list stmt
%type <nPtr> opcode regloopexpr
%type <nPtr> macrodef macrocall expr_list symbol_list
%type <nPtr> symbol_assign symbol_value var_def pc_assign 
%type <nPtr> expr subexpr ifexpr loopexpr
%type <nPtr> directive
%%

program 
    : program stmt                      { expand_node($2);                                          }
    | program symbol_value stmt         { expand_node($2); expand_node($3);                         }
    | /* NULL */
    ;

stmt    
    : opcode EOL                        { $$ = $1;                                                  }
    | directive EOL                     { $$ = $1;                                                  }
    | symbol_value EOL                  { $$ = $1;                                                  }
    | symbol_assign EOL                 { $$ = $1;                                                  }
    | pc_assign EOL                     { $$ = $1;                                                  }
    | ifexpr EOL                        { $$ = $1;                                                  }
    | loopexpr EOL                      { $$ = $1;                                                  }
    | regloopexpr EOL                   { $$ = $1;                                                  }
    | macrodef EOL                      { $$ = $1;                                                  }
    | macrocall EOL                     { $$ = $1;                                                  }
    | var_def EOL                       { $$ = $1;                                                  }
    | EOL                               { $$ = operator_node(STATEMENT, 0);                         }
    ;

stmt_list  
    : stmt                              { $$ = $1;                                                  }
    | stmt_list stmt                    { $$ = operator_node(STATEMENT, 2, $1, $2);                 }
    ;

ifexpr 
    : IF subexpr stmt_list ELSE stmt_list ENDIF                                 { $$ = operator_node(IF, 3, $2, $3, $5);                    }
    | IF subexpr stmt_list ENDIF                                                { $$ = operator_node(IF, 2, $2, $3);                        }
    ;
     
loopexpr
    : REPEAT EOL stmt_list UNTIL subexpr                                        { $$ = operator_node(REPEAT, 2, $3, $5);                    }
    | DO stmt_list ENDDO subexpr                                                { $$ = operator_node(DO, 2, $2, $4);                        }
    | WHILE subexpr EOL stmt_list WEND                                          { $$ = operator_node(WHILE, 2, $2, $4);                     }   
    | FOR symbol_assign TO subexpr EOL  stmt_list NEXT SYMBOL                   { $$ = operator_node(FOR, 4, $2, $4, $6, id_node($8));      }
    | FOR symbol_assign TO subexpr STEP subexpr EOL stmt_list NEXT SYMBOL       { $$ = operator_node(FOR, 5, $2, $4, $8, id_node($10), $6); }
    ;

regloopexpr
    : FOR REGX '=' subexpr TO subexpr EOL stmt_list NEXT 'X'                    { $$ = operator_node(REGX, 4, $4, $6, $8, constant_node(1, 0));       }
    | FOR REGX '=' subexpr DOWNTO subexpr EOL stmt_list NEXT 'X'                { $$ = operator_node(REGX, 4, $4, $6, $8, constant_node(-1,0));       }
    | FOR REGY '=' subexpr TO subexpr EOL stmt_list NEXT 'Y'                    { $$ = operator_node(REGY, 4, $4, $6, $8, constant_node(1,0));        }
    | FOR REGY '=' subexpr DOWNTO subexpr EOL  stmt_list NEXT 'Y'               { $$ = operator_node(REGY, 4, $4, $6, $8, constant_node(-1,0));       }
    ;

expr_list
    : subexpr                           { $$ = operator_node(EXPRLIST, 1, $1);                      }
    | STRING_LITERAL                    { $$ = operator_node(EXPRLIST, 1, string_node($1));         }
    | expr_list ',' subexpr             { $$ = operator_node(EXPRLIST, 2, $1, $3);                  }
    | expr_list ',' STRING_LITERAL      { $$ = operator_node(EXPRLIST, 2, $1, string_node($3));     }
    ;
          
macrodef
    : MACRO SYMBOL stmt_list ENDMACRO   {
                                            in_macro_definition++;
                                            $$ = operator_node(MACRO, 2, macro_id_node($2), $3);
                                            in_macro_definition--;
                                        }
    ;

macrocall
    : SYMBOL expr_list                  { $$ = macro_expand_node($1, $2);                           }
    ;

symbol_list
    : SYMBOL                            { $$ = operator_node(EXPRLIST, 1, id_node($1));             }
    | symbol_list ',' SYMBOL            { $$ = operator_node(EXPRLIST, 2, $1, id_node($3));         }
    | SYMBOL '=' subexpr                { $$ = operator_node(EXPRLIST, 2, id_node($1), $3);         }
    | symbol_list ',' SYMBOL '=' subexpr{ $$ = operator_node(EXPRLIST, 3, $1, id_node($3), $5);     }
    ;

var_def
    : VAR symbol_list                   { $$ = operator_node(VAR, 1, $2);                           }
    ;

symbol_assign
    : SYMBOL '=' subexpr                { $$ = operator_node('=', 2, id_node($1), $3);              }
    | SYMBOL EQU subexpr                { $$ = operator_node(EQU, 2, id_node($1), $3);              }
    ;
 
pc_assign
    : PCASSIGN '=' subexpr              { $$ = operator_node(PCASSIGN, 1, $3);                      }
    | PCASSIGN EQU subexpr              { $$ = operator_node(PCASSIGN, 1, $3);                      }
    ;

symbol_value
    : SYMBOL                            {   
                                            symbol_table_ptr sym = look_up_symbol($1);
                                            if (sym && sym->is_macro_name)
                                            {
                                                $$ = macro_expand_node($1, NULL);
                                            }
                                            else
                                            {
                                                $$ = operator_node('=', 2, label_node($1), constant_node(program_counter, true));
                                            }
                                        }
    ;

opcode
    : OPCODE                            { $$ = opcode_node($1, i, 0);                               }
    | OPCODE '#' subexpr                { $$ = opcode_node($1, I, 1, $3);                           }
    | OPCODE expr                       { $$ = opcode_node($1, a, 1, $2);                           }
    | OPCODE expr ',' 'X'               { $$ = opcode_node($1, ax, 1, $2);                          }
    | OPCODE expr ',' 'Y'               { $$ = opcode_node($1, ay, 1, $2);                          }
    | OPCODE '(' subexpr ')'            { $$ = opcode_node($1, ind, 1, $3);                         }
    | OPCODE '(' subexpr ',' 'X' ')'    { $$ = opcode_node($1, aix, 1, $3);                         }
    | OPCODE '(' subexpr ')' ',' 'Y'    { $$ = opcode_node($1, izy, 1, $3);                         }
    ;

directive
    : ORG subexpr                       { $$ = operator_node(ORG, 1, $2);                           }
    | DS subexpr                        { $$ = operator_node(DS, 1, $2);                            }
    | BYTE expr_list                    { $$ = data_node(data_byte, $2);                            }
    | WORD expr_list                    { $$ = data_node(data_word, $2);                            }
    | STR expr_list					    { $$ = data_node(data_string, $2);	 					    }
    | FILL subexpr ',' subexpr          { $$ = operator_node(FILL, 2, $2, $4);                      }
    | PRINT                             { $$ = operator_node(PRINT, 0);                             }
    | PRINT expr_list                   { $$ = operator_node(PRINT, 1, $2);                         }
    | PRINTALL                          { $$ = operator_node(PRINTALL, 0);                          }
    | PRINTALL expr_list                { $$ = operator_node(PRINTALL, 1, $2);                      }    
    | PRINTON                           { $$ = print_state_node(1);                                 }
    | PRINTOFF                          { $$ = print_state_node(0);                                 }
    | SECTION SYMBOL                    { $$ = operator_node(SECTION, 1, id_node($2));              }
    | ENDSECTION                        { $$ = operator_node(ENDSECTION, 0);                        }
    | END                               { $$ = operator_node(END, 0);                               }
    | INC STRING_LITERAL                { $$ = operator_node(INC, 1, string_node($2));              }
    | LOAD STRING_LITERAL               { $$ = operator_node(LOAD, 1, string_node($2));             }
    ;

expr
    : INTEGER                           { $$ = constant_node($1, false);                            }
    | SYMBOL                            { $$ = id_node($1);                                         }
    | '-' subexpr %prec UMINUS          { $$ = operator_node(UMINUS, 1, $2);                        }
    | '~' subexpr %prec UMINUS          { $$ = operator_node('~', 1, $2);                           }
    | '<' subexpr %prec UMINUS          { $$ = operator_node(LOBYTE, 1, $2);                        }
    | '>' subexpr %prec UMINUS          { $$ = operator_node(HIBYTE, 1, $2);                        }
    | NOT subexpr %prec UMINUS          { $$ = operator_node(NOT, 1, $2);                           }
    | subexpr OR subexpr                { $$ = operator_node(OR, 2, $1, $3);                        }
    | subexpr AND subexpr               { $$ = operator_node(AND, 2, $1, $3);                       }
    | subexpr SHIFT_LEFT subexpr        { $$ = operator_node(SHIFT_LEFT, 2, $1, $3);                }
    | subexpr SHIFT_RIGHT subexpr       { $$ = operator_node(SHIFT_RIGHT, 2, $1, $3);               }
    | subexpr '<' subexpr               { $$ = operator_node('<', 2, $1, $3);                       }
    | subexpr '>' subexpr               { $$ = operator_node('>', 2, $1, $3);                       }
    | subexpr GE subexpr                { $$ = operator_node(GE, 2, $1, $3);                        }
    | subexpr LE subexpr                { $$ = operator_node(LE, 2, $1, $3);                        }
    | subexpr NE subexpr                { $$ = operator_node(NE, 2, $1, $3);                        }
    | subexpr EQ subexpr                { $$ = operator_node(EQ, 2, $1, $3);                        }
    | subexpr BIT_AND subexpr           { $$ = operator_node(BIT_AND, 2, $1, $3);                   }
    | subexpr BIT_OR subexpr            { $$ = operator_node(BIT_OR, 2, $1, $3);                    }
    | subexpr '^' subexpr               { $$ = operator_node('^', 2, $1, $3);                       }
    | subexpr '+' subexpr               { $$ = operator_node('+', 2, $1, $3);                       }
    | subexpr '-' subexpr               { $$ = operator_node('-', 2, $1, $3);                       }
    | subexpr '*' subexpr               { $$ = operator_node('*', 2, $1, $3);                       }
    | subexpr '/' subexpr               { $$ = operator_node('/', 2, $1, $3);                       }
    | '-'                               { $$ = id_node("-");                                        }
    | '+'                               { $$ = id_node("+");                                        }
    ;

subexpr
    : expr                              { $$ = $1;                                                  }
    | '*'                               { $$ = constant_node(program_counter, true);                }
    | '(' subexpr ')'                   { $$ = $2;                                                  }
    ;

%%
