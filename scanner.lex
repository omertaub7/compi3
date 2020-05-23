%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "output.hpp"
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
(==|!=)                         {return EQUALOP;}
(<|>|<=|>=)                    	{return RELOP;}
\+                              {return PLUS;}
-                               {return MINUS;}
\*                              {return MUL;}
\/                         	 	{return DIV;}
[a-zA-Z][a-zA-Z0-9]*        	{return ID;}
0|[1-9][0-9]*               	{return NUM;}
\"([^\n\r\"\\]|\\[rnt"\\])+\" 	{return STRING;}
[\t\n\r ]                   	; 
\/\/[^\r\n]*[\r|\n|\r\n]?     	;
.                           	{output::errorLex(yylineno); exit(0);};


%%
/*C Funcs*/

