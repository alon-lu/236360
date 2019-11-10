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
    if (j != 0) {
        if ((i >= '0' && i <= '7')) {
            int first = i - '0';
            int second = -1;
            if (j >= '0' && j <= '9') {
                second = j - '0';
            } else if ((j >= 'a' && j <= 'f')) {
                second = j - 'a' + 10;
            } else if ((j >= 'A' && j <= 'F')) {
                second = j - 'A' + 10;
            }
            return first * 16 + second;
        }
    }
    return -1;
}

int checkChar(char* ch,bool location) {
    if (*ch == '\n') {
        printf("Error unclosed string\n");
        exit(0);
    }
    if (*ch == '\r') {
        printf("Error unclosed string\n");
        exit(0);
    }
    if (*ch == '\\' && location == true) {
        printf("Error unclosed string\n");
        exit(0);
    }
    if (*ch == '\"') {
        printf("Error %c\n", *ch);
        exit(0);
    }
}
void showToken(const int token) {
    if (token == WRONGCHAR) {
        printf("Error %s\n", yytext);
        exit(0);
    }
    if (token == APOST) {
        printf("Error unclosed string\n");
        exit(0);
    }
    if (token == WRONGSTRING) {
        int strlen = stringLen(yytext);
        for (size_t i = 0; i < strlen; i++) {
            if (yytext[i] == '\n' || yytext[i] == '\r' || (yytext[i] == '\\' && i == strlen - 2)) {
                printf("Error unclosed string\n");
                exit(0);
            }
            if (yytext[i] == '\\') {
                printf("Error undefined escape sequence %c\n", yytext[i + 1]);
                exit(0);
            }
            if (yytext[i] == '\"') {
                printf("Error %c\n", yytext[i]);
                exit(0);
            }
        }
    }
    if (token == COMMENT) {
        printf("%d %s %s\n", yylineno, FRUIT_STRING[token], "//");
    } else if (token == STRING) {
        int strlen = stringLen(yytext);
        std::string outputString;
        for (int i = 0; i < strlen; i++) {
            //checkChar(&yytext[i], i == strlen - 2);
            if (yytext[i] != '\\') {
                outputString.push_back(yytext[i]);
            } else {
                int insert = toInsert(yytext[i + 1]);
                if (insert == 0) {
                    printf("Error undefined escape sequence %c\n", yytext[i + 1]);
                    exit(0);
                }
                if (insert == 'x') {
                    insert = fromAscii(yytext[i + 2], yytext[i + 3]);
                    if (insert == -1) {
                        printf("Error undefined escape sequence x");
                        if (yytext[i + 3] == 0) {
                            std::cout << yytext[i + 2] << std::endl;
                        } else {
                            std::cout << yytext[i + 2] << yytext[i + 3] << std::endl;
                        }
                        exit(0);
                    }
                    i += 2;
                }
                outputString.push_back(insert);
                i++;
            }
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

/*
int main() {

    char char1=-1;
    char char2=255;

    printf("Signed char : %d\n",char1);
    printf("Unsigned char : %d\n",char2);

}
 */
