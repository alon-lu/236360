#include <stdio.h>
#include <vector>
#include <iostream>
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
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case '\\':
            return '\\';
        case '"':
            return '"';
        case 'x':
            return 'x';
        default:
            return 0;
    }
}

char fromAscii(char i,char j) {
    if ((i >= '0' && i <= '7')) {
        int first = i - '0';
        int second = 0;
        if (j >= '0' && j <= '9') {
            second = j - '0';
        }
        if ((j >= 'a' && j <= 'f')) {
            second = j - 'a' + 1;
        }
        if ((j >= 'A' && j <= 'F')) {
            second = j - 'A' + 1;
        }
        return first * 10 + second;
    }
}

void showToken(const int token) {
    if (token == WRONGCHAR) {

        printf("Error <char>\n", yytext);
        exit(0);
    }
    if (token == COMMENT) {
        printf("%d %s %s\n", yylineno, FRUIT_STRING[token], "//");
    } else if (token == STRING) {
        int strlen = stringLen(yytext);
        std::string outputString;
        for (int i = 1; i < strlen - 1; i++) {
            if (yytext[i] != '\\')
                outputString.push_back(yytext[i]);
            else {
                char insert = toInsert(yytext[i + 1]);
                if (insert == 'n') {
                    printf("Error enclosed string\n");
                    exit(0);
                }
                if (insert == 0) {
                    printf("Error undefined escape sequence q\n");
                    exit(0);
                }
                if (insert == 'x') {
                    insert = fromAscii(yytext[i + 2], yytext[i + 3]);
                }
                outputString.push_back(insert);
                i++;
            }
        }
        if (*outputString.end()=='\\'){
            printf("Error unclosed string\n");
            exit(0);
        }
        printf("%d %s ", yylineno, FRUIT_STRING[token]);
        std::cout << outputString << std::endl;
    } else {
        printf("%d %s %s\n", yylineno, FRUIT_STRING[token], yytext);
    }
}

int main() {
    int token;
    while (token = yylex()) {
        showToken(token);
    }
    return 0;
}
