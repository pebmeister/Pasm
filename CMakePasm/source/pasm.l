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
static char* last_token = nullptr;

// #define DEBUG_TOKEN


#ifdef DEBUG_TOKEN
#define P_TOK(str)          { fprintf(console, "TOK (%10s) %s\n", (yytext[0] == '\n' ? "EOL" : yytext), (str)); if (yytext[0] == '\n') fprintf(console, "\n"); }
#define P_C_TOK(ch)         { fprintf(console, "TOK (%10s) %c\n", yytext, ch ); }
#else
#define P_TOK(str)
#define P_C_TOK(ch)
#endif



#define SV_TOK              { if (last_token != nullptr) FREE(last_token); last_token = (char*)STRDUP(yytext); }

#define OP(op)              { P_TOK("OPCODE") SV_TOK yylval.iValue = _ ## op; return OPCODE; }
#define OP_REL(op)          { P_TOK("OPCODE") SV_TOK yylval.iValue = _ ## op ## 0 + yytext[3] - '0'; return OPCODE; }
#define OP_ILLEGAL(op)      { OP(op); }
#define OP_65C02(op)        { OP(op); }
#define OP_65C02_REL(op)    { OP_REL(op); }
#define INT(off, base)      { \
                                P_TOK("INTEGER") \
                                yylval.iValue = (int) strtol (yytext + off, NULL, base);\
                                return INTEGER;\
                            }
#define INT_1BYTE           {\
                                \
                                P_TOK("(1byte) INTEGER") \
                                char* tmpStr = sanitize_string(yytext);\
                                yylval.iValue = (unsigned char)tmpStr[1] & 0xFF;\
                                free(tmpStr);\
                                return INTEGER;\
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

"ORA"                       { OP(ora) }
"AND"                       { OP(and) }
"EOR"                       { OP(eor) }
"ADC"                       { OP(adc) }
"SBC"                       { OP(sbc) }
"CMP"                       { OP(cmp) }
"CPX"                       { OP(cpx) }
"CPY"                       { OP(cpy) }
"DEC"                       { OP(dec) }
"DEX"                       { OP(dex) }
"DEY"                       { OP(dey) }
"INC"                       { OP(inc) }
"INX"                       { OP(inx) }
"INY"                       { OP(iny) }
"ASL"                       { OP(asl) }
"ROL"                       { OP(rol) }
"LSR"                       { OP(lsr) }
"ROR"                       { OP(ror) }
"LDA"                       { OP(lda) }
"STA"                       { OP(sta) }
"LDX"                       { OP(ldx) }
"STX"                       { OP(stx) }
"LDY"                       { OP(ldy) }
"STY"                       { OP(sty) }
"TAX"                       { OP(tax) }
"TXA"                       { OP(txa) }
"TAY"                       { OP(tay) }
"TYA"                       { OP(tya) }
"TSX"                       { OP(tsx) }
"TXS"                       { OP(txs) }
"PLA"                       { OP(pla) }
"PHA"                       { OP(pha) }
"PLP"                       { OP(plp) }
"PHP"                       { OP(php) }
"BPL"                       { OP(bpl) }
"BMI"                       { OP(bmi) }
"BVC"                       { OP(bvc) }
"BVS"                       { OP(bvs) }
"BCC"                       { OP(bcc) }
"BCS"                       { OP(bcs) }
"BNE"                       { OP(bne) }
"BEQ"                       { OP(beq) }
"BRK"                       { OP(brk) }
"RTI"                       { OP(rti) }
"JSR"                       { OP(jsr) }
"RTS"                       { OP(rts) }
"JMP"                       { OP(jmp) }
"BIT"                       { OP(bit) }
"CLC"                       { OP(clc) }
"SEC"                       { OP(sec) }
"CLD"                       { OP(cld) }
"SED"                       { OP(sed) }
"CLI"                       { OP(cli) }
"SEI"                       { OP(sei) }
"CLV"                       { OP(clv) }
"NOP"                       { OP(nop) }
"SLO"                       { OP_ILLEGAL(slo) }
"RLA"                       { OP_ILLEGAL(rla) }
"SRE"                       { OP_ILLEGAL(sre) }
"RRA"                       { OP_ILLEGAL(rra) }
"SAX"                       { OP_ILLEGAL(sax) }
"LAX"                       { OP_ILLEGAL(lax) }
"DCP"                       { OP_ILLEGAL(dcp) }
"ISC"                       { OP_ILLEGAL(isc) }
"ANC"                       { OP_ILLEGAL(anc) }
"ANC2"                      { OP_ILLEGAL(anc2) }
"ALR"                       { OP_ILLEGAL(alr) }
"ARR"                       { OP_ILLEGAL(arr) }
"XAA"                       { OP_ILLEGAL(xaa) }
"AXS"                       { OP_ILLEGAL(axs) }
"USBC"                      { OP_ILLEGAL(usbc) }
"AHX"                       { OP_ILLEGAL(ahx) }
"SHY"                       { OP_ILLEGAL(shy) }
"SHX"                       { OP_ILLEGAL(shx) }
"TAS"                       { OP_ILLEGAL(tas) }
"LAS"                       { OP_ILLEGAL(las) }
"BRA"                       { OP_65C02(bra) }
"PHX"                       { OP_65C02(phx) }
"PHY"                       { OP_65C02(phy) }
"PLX"                       { OP_65C02(plx) }
"PLY"                       { OP_65C02(ply) }
"STZ"                       { OP_65C02(stz) }
"TRB"                       { OP_65C02(trb) }
"TSB"                       { OP_65C02(tsb) }
"STP"                       { OP_65C02(stp) }
"WAI"                       { OP_65C02(wai) }
"BBR"[0-7]                  { OP_65C02_REL(bbr) }
"BBS"[0-7]                  { OP_65C02_REL(bbs) }
"RMB"[0-7]                  { OP_65C02_REL(rmb) }
"SMB"[0-7]                  { OP_65C02_REL(smb) }

[\xD\xA]|[\xA]|[\xD]        { P_TOK("EOL") SV_TOK return EOL; }
".IFDEF"[ \t]+{SYMB}        {
                                P_TOK("IF_DEF")
                                SV_TOK
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
                                P_TOK("IF_DEF")
                                SV_TOK
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
                                P_TOK("IF")
                                SV_TOK
                                int last_if = 1; 
                                ifdef_stack->push(ifdef_stack->instance, &last_if); 
                            }
<IF_DEF>".ELSE"             {
                                P_TOK("ELSE")
                                SV_TOK
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
                                P_TOK("ENDIF")
                                SV_TOK
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
                                P_TOK("ENDIF")
                                SV_TOK
                                if (ifdef_stack->instance->index < 0) return ENDIF;

                                int last_if = * (int*) ifdef_stack->pop(ifdef_stack->instance);
                                if (last_if != 0) 
                                {
                                    return ENDIF; 
                                }
                            }
".IF"                       { 
                                P_TOK("IF")
                                SV_TOK 
                                int last_if = 1; ifdef_stack->push(ifdef_stack->instance, &last_if); return IF; 
                            }
".ELSE"                     {
                                P_TOK("ELSE")
                                SV_TOK
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
".BYTE"                     { P_TOK("BYTE") SV_TOK return BYTE; }
".DB"                       { P_TOK("BYTE") SV_TOK return BYTE; }
".WORD"                     { P_TOK("WORD") SV_TOK return WORD; }
".DW"                       { P_TOK("WORD") SV_TOK return WORD; }
".DCW"                      { P_TOK("WORD") SV_TOK return WORD; }
".DS"                       { P_TOK("DS")   SV_TOK return DS;   }
".EQU"                      { P_TOK("EQU")  SV_TOK return EQU;  }
"NOT"                       { P_TOK("NOT")  SV_TOK return NOT;  }
">="                        { P_TOK("GE")   SV_TOK return GE;   }
"<="                        { P_TOK("LE")   SV_TOK return LE;   }
"=="                        { P_TOK("EQ")   SV_TOK return EQ;   }
"!="                        { P_TOK("NE")   SV_TOK return NE;   }
"<>"                        { P_TOK("NE")  SV_TOK return NE;   }
"<<"                        { P_TOK("SHIFT_LEFT")  SV_TOK return SHIFT_LEFT;  }
">>"                        { P_TOK("SHIFT_RIGHT") SV_TOK return SHIFT_RIGHT; }
{LOGOR}                     { P_TOK("OR")          SV_TOK return OR; }
{BITOR}                     { P_TOK("BIT_OR") SV_TOK return BIT_OR; }
{LOGAND}                    { P_TOK("AND") SV_TOK return AND; }
{BITAND}                    { P_TOK("BIT_AND") SV_TOK return BIT_AND; }
".REPEAT"                   { P_TOK("REPEAT") SV_TOK return REPEAT; }
".UNTIL"                    { P_TOK("UNTIL") SV_TOK return UNTIL; }
".END"                      { P_TOK("END") SV_TOK return END;  }
".PRINT"                    { P_TOK("PRINT") SV_TOK return PRINT; }
"\?"                        { P_TOK("PRINT") SV_TOK return PRINT; }
".PRINTALL"                 { P_TOK("PRINTALL") SV_TOK return PRINTALL; }
"\?\?"                      { P_TOK("PRINTALL") SV_TOK return PRINTALL; }
".FOR"                      { P_TOK("FOR") SV_TOK return FOR; }
".NEXT"                     { P_TOK("NEXT") SV_TOK return NEXT; }
".STEP"                     { P_TOK("STEP") SV_TOK return STEP; }
".TO"                       { P_TOK("TO") SV_TOK return TO; }
".DOWNTO"                   { P_TOK("DOWNTO") SV_TOK return DOWNTO; }
".STR"						{ P_TOK("STR") SV_TOK return STR; }
".TEXT"						{ P_TOK("STR") SV_TOK return STR; }
".STRING"				    { P_TOK("STR") SV_TOK return STR; }
".ORG"                      { P_TOK("ORG") SV_TOK return ORG; }
".SECTION"                  { P_TOK("SECTION") SV_TOK return SECTION; }
".ENDSECTION"               { P_TOK("ENDSECTION") SV_TOK return ENDSECTION; }
".SECT"                     { P_TOK("SECTION") SV_TOK return SECTION; }
".ENDS"                     { P_TOK("ENDSECTION") SV_TOK return ENDSECTION; }
".INC"                      { P_TOK("INC") SV_TOK return INC; }
".LOAD"                     { P_TOK("LOAD") SV_TOK return LOAD; }
".FILL"                     { P_TOK("FILL") SV_TOK return FILL; }
".WHILE"                    { 
                                SV_TOK
                                if (do_count > 0)
                                {
                                    P_TOK("ENDDO")
                                    --do_count; 
                                    return ENDDO;
                                }
                                P_TOK("WHILE")
                                return WHILE;
                            }
".WEND"                     { P_TOK("WEND") SV_TOK return WEND; }
".DO"                       { P_TOK("DO") SV_TOK ++do_count; return DO; }
".MACRO"                    { P_TOK("MACRO") SV_TOK in_macro_definition++; return MACRO; }
".MAC"                      { P_TOK("MACRO") SV_TOK in_macro_definition++; return MACRO; }
".ENDMACRO"                 { P_TOK("ENDMACRO") SV_TOK in_macro_definition--; return ENDMACRO; }
".ENDM"                     { P_TOK("ENDMACRO") SV_TOK in_macro_definition--; return ENDMACRO; }
".REGX"                     { P_TOK("REGX") SV_TOK return REGX; }
".REGY"                     { P_TOK("REGY") SV_TOK return REGY; }
".VAR"                      { P_TOK("VAR") SV_TOK return VAR;  }
".6502"[ \t]*"ON"           { SV_TOK cpu_mode = cpu_6502; }
".65C02"[ \t]*"ON"          { SV_TOK cpu_mode = cpu_65C02; }
".ILLEGAL"[ \t]*"ON"        { SV_TOK allow_illegal_op_codes = true; }
".ILLEGAL"[ \t]*"OFF"       { SV_TOK allow_illegal_op_codes = false; } 
".WARN"[ \t]*"ON"           { SV_TOK ignore_warnings = false; }
".WARN"[ \t]*"OFF"          { SV_TOK ignore_warnings = true; }
".C64"                      { SV_TOK output_file_format = c64; }
".PRINT"[ \t]"ON"           { P_TOK("PRINTON") SV_TOK return PRINTON; }
".PRINT"[ \t]"OFF"          { P_TOK("PRINTOFF") SV_TOK return PRINTOFF; }
^"-"                        { P_TOK("LABEL") SV_TOK yylval.strValue = (char*) STRDUP(yytext); return LABEL; }
^"+"                        { P_TOK("LABEL") SV_TOK yylval.strValue = (char*) STRDUP(yytext); return LABEL; }
[\\][1-9]+[0-9]*            {
                                P_TOK("SYMBOL")
                                SV_TOK
                                if (in_macro_definition == 0)
                                    REJECT;
                                yylval.strValue = (char*)STRDUP(yytext); 
                                return SYMBOL;
                            }
[\+|\-]+                    {
                                SV_TOK
                                if (strlen(yytext) > 1)
                                {
                                    P_TOK("SYMBOL")
                                    yylval.strValue = (char*)STRDUP(yytext); 
                                    return SYMBOL;
                                }
                                else
                                {
                                   P_C_TOK(*yytext)
                                   return *yytext;
                                }
                            }
[\'].[\']                   { SV_TOK INT_1BYTE }
[\'][\\].[\']               { SV_TOK INT_1BYTE }
[\"].[\"]                   { SV_TOK INT_1BYTE }
[\"][\\].[\'"]              { SV_TOK INT_1BYTE }
$[0-9A-Fa-f]*               { SV_TOK INT(1, 16)}
[\'][\\]x[0-9A-Fa-f]+[\']   { SV_TOK INT(3, 16)}
[0-9]*                      { SV_TOK INT(0, 10)}
%[0-1]*                     { SV_TOK INT(1, 2) }
0b[0-1]*                    { SV_TOK INT(2, 2) }
[\'][\\][0-7]{3}[\']        { SV_TOK INT(2, 8) }

{STRVALUE}					{ P_TOK("STRING_LITERAL") SV_TOK yylval.strValue = (char*)STRDUP(yytext); return STRING_LITERAL; }
"*"[ \t]*[=]                { P_TOK("PCASSIGN") SV_TOK unput('='); return PCASSIGN; }
"*"[ \t]*".EQU"             { P_TOK("PCASSIGN") SV_TOK unput('='); return PCASSIGN; }
"X"                         { P_TOK("X") SV_TOK return 'X'; }
"Y"                         { P_TOK("Y") SV_TOK return 'Y'; }
{SYMB}:?                    {                
                                auto len =  last_token == nullptr ? 0 : (int)strlen(last_token);                               
                                bool is_label = (last_token == nullptr || *last_token == '\n');                                
                                bool is_macro_name = len > 3 && 
                                last_token[0] == '.' && 
                                    (last_token[1] == 'm' || last_token[1] == 'M') &&
                                    (last_token[2] == 'a' || last_token[2] == 'A') &&
                                    (last_token[3] == 'c' || last_token[3] == 'C')
                                    ;
                                                                        
                                SV_TOK
                                yylval.strValue = (char*)STRDUP(yytext); 
                                symbol_table_ptr sym = look_up_symbol(yylval.strValue);
                                if (sym != NULL)
                                {
                                    if (sym->is_macro_name)
                                    {
                                        P_TOK("MACSYMBOL")
                                        return MACSYMBOL;
                                    }
                                }
                                if (is_label)
                                {
                                    if (sym != nullptr)
                                        sym->is_label = true;
                                    P_TOK("LABEL")
                                    return LABEL;
                                }
                                if (is_macro_name)
                                {
                                    if (sym != nullptr)
                                        sym->is_macro_name = true;
                                    P_TOK("MACSYMBOL")
                                    return MACSYMBOL;
                                }
                                P_TOK("SYMBOL")
                                return SYMBOL;
                            }
"/*"                        { SV_TOK BEGIN(C_COMMENT); }
<C_COMMENT>"*/"             { SV_TOK BEGIN(INITIAL); }
<C_COMMENT>.|[\n]           { /* ignore comments */ }
;[^\n]*                     { /* ignore comments */ }
[/][/][^\n]*                { /* ignore comments */ }

"~"                         { P_C_TOK('~') SV_TOK return '~'; }
"^"                         { P_C_TOK('^') SV_TOK return '^'; }
[-<>=+*/#,();=\n]           { P_C_TOK(*yytext) SV_TOK return *yytext; }
[ \t]+                      { /* ignore white space */ }
<<EOF>>                     {
                                if (last_token != nullptr) FREE(last_token); last_token = nullptr;
                                
                                yylineno++;
                                if (final_pass) generate_list_node(NULL);
                                
                                yypop_buffer_state();
                                if (file_stack->instance->index >= 0)
                                {

                                    const file_line_stack_entry* file_line = (file_line_stack_entry*) file_stack->pop(file_stack->instance);
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
