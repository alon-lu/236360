%{

/* Declarations section */
#include <stdio.h>
#define YYSTYPE int
#include "parser.tab.hpp"
#include "output.hpp"
%}

%option yylineno
%option noyywrap
digit   		  ([1-9])
letter  		  ([a-zA-Z])
whitespace	  ([\t\n\r ])
%x STRINGS
%%

void                                  return VOID;
int                                   return INT;
byte                                  return BYTE;
b                                     return B;
bool                                  return BOOL;
enum                                  return ENUM;
and                                   return AND;
or                                    return OR;
not                                   return NOT;
true                                  return TRUE;
false                                 return FALSE;
return                                return RETURN;
if                                    return IF;
else                                  return ELSE;
while                                 return WHILE;
break                                 return BREAK;
continue                              return CONTINUE;
(\;)                                  return SC;
(\,)                                  return COMMA;
(\()                                  return LPAREN;
(\))                                  return RPAREN;
(\{)                                  return LBRACE;
(\})                                  return RBRACE;
(=)                                   return ASSIGN;
(==)|(!=)                      			  return RELOPL;
(<)|(>)|(<=)|(>=)                     return RELOPN;
\+|\-                                 return ADD;
\*|\/							                    return MUL;
[a-zA-Z][a-zA-Z0-9]*                  return ID;
({digit}({digit}|0)*)|0               return NUM;
(\")                                  BEGIN(STRINGS);
<STRINGS><<EOF>>                      {output::errorLex(yylineno); exit(0);};
<STRINGS>([\x00-\x09\x0b-\x0c\x0e-\x21\x23-\x5b\x5d-\x7f]|((\\)(\\))|((\\)(\"))|((\\)(n))|((\\)(r)))*(\") {BEGIN(INITIAL);return STRING;}
<STRINGS>([^(\")])*((\")?)            {output::errorLex(yylineno); exit(0);};
{whitespace}				                  ;
\/\/[^\r\n]*[\r|\n|\r\n]?             ;
.                                     {output::errorLex(yylineno); exit(0);};
%%
