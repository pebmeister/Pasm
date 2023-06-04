%option case-insensitive yylineno noyywrap
 
%{
#pragma warning(disable:6011 6387 4996 4090)


#include <stdbool.h>

#include "flex.h"
#include "pasm.h"
#include "str.h"
#include "sym.h"
#include "node.h"
#include "opcodes.h"
#include "pasm.tab.h"
#include "stacks.h"
#include "memory.h"

int do_count = 0;

#define OP(op)              { yylval.iValue = _ ## op; return OPCODE; }
#define OP_REL(op)          { yylval.iValue = _ ## op ## 0 + yytext[3] - '0'; return OPCODE; }
#define OP_ILLEGAL(op)      { OP(op); }
#define OP_65C02(op)        { OP(op); }
#define OP_65C02_REL(op)    { OP_REL(op); }
#define INT(off, base)      { yylval.iValue = (int) strtol (yytext + off, NULL, base); \
                                return INTEGER; \
                            }
#define INT_1BYTE           { \
                                char* tmpStr = sanitize_string(yytext); \
                                yylval.iValue = (unsigned char)tmpStr[1] & 0xFF; \
                                free(tmpStr); \
                                return INTEGER; \
                            }

#define SKIP_WHITE(p)       { while (*p != ' ' && *p != '\t') p += 1; while (*p == ' ' || *p == '\t') p += 1; }     
%}

%x C_COMMENT
%xx IF_DEF

ES                          (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
LOGOR                       [|][|]
BITOR                       [|]
LOGAND                      [&][&]
BITAND                      [&]
STRVALUE					(\"([^"\\\n]|{ES})*\")+
SYMB                        @?[A-Za-z_][A-Za-z0-9_.]*
%%

"ORA"                       OP(ora)
"AND"                       OP(and)
"EOR"                       OP(eor)
"ADC"                       OP(adc)
"SBC"                       OP(sbc)
"CMP"                       OP(cmp)
"CPX"                       OP(cpx)
"CPY"                       OP(cpy)
"DEC"                       OP(dec)
"DEX"                       OP(dex)
"DEY"                       OP(dey)
"INC"                       OP(inc)
"INX"                       OP(inx)
"INY"                       OP(iny)
"ASL"                       OP(asl)
"ROL"                       OP(rol)
"LSR"                       OP(lsr)
"ROR"                       OP(ror)
"LDA"                       OP(lda)
"STA"                       OP(sta)
"LDX"                       OP(ldx)
"STX"                       OP(stx)
"LDY"                       OP(ldy)
"STY"                       OP(sty)
"TAX"                       OP(tax)
"TXA"                       OP(txa)
"TAY"                       OP(tay)
"TYA"                       OP(tya)
"TSX"                       OP(tsx)
"TXS"                       OP(txs)
"PLA"                       OP(pla)
"PHA"                       OP(pha)
"PLP"                       OP(plp)
"PHP"                       OP(php)
"BPL"                       OP(bpl)
"BMI"                       OP(bmi)
"BVC"                       OP(bvc)
"BVS"                       OP(bvs)
"BCC"                       OP(bcc)
"BCS"                       OP(bcs)
"BNE"                       OP(bne)
"BEQ"                       OP(beq)
"BRK"                       OP(brk)
"RTI"                       OP(rti)
"JSR"                       OP(jsr)
"RTS"                       OP(rts)
"JMP"                       OP(jmp)
"BIT"                       OP(bit)
"CLC"                       OP(clc)
"SEC"                       OP(sec)
"CLD"                       OP(cld)
"SED"                       OP(sed)
"CLI"                       OP(cli)
"SEI"                       OP(sei)
"CLV"                       OP(clv)
"NOP"                       OP(nop)
"SLO"                       OP_ILLEGAL(slo)
"RLA"                       OP_ILLEGAL(rla)
"SRE"                       OP_ILLEGAL(sre)
"RRA"                       OP_ILLEGAL(rra)
"SAX"                       OP_ILLEGAL(sax)
"LAX"                       OP_ILLEGAL(lax)
"DCP"                       OP_ILLEGAL(dcp)
"ISC"                       OP_ILLEGAL(isc)
"ANC"                       OP_ILLEGAL(anc)
"ANC2"                      OP_ILLEGAL(anc2)
"ALR"                       OP_ILLEGAL(alr)
"ARR"                       OP_ILLEGAL(arr)
"XAA"                       OP_ILLEGAL(xaa)
"AXS"                       OP_ILLEGAL(axs)
"USBC"                      OP_ILLEGAL(usbc)
"AHX"                       OP_ILLEGAL(ahx)
"SHY"                       OP_ILLEGAL(shy)
"SHX"                       OP_ILLEGAL(shx)
"TAS"                       OP_ILLEGAL(tas)
"LAS"                       OP_ILLEGAL(las)
"BRA"                       OP_65C02(bra)
"PHX"                       OP_65C02(phx)
"PHY"                       OP_65C02(phy)
"PLX"                       OP_65C02(plx)
"PLY"                       OP_65C02(ply)
"STZ"                       OP_65C02(stz)
"TRB"                       OP_65C02(trb)
"TSB"                       OP_65C02(tsb)
"STP"                       OP_65C02(stp)
"WAI"                       OP_65C02(wai)
"BBR"[0-7]                  OP_65C02_REL(bbr)
"BBS"[0-7]                  OP_65C02_REL(bbs)
"RMB"[0-7]                  OP_65C02_REL(rmb)
"SMB"[0-7]                  OP_65C02_REL(smb)

[\xD\xA]|[\xA]|[\xD]        return EOL;
".IFDEF"[ \t]+{SYMB}        {
                                int last_if = 0;
                                ifdef_stack->push(ifdef_stack->instance, &last_if);

                                char* p = yytext;
                                SKIP_WHITE(p)

                                symbol_table_ptr sym = look_up_symbol(p); 
                                if (sym == NULL)
                                {
                                    BEGIN(IF_DEF);
                                }
                            }
".IFNDEF"[ \t]+{SYMB}       {
                                int last_if = 0;
                                ifdef_stack->push(ifdef_stack->instance, &last_if);

                                char* p = yytext;
                                SKIP_WHITE(p)

                                symbol_table_ptr sym = look_up_symbol(p); 
                                if (sym != NULL)
                                {
                                    BEGIN(IF_DEF);
                                }
                            }
<IF_DEF>".IF"               { 
                                int last_if = 1; 
                                ifdef_stack->push(ifdef_stack->instance, &last_if); 
                            }
<IF_DEF>".ELSE"             {
                                int last_if = * (int*) ifdef_stack->top(ifdef_stack->instance);
                                if (last_if == 0)
                                {
                                    if (final_pass)
                                    {
                                        generate_list_node(NULL);
                                    }
                                    BEGIN(INITIAL);
                                }
                            }
<IF_DEF>".ENDIF"            {
                                ifdef_stack->pop(ifdef_stack->instance); 
                                if (ifdef_stack->instance->index < 0)
                                {
                                    if (final_pass)
                                    {
                                        generate_list_node(NULL);
                                    }
                                    BEGIN(INITIAL); 
                                }
                            }
<IF_DEF>.|[\n]              { /* ignore ifdef false */ }
".ENDIF"                    {  
                                if (ifdef_stack->instance->index < 0) return ENDIF;

                                int last_if = * (int*) ifdef_stack->pop(ifdef_stack->instance);
                                if (last_if != 0) 
                                {
                                    return ENDIF; 
                                }
                            }
".IF"                       { int last_if = 1; ifdef_stack->push(ifdef_stack->instance, &last_if); return IF; }
".ELSE"                     {
                                int last_if = * (int*) ifdef_stack->top(ifdef_stack->instance);
                                if (last_if != 0)
                                {
                                    return ELSE;
                                }
                                else
                                {
                                    BEGIN(IF_DEF);
                                }
                            }
".BYTE"                     return BYTE;
".DB"                       return BYTE;
".WORD"                     return WORD;
".DW"                       return WORD;
".DCW"                      return WORD;
".DS"                       return DS;
".EQU"                      return EQU;
"NOT"                       return NOT;
">="                        return GE;
"<="                        return LE;
"=="                        return EQ;
"!="                        return NE;
"<>"                        return NE;
"<<"                        return SHIFT_LEFT;
">>"                        return SHIFT_RIGHT;
{LOGOR}                     return OR;
{BITOR}                     return BIT_OR;
{LOGAND}                    return AND;
{BITAND}                    return BIT_AND;
".REPEAT"                   return REPEAT;
".UNTIL"                    return UNTIL;
".END"                      return END; 
".PRINT"                    return PRINT;
"\?"                        return PRINT;
".PRINTALL"                 return PRINTALL;                   
"\?\?"                      return PRINTALL;
".FOR"                      return FOR;
".NEXT"                     return NEXT;
".STEP"                     return STEP;
".TO"                       return TO;
".DOWNTO"                   return DOWNTO;
".STR"						return STR;
".TEXT"						return STR;
".STRING"				    return STR;
".ORG"                      return ORG;
".SECTION"                  return SECTION;
".ENDSECTION"               return ENDSECTION;
".SECT"                     return SECTION;
".ENDS"                     return ENDSECTION;
".INC"                      return INC;
".LOAD"                     return LOAD;
".FILL"                     return FILL;
".WHILE"                    { 
                                if (do_count > 0)
                                {
                                    --do_count; 
                                    return ENDDO;
                                }
                                return WHILE;
                            }
".WEND"                     { return WEND; }
".DO"                       { ++do_count; return DO; }
".MACRO"                    { in_macro_definition++; return MACRO; }
".MAC"                      { in_macro_definition++; return MACRO; }
".ENDMACRO"                 { in_macro_definition--; return ENDMACRO; }
".ENDM"                     { in_macro_definition--; return ENDMACRO; }
".REGX"                     { return REGX; }
".REGY"                     { return REGY; }
".VAR"                      { return VAR;  }
".6502"[ \t]*"ON"           { cpu_mode = cpu_6502; }
".65C02"[ \t]*"ON"          { cpu_mode = cpu_65C02; }
".ILLEGAL"[ \t]*"ON"        { allow_illegal_op_codes = true; }
".ILLEGAL"[ \t]*"OFF"       { allow_illegal_op_codes = false; } 
".WARN"[ \t]*"ON"           { ignore_warnings = false; }
".WARN"[ \t]*"OFF"          { ignore_warnings = true; }
".C64"                      { output_file_format = c64; }
".PRINT"[ \t]"ON"           { return PRINTON; }
".PRINT"[ \t]"OFF"          { return PRINTOFF; }
^"-"                        { yylval.strValue = STRDUP(yytext); return SYMBOL; }
^"+"                        { yylval.strValue = STRDUP(yytext); return SYMBOL; }
[\\][1-9]+[0-9]*            {
                                if (in_macro_definition == 0)
                                    REJECT;
                                yylval.strValue = STRDUP(yytext); 
                                return SYMBOL;
                            }
[\+|\-]+                    {
                                if (strlen(yytext) > 1)
                                {
                                    yylval.strValue = STRDUP(yytext); 
                                    return SYMBOL;
                                }
                                else
                                {
                                    return *yytext;
                                }
                            }
[\'].[\']                   INT_1BYTE
[\'][\\].[\']               INT_1BYTE
[\"].[\"]                   INT_1BYTE
[\"][\\].[\'"]              INT_1BYTE
$[0-9A-Fa-f]*               INT(1, 16)
[\'][\\]x[0-9A-Fa-f]+[\']   INT(3, 16)
[0-9]*                      INT(0, 10)
%[0-1]*                     INT(1, 2)
0b[0-1]*                    INT(2, 2)
[\'][\\][0-7]{3}[\']        INT(2, 8)

{STRVALUE}					{ yylval.strValue = STRDUP(yytext); return STRING_LITERAL; }
"*"[ \t]*[=]                { unput('='); return PCASSIGN; }
"*"[ \t]*".EQU"             { unput('='); return PCASSIGN; }
"X"                         { return 'X'; }
"Y"                         { return 'Y'; }
{SYMB}:?                    { yylval.strValue = STRDUP(yytext); return SYMBOL; }
"/*"                        { BEGIN(C_COMMENT); }
<C_COMMENT>"*/"             { BEGIN(INITIAL); }
<C_COMMENT>.|[\n]           { /* ignore comments */ }
;[^\n]*                     { /* ignore comments */ }
[/][/][^\n]*                { /* ignore comments */ }

"~"                         { return '~'; }
"^"                         { return '^'; }
[-<>=+*/#,();=\n]           { return *yytext; }
[ \t]+                      { /* ignore white space */ }
<<EOF>>                     {
                                yylineno++;
                                if (final_pass) generate_list_node(NULL);
                                
                                yypop_buffer_state();
                                if (file_stack->instance->index >= 0)
                                {

                                    const file_line_stack_entry* file_line = file_stack->pop(file_stack->instance);
                                    current_file_name = file_line->file;
                                    yylineno = file_line->line;

                                    if (final_pass) generate_list_node(NULL);
                                }
                                if ( !YY_CURRENT_BUFFER )
                                {
                                    yyterminate();
                                }
                            }      
.                           { yyerror("Parse: syntax error"); }

%%
