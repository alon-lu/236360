#include "tokens.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

enum operations
{
    PLUS=-1,
    MINUS =-2,
    DIV=-3,
    MUL=-4
};

int content(int token){
    if (token==BINOP) {
        switch (*yytext) {
            case '+':
                return PLUS;
            case '-':
                return MINUS;
            case '/':
                return DIV;
            case '*':
                return MUL;
        }
    }
    ///token is a number
    int retVal=0;
    int number=0;
    for (int i = 0; yytext[i]!=0; ++i) {
        number= yytext[i]-'0'; //getting the number from the ascii value
        retVal=retVal*10+number;
    }
    return retVal;
}





int main() {
    int token;
    bool wasBinop = true;
    std::vector<int> tokens;
    std::vector<int> text;
    while (token = yylex()) {
        if (token == NUM || token == BINOP) {
            tokens.push_back(token);
            text.push_back(content(token));
        } else if (token == WRONGCHAR) {
            printf("Error %s\n", yytext);
            exit(0);
        } else {
            printf("Error: %s\n", FRUIT_STRING[token]);
            exit(0);
        }
    }
    while (tokens.size() > 1 && wasBinop == true) {
        wasBinop = false;
        for (int i = tokens.size() - 1; i >= 0; i--) {
            if (tokens[i] == BINOP) {
                wasBinop = true;
                if (tokens[i + 1] == NUM && tokens[i + 2] == NUM) {
                    int oper1 = text[i + 1], oper2 = text[i + 2], result = 0;
                    switch (text[i]) {
                        case PLUS:
                            result = oper1 + oper2;
                            break;
                        case MINUS:
                            result = oper1 - oper2;
                            break;
                        case DIV:
                            result = oper1 / oper2;
                            break;
                        case MUL:
                            result = oper1 * oper2;
                            break;
                        default:
                            printf("NOOOOOOOOO");
                            exit(0);
                    }
                    text[i + 2] = result;
                    tokens.erase(tokens.begin() + i, tokens.begin() + (i + 2));
                    text.erase(text.begin() + i, text.begin() + (i + 2));
                    break;
                } else {
                    printf("Error: Bad Expression\n");
                    exit(0);
                }
            }
        }
    }
    if (tokens[0] != NUM || wasBinop == false) {
        printf("Error: Bad Expression\n");
        exit(0);
    }

    printf("%d\n", text[0]);
    return 0;
}
