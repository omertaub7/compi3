%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hw3_output.hpp"
#include "Node.hpp"
#include "parser.tab.hpp"


%}

%option yylineno
%option noyywrap

WHITESPACE 			([\t\n\r ])


%%
void                        	{return VOID;}
int                         	{return INT;}
byte                      	  	{return BYTE;}
b                           	{return B;}
bool                        	{return BOOL;}
and                         	{return AND;}
or                          	{return OR;}
not                         	{return NOT;}
true                        	{return TRUE;}
false                       	{return FALSE;}
return                      	{return RETURN;}
if                          	{return IF;}
else                        	{return ELSE;}
while                       	{return WHILE;}
break                       	{return BREAK;}
continue                    	{return CONTINUE;}
;                           	{return SC;}
,                           	{return COMMA;}
\(                          	{return LPAREN;}
\)                          	{return RPAREN;}
\{                          	{return LBRACE;}
\}                          	{return RBRACE;}
=                           	{return ASSIGN;}
(==|!=)                         {yylval = new Relop(yytext); return EQUALOP;}
(<|>|<=|>=)                    	{yylval = new Relop(yytext); return RELOP;}
\+                              {yylval = new Binop(yytext); return PLUS;}
-                               {yylval = new Binop(yytext); return MINUS;}
\*                              {yylval = new Binop(yytext); return MUL;}
\/                         	 	{yylval = new Binop(yytext); return DIV;}
[a-zA-Z][a-zA-Z0-9]*        	{yylval = new Id(yytext); return ID;}
0|[1-9][0-9]*               	{
                                    yylval = new Num(yytext);
                                    return NUM;
                                }
\"([^\n\r\"\\]|\\[rnt"\\])+\" 	{yylval = new String(yytext); return STRING;}
[\t\n\r ]                   	; 
\/\/[^\r\n]*[\r|\n|\r\n]?     	;
.                           	{output::errorLex(yylineno); exit(0);};


%%
/*C Funcs*/

