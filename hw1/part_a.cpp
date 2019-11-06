#include <stdio.h>
#include <vector>
#include "tokens.hpp"
extern int yylex();
unsigned int stringLen(const char* str){
    int i=0;
    for (;str[i]!=0;i++);
    return i;
}

char toInsert(char next) {
    switch (next) {
        case 'n':
						yylineno++ ;
            return '\n';
        case 'r':
						yylineno++ ;
            return '\r';
        case 't':
            return '\t';
        case '\\':
            return '\\';
        case '"':
            return '"';
    }
}

void showToken(const int token) {
    if (token == COMMENT) {
        printf("%d %s %s\n", yylineno, FRUIT_STRING[token], "//");
    } else
		if (token == STRING) {
        int strlen = stringLen(yytext);
        std::vector<char> outputString;
        for (int i = 1; i < strlen -1 ; i++) {
            if (yytext[i] != '\\')
                outputString.push_back(yytext[i]);
            else {
                char next = yytext[i + 1];
								outputString.push_back(toInsert(next));
								i++;
            }
        }
				printf("%d %s %s\n", yylineno, FRUIT_STRING[token], outputString);
    } else {
        printf("%d %s %s\n", yylineno, FRUIT_STRING[token], yytext);
    }
}

int main(){
    int token;
    while(token = yylex()) {
        showToken(token);
    }
    return 0;
}
