#include "tokens.hpp"
#include <stdio.h>
#include <vector>

enum operations
{
	PLUS=-1,
	MINUS =-2,
	DIV=-3,
	MUL=-4
}

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

void showToken(const int token,	std::vector<int> tokens, std::vector<int> text)
{
		if (token != NUM || token != BINOP) {
        printf("Error: %s\n", FRUIT_STRING[token]);
        exit(0);
    }
		tokens.push_back(token);
		text.push_back(content(token));
}



int main()
{
	int token;
	std::vector<int> tokens;
	std::vector<int> text;
	while(token = yylex()) {
		showToken(token, tokens, text);
	}
	while(tokens.size > 1){
		for (int i = tokens.size -1; i >=0; i--) {
			if(tokens[i] == BINOP){
				if(tokens[i+1] == NUM && tokens[i+2] == NUM){
					int oper1 = text[i+1], oper2 = text[i+2];, result = 0;
					switch (text[i]){
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
					}
					text[i+2] = result;
					tokens.erase(tokens[i], tokens[i+1]);
					text.erase(text[i], text[i+1]);
					break;
				}
				else{
					printf("Error: Bad Expression\n");
					exit(0);
				}
			}

		}
	}
	if(tokens[0] != NUM){
		printf("Error: Bad Expression\n");
		exit(0);
	}

	printf("%s\n", text[0]);
	return 0;
}
