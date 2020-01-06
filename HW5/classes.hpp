#ifndef CLASSES_HPP
#define CLASSES_HPP

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include "hw3_output.hpp"
#include "regPool.hpp"
#include "bp.hpp"


extern int yylineno;
using namespace std;

void endFunc();


//Single entry for symbol table
class Entry {
public:
    string name;
    vector<string> types;
    int offset;

    Entry(string name, vector<string> types, int offset) {
        this->name = name;
        this->types = types;
        this->offset = offset;
    }

    Entry(string name, string types, int offset) {
        this->name = name;
        this->types.emplace_back(types);
        this->offset = offset;
    }
};

class SymbolTable {
public:
    vector<shared_ptr<Entry>> lines;//scope table
    SymbolTable() = default;
};

class Enum {
public:
    string name;//name "enum ID"
    vector<string> values;//will hold the string that are the enums, ordered

    Enum(string n, vector<string> vec) : name(n), values(vec) {};
};

class EnumTable {
public:
    vector<shared_ptr<Enum>> enumLines;//scope table
    EnumTable() = default;
};




void inLoop();
void outLoop();

void openScope();

void closeScope();

void endProgram();

bool identifierExists(string str);


class Node {
public:
    string value;

    Node(string str) {
        if (str == "void") {
            value = "VOID";
        } else if (str == "bool") {
            value = "BOOL";
        } else if (str == "int") {
            value = "INT";
        } else if (str == "byte") {
            value = "BYTE";
        } else
            value = str;
    }

    Node() {
        value = "";
    }

    virtual ~Node() {};
};

#define YYSTYPE Node*


class Type : public Node {
public:
    Type(Node *type) : Node(type->value) {};
};

class Call;//declaring class Call son of node

class Exp : public Node {
public:
    string type;
    bool boolVal;
    string reg;

    // handles NUM,B,string,true,false
    Exp(Node *terminal, string str);

    Exp(Node *Not, Exp *exp);

    ///handels RELOP,MUL,ADD,OR,AND
    Exp(Exp *left, Node *op, Exp *right, string str);

    ///check if the string is lost or not
    Exp(Exp *exp);

// LPAREN Type RPAREN Exp
    Exp(Type *type, Exp *exp);

//  ID
    Exp(Node *ID);

    Exp(Call *call);

    Exp(Exp *exp, string str);
};


class EnumType : public Node {
public:
    EnumType(Node *Enum, Node *id);

};

class ExpList : public Node {
public:
    vector<Exp> expList;///save in order in text(not derivation)

    ExpList(Exp *exp) {
//        expList.emplace_back(exp);
expList.emplace(expList.begin(),exp);
    }

    ExpList(Exp *exp, ExpList *expList) {
        this->expList = vector<Exp>(expList->expList);
//        this->expList.emplace_back(exp);
        this->expList.emplace(this->expList.begin(),exp);

    }
};

class Call : public Node {
public:
    Call(Node *ID, ExpList *list);
    Call(Node *ID);
};


class Enumerator : public Node {
public:
    Enumerator(Node *id) : Node(id->value) {};
};

class EnumeratorList : public Node {
public:
    vector<string> enumerators;

    //The value of EnumeratorList is empty!!!
    EnumeratorList(Enumerator *enumerator) {
        enumerators.emplace_back(enumerator->value);
    }

    EnumeratorList(EnumeratorList *elist, Enumerator *enumerator) {
        enumerators = vector<string>(elist->enumerators);
        enumerators.emplace_back(enumerator->value);
    }
};


class FormalDecl : public Node {
public:
    string type;
    //Node->value is the ID

    FormalDecl(Type *t, Node *id) : Node(id->value), type(t->value) {

    }

    FormalDecl(EnumType *t, Node *id) : Node(id->value), type(t->value) {
        if (checkingTypes(type) == false) {
            output::errorUndefEnum(yylineno, type);
            exit(0);
        }
    }
    bool checkingTypes(string str);


};

class FormalsList : public Node {
public:
    vector<FormalDecl *> formals;///save in order in text(not derivation)

    //The value of FormalsList is empty!!!
    FormalsList(FormalDecl *formal) {
        formals.insert(formals.begin(),formal);
//        formals.emplace_back(formal);
    }

    FormalsList(FormalsList *flist, FormalDecl *formal) {
        formals = vector<FormalDecl *>(flist->formals);
        formals.insert(formals.begin(),formal);
    }
};

class Formals : public Node {
public:
    vector<FormalDecl *> formals;

    Formals() {};

    Formals(FormalsList *flist) {
        this->formals = vector<FormalDecl *>(flist->formals);
    }

};

void enterArguments(Formals *fm);

class RetType : public Node {
public:
    RetType(Node *type) : Node(type->value) {};
};

class EnumDecl : public Node {
public:
    vector<string> enumerators;

    EnumDecl(Node *id, EnumeratorList *lst);

};

class Enums : public Node {
public:
    Enums() {};
};

class FuncDecl : public Node {
public:
    vector<string> types;
    FuncDecl(RetType *retType, Node *ID, Formals *args);

};

class Statements;

class Statement : public Node {
public:
    string data;

//      Type ID SC
    Statement(Type *type, Node *id);

//      EnumType ID SC
    Statement(EnumType *enumType, Node *id);

//      Type ID ASSIGN Exp SC
    Statement(Type *type, Node *id, Exp *exp);

//      EnumType ID ASSIGN Exp SC
    Statement(EnumType *enumType, Node *id, Exp *exp);

//    EnumDecl
    Statement(EnumDecl *enumDecl){
        value = "was enumdecl";
    }

//    ID ASSIGN Exp SC
    Statement(Node *id, Exp *exp);

//      RETURN SC; from a fucntion with a retVal of void

    Statement(string retType);
//    RETURN Exp SC
    Statement(Exp *exp);

//    LBRACE Statements RBRACE
    Statement(Statements *sts) {
        data = "this was a block";
    }

    // handels if, if else, while
    Statement(string str, Exp *exp) {
        if (exp->type != "BOOL") {
            output::errorMismatch(yylineno);
            exit(0);
        }
        data = "this was an if/ if else /while";
    }


// handling break and continue
    Statement(Node *word);


    Statement(Call *call) {
        data = "this was a call for a function";
    };

};


class Statements : public Node {
public:
    Statements(Statement *st) {};

    Statements(Statements *sts, Statement *st) {};
};

class Funcs : public Node {
public:
    Funcs() {};
};


class Program : public Node {
public:
    Program();
};




//#define YYSTYPE yystype
#endif //CLASSES_HPP
