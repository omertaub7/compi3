%{

#include "utils.hpp"
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
(==|!=)                         {yylval = getRelOp(yytext); return EQUALOP;}
(<|>|<=|>=)                    	{yylval = getRelOp(yytext); return RELOP;}
\+                              {yylval = getBinOp(BinOp::PLUS); return PLUS;}
-                               {yylval = getBinOp(BinOp::MINUS); return MINUS;}
\*                              {yylval = getBinOp(BinOp::MUL); return MUL;}
\/                         	 	{yylval = getBinOp(BinOp::DIV); return DIV;}
[a-zA-Z][a-zA-Z0-9]*        	{yylval = getId(yytext); return ID;}
0|[1-9][0-9]*               	{yylval = getNum(yytext); return NUM;}
\"([^\n\r\"\\]|\\[rnt"\\])+\" 	{yylval = getStr(yytext); return STRING;}
[\t\n\r ]                   	; 
\/\/[^\r\n]*[\r|\n|\r\n]?     	;
.                           	{throw lexException();}


%%


