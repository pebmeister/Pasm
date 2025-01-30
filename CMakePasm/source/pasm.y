%no-lines

%{

// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

#pragma warning(disable:4065 4996 4244)

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


bool Debug_Bison = false;

#define B_TOK(str)          { if (Debug_Bison){ fprintf(console, "--------   BISON (%20s)    -------- \n\n", str); }}
#define E_TOK(str)          { if (Debug_Bison){ fprintf(console, "--------   BISON EX (%20s) -------- \n\n", str); }}

%}

%union
{
    int iValue;                 /* integer value */
    char* strValue;             /* string */
    char* sIndex;               /* symbol table pointer */
    struct parse_node *nPtr;    /* node pointer */
};

%token <iValue> INTEGER
%token <iValue> OPCODE
%token <sIndex> SYMBOL
%token <sIndex> LABEL
%token <sIndex> MACSYMBOL
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
    : program stmt                      { E_TOK("program stmt") expand_node($2);                                          }
    | program LABEL stmt                { 
    
                                            E_TOK("program LABEL stmt")
                                            expand_node(operator_node('=', 2, label_node($2), constant_node(program_counter, true))); 
                                            expand_node($3);                
                                        }
    | /* NULL */
    ;

stmt
    : opcode EOL                        { B_TOK("stmt opcode")          $$ = $1;                            }
    | directive EOL                     { B_TOK("stmt directive")       $$ = $1;                            }
    | symbol_value EOL                  { B_TOK("stmt symbol_value")    $$ = $1;                            }
    | symbol_assign EOL                 { B_TOK("stmt symbol_assign")   $$ = $1;                            }
    | pc_assign EOL                     { B_TOK("stmt pc_assign")       $$ = $1;                            }
    | ifexpr EOL                        { B_TOK("stmt ifexpr")          $$ = $1;                            }
    | loopexpr EOL                      { B_TOK("stmt loopexpr")        $$ = $1;                            }
    | regloopexpr EOL                   { B_TOK("stmt loopexpr")        $$ = $1;                            }
    | macrodef EOL                      { B_TOK("stmt macrodef")        $$ = $1;                            }
    | macrocall EOL                     { B_TOK("stmt macrocall")       $$ = $1;                            }
    | var_def EOL                       { B_TOK("stmt var_def")         $$ = $1;                            }
    | EOL                               { B_TOK("stmt EOL")             $$ = operator_node(STATEMENT, 0);   }
    ; 

stmt_list
    : stmt                              { B_TOK("stmt_list stmt")           $$ = $1;                                    }
    | stmt_list stmt                    { B_TOK("stmt_list stmt_list stmt") $$ = operator_node(STATEMENT, 2, $1, $2);   }
    ;

ifexpr
    : IF subexpr stmt_list ELSE stmt_list ENDIF     { B_TOK("ifexpr ELSE ENDIF")    $$ = operator_node(IF, 3, $2, $3, $5);  }
    | IF subexpr stmt_list ENDIF                    { B_TOK("ifexpr ENDIF")         $$ = operator_node(IF, 2, $2, $3);      }
    ;

loopexpr
    : REPEAT EOL stmt_list UNTIL subexpr                                        { B_TOK("loopexpr REPEAT")          $$ = operator_node(REPEAT, 2, $3, $5);                    }
    | DO stmt_list ENDDO subexpr                                                { B_TOK("loopexpr DO WHILE")        $$ = operator_node(DO, 2, $2, $4);                        }
    | WHILE subexpr EOL stmt_list WEND                                          { B_TOK("loopexpr WHILE DO")        $$ = operator_node(WHILE, 2, $2, $4);                     }
    | FOR symbol_assign TO subexpr EOL  stmt_list NEXT SYMBOL                   { B_TOK("loopexpr FOR NEXT")        $$ = operator_node(FOR, 4, $2, $4, $6, id_node($8));      }
    | FOR symbol_assign TO subexpr STEP subexpr EOL stmt_list NEXT SYMBOL       { B_TOK("loopexpr FOR NEXT STEP")   $$ = operator_node(FOR, 5, $2, $4, $8, id_node($10), $6); }
    ;

regloopexpr
    : FOR REGX '=' subexpr TO subexpr EOL stmt_list NEXT 'X'                    { B_TOK("regloopexpr FOR REGX TO")      $$ = operator_node(REGX, 4, $4, $6, $8, constant_node(1, 0));   }
    | FOR REGX '=' subexpr DOWNTO subexpr EOL stmt_list NEXT 'X'                { B_TOK("regloopexpr FOR REGX DOWNTO")  $$ = operator_node(REGX, 4, $4, $6, $8, constant_node(-1,0));   }
    | FOR REGY '=' subexpr TO subexpr EOL stmt_list NEXT 'Y'                    { B_TOK("regloopexpr FOR REGY TO")      $$ = operator_node(REGY, 4, $4, $6, $8, constant_node(1,0));    }
    | FOR REGY '=' subexpr DOWNTO subexpr EOL  stmt_list NEXT 'Y'               { B_TOK("regloopexpr FOR REGX DOWNTO")  $$ = operator_node(REGY, 4, $4, $6, $8, constant_node(-1,0));   }
    ;

expr_list
    : subexpr                           { B_TOK("expr_list subexpr")                    $$ = operator_node(EXPRLIST, 1, $1);                    }
    | STRING_LITERAL                    { B_TOK("expr_list STRING_LITERAL")             $$ = operator_node(EXPRLIST, 1, string_node($1));       }
    | expr_list ',' subexpr             { B_TOK("expr_list expr_list subexpr")          $$ = operator_node(EXPRLIST, 2, $1, $3);                }
    | expr_list ',' STRING_LITERAL      { B_TOK("expr_list expr_list STRING_LITERAL")   $$ = operator_node(EXPRLIST, 2, $1, string_node($3));   }
    ;

macrodef
    : MACRO MACSYMBOL stmt_list ENDMACRO{
                                            B_TOK("macrodef MACSYMBOL")
                                            in_macro_definition++;
                                            $$ = operator_node(MACRO, 2, macro_id_node($2), $3);
                                            in_macro_definition--;
                                        }
    ;

macrocall
    : MACSYMBOL                         { B_TOK("macrocall MACSYMBOL")  $$ = macro_expand_node($1, NULL);  }
    | MACSYMBOL expr_list               { B_TOK("macrocall MACSYMBOL")  $$ = macro_expand_node($1, $2);  }
    ;

symbol_list
    : SYMBOL                            { B_TOK("symbol_list SYMBOL")                       $$ = operator_node(EXPRLIST, 1, id_node($1));           }
    | symbol_list ',' SYMBOL            { B_TOK("symbol_list symbol_list SYMBOL")           $$ = operator_node(EXPRLIST, 2, $1, id_node($3));       }
    | SYMBOL '=' subexpr                { B_TOK("symbol_list SYMBOL = subexpr")             $$ = operator_node(EXPRLIST, 2, id_node($1), $3);       }
    | symbol_list ',' SYMBOL '=' subexpr{ B_TOK("symbol_list symbol_list SYMBOL = subexpr") $$ = operator_node(EXPRLIST, 3, $1, id_node($3), $5);   }
    ;

var_def
    : VAR symbol_list                   { B_TOK("var_def symbol_list")  $$ = operator_node(VAR, 1, $2); }
    ;

symbol_assign
    : LABEL '=' subexpr                 { B_TOK("symbol_assign LABEL =")    $$ = operator_node('=', 2, id_node($1), $3);    }
    | LABEL EQU subexpr                 { B_TOK("symbol_assign LABEL EQU")  $$ = operator_node(EQU, 2, id_node($1), $3);    }
    | SYMBOL '=' subexpr                { B_TOK("symbol_assign SYMBOL =")   $$ = operator_node(EQU, 2, id_node($1), $3);    }
    | SYMBOL EQU subexpr                { B_TOK("symbol_assign SYMBOL EQU") $$ = operator_node(EQU, 2, id_node($1), $3);    }
    ;

pc_assign
    : PCASSIGN '=' subexpr              { B_TOK("pc_assign PCASSIGN =")     $$ = operator_node(PCASSIGN, 1, $3);    }
    | PCASSIGN EQU subexpr              { B_TOK("pc_assign PCASSIGN EQU")   $$ = operator_node(PCASSIGN, 1, $3);    }
    ;

symbol_value
    : SYMBOL                            {
                                            symbol_table_ptr sym = look_up_symbol($1);
                                            if (sym && sym->is_macro_name)
                                            {
                                                B_TOK("symbol_value SYMBOL macro_expand_node")
                                                $$ = macro_expand_node($1, NULL);
                                            }
                                            else
                                            {
                                                B_TOK("symbol_value SYMBOL label_node")
                                                $$ = operator_node('=', 2, label_node($1), constant_node(program_counter, true));
                                            }
                                        }
    ;

opcode
    : OPCODE                            { B_TOK("opcode OPCODE")                    $$ = opcode_node($1, i, 0);         }
    | OPCODE '#' subexpr                { B_TOK("opcode OPCODE # subexpr")          $$ = opcode_node($1, I, 1, $3);     }
    | OPCODE expr                       { B_TOK("opcode OPCODE expr")               $$ = opcode_node($1, a, 1, $2);     }
    | OPCODE expr ',' 'X'               { B_TOK("opcode OPCODE expr , X")           $$ = opcode_node($1, ax, 1, $2);    }
    | OPCODE expr ',' 'Y'               { B_TOK("opcode OPCODE expr , Y")           $$ = opcode_node($1, ay, 1, $2);    }
    | OPCODE '(' subexpr ')'            { B_TOK("opcode OPCODE ( subexpr )")        $$ = opcode_node($1, ind, 1, $3);   }
    | OPCODE '(' subexpr ',' 'X' ')'    { B_TOK("opcode OPCODE ( subexpr , X)")     $$ = opcode_node($1, aix, 1, $3);   }
    | OPCODE '(' subexpr ')' ',' 'Y'    { B_TOK("opcode OPCODE ( subexpr ) , Y")    $$ = opcode_node($1, izy, 1, $3);   }
    ;

directive
    : ORG subexpr                       { B_TOK("directive ORG")        $$ = operator_node(ORG, 1, $2);                 }
    | DS subexpr                        { B_TOK("directive DS")         $$ = operator_node(DS, 1, $2);                  }
    | BYTE expr_list                    { B_TOK("directive BYTE")       $$ = data_node(data_byte, $2);                  }
    | WORD expr_list                    { B_TOK("directive WORD")       $$ = data_node(data_word, $2);                  }
    | STR expr_list                     { B_TOK("directive STR")        $$ = data_node(data_string, $2);                }
    | FILL subexpr ',' subexpr          { B_TOK("directive FILL")       $$ = operator_node(FILL, 2, $2, $4);            }
    | PRINT                             { B_TOK("directive PRINT")      $$ = operator_node(PRINT, 0);                   }
    | PRINT expr_list                   { B_TOK("directive PRINT")      $$ = operator_node(PRINT, 1, $2);               }
    | PRINTALL                          { B_TOK("directive PRINTALL")   $$ = operator_node(PRINTALL, 0);                }
    | PRINTALL expr_list                { B_TOK("directive PRINTALL")   $$ = operator_node(PRINTALL, 1, $2);            }
    | PRINTON                           { B_TOK("directive PRINTON")    $$ = print_state_node(1);                       }
    | PRINTOFF                          { B_TOK("directive PRINTOFF")   $$ = print_state_node(0);                       }
    | SECTION SYMBOL                    { B_TOK("directive SECTION")    $$ = operator_node(SECTION, 1, id_node($2));    }
    | ENDSECTION                        { B_TOK("directive ENDSECTION") $$ = operator_node(ENDSECTION, 0);              }
    | END                               { B_TOK("directive END")        $$ = operator_node(END, 0);                     }
    | INC STRING_LITERAL                { B_TOK("directive INC")        $$ = operator_node(INC, 1, string_node($2));    }
    | LOAD STRING_LITERAL               { B_TOK("directive LOAD")       $$ = operator_node(LOAD, 1, string_node($2));   }
    ;

expr
    : INTEGER                           { B_TOK("expr INTEGER")                     $$ = constant_node($1, false);                              }
    | SYMBOL                            { B_TOK("expr SYMBOL")                      $$ = id_node($1);                                           }
    | '-'  INTEGER %prec UMINUS         { B_TOK("expr INTEGER UMINUS")              $$ = operator_node(UMINUS, 1, constant_node($2, false));    }
    | '-' '(' subexpr ')' %prec UMINUS  { B_TOK("expr subexpr UMINUS")              $$ = operator_node(UMINUS, 1, $3);                          }
    | '~' subexpr %prec UMINUS          { B_TOK("expr subexpr ~")                   $$ = operator_node('~', 1, $2);                             }
    | '<' subexpr %prec UMINUS          { B_TOK("expr subexpr LOBYTE")              $$ = operator_node(LOBYTE, 1, $2);                          }
    | '>' subexpr %prec UMINUS          { B_TOK("expr subexpr HIBYTE")              $$ = operator_node(HIBYTE, 1, $2);                          }
    | NOT subexpr %prec UMINUS          { B_TOK("expr subexpr NOT")                 $$ = operator_node(NOT, 1, $2);                             }
    | subexpr OR subexpr                { B_TOK("expr subexpr OR subexpr")          $$ = operator_node(OR, 2, $1, $3);                          }
    | subexpr AND subexpr               { B_TOK("expr subexpr AND subexpr")         $$ = operator_node(AND, 2, $1, $3);                         }
    | subexpr SHIFT_LEFT subexpr        { B_TOK("expr subexpr SHIFT_LEFT subexpr")  $$ = operator_node(SHIFT_LEFT, 2, $1, $3);                  }
    | subexpr SHIFT_RIGHT subexpr       { B_TOK("expr subexpr SHIFT_RIGHT subexpr") $$ = operator_node(SHIFT_RIGHT, 2, $1, $3);                 }
    | subexpr '<' subexpr               { B_TOK("expr subexpr < subexpr")           $$ = operator_node('<', 2, $1, $3);                         }
    | subexpr '>' subexpr               { B_TOK("expr subexpr > subexpr")           $$ = operator_node('>', 2, $1, $3);                         }
    | subexpr GE subexpr                { B_TOK("expr subexpr GE subexpr")          $$ = operator_node(GE, 2, $1, $3);                          }
    | subexpr LE subexpr                { B_TOK("expr subexpr LE subexpr")          $$ = operator_node(LE, 2, $1, $3);                          }
    | subexpr NE subexpr                { B_TOK("expr subexpr NE subexpr")          $$ = operator_node(NE, 2, $1, $3);                          }
    | subexpr EQ subexpr                { B_TOK("expr subexpr EQ subexpr")          $$ = operator_node(EQ, 2, $1, $3);                          }
    | subexpr BIT_AND subexpr           { B_TOK("expr subexpr BIT_AND subexpr")     $$ = operator_node(BIT_AND, 2, $1, $3);                     }
    | subexpr BIT_OR subexpr            { B_TOK("expr subexpr BIT_OR subexpr")      $$ = operator_node(BIT_OR, 2, $1, $3);                      }
    | subexpr '^' subexpr               { B_TOK("expr subexpr ^ subexpr")           $$ = operator_node('^', 2, $1, $3);                         }
    | subexpr '+' subexpr               { B_TOK("expr subexpr + subexpr")           $$ = operator_node('+', 2, $1, $3);                         }
    | subexpr '-' subexpr               { B_TOK("expr subexpr - subexpr")           $$ = operator_node('-', 2, $1, $3);                         }
    | subexpr '*' subexpr               { B_TOK("expr subexpr * subexpr")           $$ = operator_node('*', 2, $1, $3);                         }
    | subexpr '/' subexpr               { B_TOK("expr subexpr / subexpr")           $$ = operator_node('/', 2, $1, $3);                         }
    | '-'                               { B_TOK("expr id_node -")                   $$ = id_node("-");                                          }
    | '+'                               { B_TOK("expr id_node +")                   $$ = id_node("+");                                          }
    ;

subexpr
    : expr                              { B_TOK("subexpr expr")         $$ = $1;                                    }
    | '*'                               { B_TOK("subexpr PC")           $$ = constant_node(program_counter, true);  }
    | '(' subexpr ')'                   { B_TOK("subexpr ( subexpr )")  $$ = $2;                                    }
    ;

%%