%option noyywrap 
%{
#include <stdio.h>
#include "header.h" 
int linenumber;
symtab * lookup();
symtab * ptr;
void     insertID();
void 	 printSymTab();
/* You need to define for all tokens in C--, here are some examples */
#define RETURN      10
#define TYPEDEF     11
#define IF          12
#define ELSE        13
#define INT		    14
#define FLOAT 		15 
#define FOR         16
#define VOID        17
#define WHILE       18
#define INT_CONST   19
#define FLOAT_CONST 20
#define STR_CONST   21
#define ID          22
#define OP_ADD      23
#define OP_MINUS    24
#define OP_DIV      25
#define OP_MUL      26
#define OP_LT       27
#define OP_GT       28
#define OP_GEQ      29
#define OP_LEQ      30
#define OP_NEQ      31
#define OP_EQ       32
#define OP_OR 		33
#define OP_AND      34
#define OP_NOT      35
#define OP_ASSIGN 	36 
#define MK_LBRACE   37
#define MK_RBRACE   38
#define MK_LBRACK 	39
#define MK_RBRACK 	40
#define MK_LPAREN   41
#define MK_RPAREN   42
#define MK_SEMI     43
#define MK_COMMA    44
#define MK_DOT      45
#define ERROR 		100 
%}

letter   [A-Za-z]
digit    [0-9]
reserved_return   "return"
reserved_typedef  "typedef"
reserved_if       "if"
reserved_else     "else"
reserved_int      "int"
reserved_float    "float"
reserved_for      "for"
reserved_void     "void"
reserved_while    "while"
ID	 {letter}({letter}|{digit}|"_")*
WS	 [ \t]+
Int_constant [+-]?{digit}+
Float_number [+-]?({digit}+\.{digit}*|{digit}*\.{digit}+)
Float_constant ({Float_number}|{Int_constant})([Ee]{Int_constant})?
String_constant \"("\n"|\\\"|[^"])*\"
left_comment  "/*"
right_comment \*+"/"
comment     {left_comment}([^*]|(\*)+[^/*])*({right_comment})

OP_add      "+"
OP_minus    "-"
OP_div      "/"
OP_mul      "*"
OP_lt       "<"
OP_gt       ">"
OP_geq      ">="
OP_leq      "<="
OP_neq      "!="
OP_eq       "=="
OP_or       "||"
OP_and      "&&"
OP_not      "!"
OP_assign   "="

newline  "\n"

DL_lparen "("
DL_rparen ")"
DL_lbrace "{"
DL_rbrace "}"
DL_lbrack "\["
DL_rbrack "\]"
DL_comma  ","
DL_semicol ";"
DL_dot  "."

error    .

%%

{WS}                   {}
{reserved_typedef}  /* return TYPEDEF; */
{reserved_if}       /* return IF; */
{reserved_else}     /* return ELSE; */ 
{reserved_int}      /* return INT; */
{reserved_float}    /* return FLOAT; */
{reserved_for}      /* return FOR; */
{reserved_void}     /* return VOID; */
{reserved_while}    /* return WHILE; */
{comment}   {
                printf("%s\n", yytext);
            }
{ID}    { 
			ptr = lookup(yytext);
			if (ptr == NULL)
			     insertID(yytext);	
			else 
			     ptr->counter++;
		}
{Int_constant}      /* return INT_CONST; */
{Float_constant}    {ECHO; printf("\n");} /* return FLOAT_CONST; */
{String_constant}   /* return STR_CONST; */ {/*printf("read %s at %d\n", yytext, linenumber);*/}
{OP_add}        /* return OP_ADD; */
{OP_minus}      /* return OP_MINUS; */
{OP_div}        /* return OP_DIV; */
{OP_mul}        /* return OP_MUL; */
{OP_lt}         /* return OP_LT; */
{OP_gt}         /* return OP_GT; */
{OP_geq}        /* return OP_GEQ; */
{OP_leq}        /* return OP_LEQ; */
{OP_neq}        /* return OP_NEQ; */
{OP_eq}         /* return OP_EQ; */
{OP_or}         /* return OP_OR; */
{OP_and}        /* return OP_AND; */
{OP_not}        /* return OP_NOT; */
{OP_assign}     /* return OP_ASSIGN; */

{newline}       linenumber += 1;

{DL_lparen}     /* return MK_LPAREN; */
{DL_rparen}     /* return MK_RPAREN; */
{DL_lbrace}     /* return MK_LBRACE; */
{DL_rbrace}     /* return MK_RBRACE; */
{DL_lbrack}     /* return MK_LBRACK; */
{DL_rbrack}     /* return MK_RBRACE; */
{DL_dot}        {}
{DL_comma}      {}
{DL_semicol}    {}
{error}         {
                   printf("Error: unrecongnized token %s\n", yytext);
                   printf("Exiting...\n");
                   exit(1);
                }
%%

int main(int argc, char **argv)
{
 argc--; ++argv;
   if (argc > 0)
      yyin = fopen(argv[0], "r");
   else
      yyin = stdin;
   yylex();
   printSymTab();
}

