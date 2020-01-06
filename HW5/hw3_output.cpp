#include <iostream>
#include "hw3_output.hpp"
#include <sstream>

using namespace std;


// const string output::rules[] = {
// 	"Program -> Enums Funcs",
// 	"Funcs -> epsilon",
// 	"Funcs -> FuncDecl Funcs",
// 	"FuncDecl -> RetType ID LPAREN Formals RPAREN LBRACE Statements RBRACE",
// 	"Enums -> epsilon",
// 	"Enums -> EnumDecl Enums",
// 	"EnumDecl -> ENUM ID LBRACE EnumeratorList RBRACE SC",
// 	"RetType -> Type",
// 	"RetType ->  VOID",
// 	"Formals -> epsilon",
// 	"Formals -> FormalsList",
// 	"FormalsList -> FormalDecl",
// 	"FormalsList -> FormalDecl COMMA FormalsList",
// 	"FormalDecl -> Type ID",
// 	"FormalDecl -> EnumType ID",
// 	"EnumeratorList -> Enumerator",
// 	"EnumeratorList -> EnumeratorList COMMA Enumerator",
// 	"Enumerator -> ID",
// 	"Statements -> Statement",
// 	"Statements -> Statements Statement",
// 	"Statement -> LBRACE Statements RBRACE",
// 	"Statement -> Type ID SC",
// 	"Statement -> EnumType ID SC",
// 	"Statement -> EnumDecl",
// 	"Statement -> Type ID ASSIGN Exp SC",
// 	"Statement -> EnumType ID ASSIGN Exp SC",
// 	"Statement -> ID ASSIGN Exp SC",
// 	"Statement -> Call SC",
// 	"Statement -> RETURN SC",
// 	"Statement -> RETURN Exp SC",
// 	"Statement -> IF LPAREN Exp RPAREN Statement",
// 	"Statement -> IF LPAREN Exp RPAREN Statement ELSE Statement",
// 	"Statement -> WHILE LPAREN Exp RPAREN Statement",
// 	"Statement -> BREAK SC",
// 	"Statement -> CONTINUE SC",
// 	"Call -> ID LPAREN ExpList RPAREN",
// 	"Call -> ID LPAREN RPAREN",
// 	"ExpList -> Exp",
// 	"ExpList -> Exp COMMA ExpList",
// 	"Type -> INT",
// 	"Type -> BYTE",
// 	"Type -> BOOL",
// 	"EnumType -> ENUM ID",
// 	"Exp -> LPAREN Exp RPAREN",
// 	"Exp -> Exp BINOP Exp",
// 	"Exp -> ID",
// 	"Exp -> Call",
// 	"Exp -> NUM",
// 	"Exp -> NUM B",
// 	"Exp -> STRING",
// 	"Exp -> TRUE",
// 	"Exp -> FALSE",
// 	"Exp -> NOT Exp",
// 	"Exp -> Exp AND Exp",
// 	"Exp -> Exp OR Exp",
// 	"Exp -> Exp RELOP Exp",
// 	"Exp -> LPAREN Type RPAREN Exp"
// };

void output::endScope(){
    cout << "---end scope---" << endl;
}

void output::printID(const string& id, int offset, const string& type) {
    cout << id << " " << type <<  " " << offset <<  endl;
}

string typeListToString(const std::vector<string>& argTypes) {
	stringstream res;
	res << "(";
	for(int i = 0; i < argTypes.size(); ++i) {
		res << argTypes[i];
		if (i + 1 < argTypes.size())
			res << ",";
	}
	res << ")";
	return res.str();
}

string valueListsToString(const std::vector<string>& values) {
	stringstream res;
	res << "{";
	for(int i = 0; i < values.size(); ++i) {
		res << values[i];
		if (i + 1 < values.size())
			res << ",";
	}
	res << "}";
	return res.str();
}

string output::makeFunctionType(const string& retType, std::vector<string>& argTypes)
{
	stringstream res;
	res << typeListToString(argTypes) << "->" << retType;
	return res.str();
}

void output::printEnumType(const string& name, std::vector<string>& values)
{
	cout << "enum " << name << " " << valueListsToString(values) <<  endl;
}


void output::errorLex(int lineno){
    cout << "line " << lineno << ":" << " lexical error" << endl;
}

void output::errorSyn(int lineno){
    cout << "line " << lineno << ":" << " syntax error" << endl;
}

void output::errorUndef(int lineno, const string& id){
    cout << "line " << lineno << ":" << " variable " << id << " is not defined" << endl;
}

void output::errorDef(int lineno, const string& id){
    cout << "line " << lineno << ":" << " identifier " << id << " is already defined" << endl;
}

void output::errorUndefFunc(int lineno, const string& id) {
    cout << "line " << lineno << ":" << " function " << id << " is not defined" << endl;
}

void output::errorUndefEnum(int lineno, const string& id) {
    cout << "line " << lineno << ":" << " enum " << id << " is not defined" << endl;
}

void output::errorUndefEnumValue(int lineno, const string& id) {
    cout << "line " << lineno << ":" << " assignment of undefined value to " << id << endl;
}

void output::errorMismatch(int lineno){
    cout << "line " << lineno << ":" << " type mismatch" << endl;
}

void output::errorPrototypeMismatch(int lineno, const string& id, std::vector<string>& argTypes) {
    cout << "line " << lineno << ": prototype mismatch, function " << id << " expects arguments " << typeListToString(argTypes) << endl;
}

void output::errorUnexpectedBreak(int lineno) {
	cout << "line " << lineno << ":" << " unexpected break statement" << endl;
}

void output::errorUnexpectedContinue(int lineno) {
	cout << "line " << lineno << ":" << " unexpected continue statement" << endl;
}

void output::errorMainMissing() {
	cout << "Program has no 'void main()' function" << endl;
}

void output::errorByteTooLarge(int lineno, const string& value) {
	cout << "line " << lineno << ": byte value " << value << " out of range"<< endl;
}
