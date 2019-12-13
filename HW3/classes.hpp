#ifndef CLASSES_HPP
#define CLASSES_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include "hw3_output.hpp"
extern int yylineno;
using namespace std;



//Single entry for symbol table
class Entry{
public:
    string name;
    vector<string> types;
    int offset;

    Entry(string name, vector<string> types, int offset){
        name.assign(name);
        types = types;
        offset = offset;
    }
};
class SymbolTable{
public:
    vector<Entry*> lines;//scope table
    SymbolTable()= default;
};
class Enum{
public:
    string name;
    vector<string> values;

    Enum(string n, vector<string> vec):name(n), values(vec){};
};
static stack<SymbolTable*> tablesStack;
static stack<int> offsetsStack;
static vector<Enum> enums;
static vector<string> TYPES = {"void", "int", "byte", "bool", "string"};

class Node{
public:
    string value;
    Node(string str){
        value.assign(str);
    }
    Node(){
        value = "";
    }
};


class Exp: public Node{

};

class EnumType: public Node{
    EnumType(Node* Enum, Node* id){
        for(auto i:enums){
            if(i.name == id->value){
                value = Enum->value + id->value;
                return;
            }
        }
        output::errorUndefEnum(yylineno, id->value);
    }
};

class Type: public Node{
    Type(Node* type):Node(type->value){};
}

;class ExpList: public Node{

};class Call: public Node{

};class Statement: public Node{

};class Statements: public Node{

};
class Enumerator: public Node{
    Enumerator(Node* id):Node(id->value){};
};

class EnumeratorList: public Node{
public:
    vector<string> enumerators;
    //The value of EnumeratorList is empty!!!
    EnumeratorList(Enumerator* enumerator){
        enumerators.push_back(enumerator->value);
    }

    EnumeratorList(EnumeratorList* elist, Enumerator* enumerator){
        enumerators = vector<string> (elist->enumerators);
        enumerators.push_back(enumerator->value);
    }
};


class FormalDecl: public Node{
public:
    string type;
    //Node->value is the ID

    FormalDecl(Type* t, Node* id):Node(id->value), type(t->value){
        if(checkingTypes(type) == false){
            std::cout<<"NOT SUPPOSE TO HAPPENED!!"<<std::endl;
        }
    }
    FormalDecl(EnumType* t, Node* id):Node(id->value), type(t->value){
        if(checkingTypes(type) == false){
            output::errorUndefEnum(yylineno,type);
        }
    }
    bool checkingTypes(string str){
        for(auto i: TYPES){
            if(i.compare(str) == 0){
                return true;
            }
        }
        return false;
    }
};
class FormalsList: public Node{
public:
    vector<FormalDecl*> formals;
    //The value of FormalsList is empty!!!
    FormalsList(FormalDecl* formal){
        formals.push_back(formal);
    }

    FormalsList(FormalsList* flist, FormalDecl* formal){
        formals = vector<FormalDecl*> (flist->formals);
        formals.push_back(formal);
    }
};

class Formals: public Node{

};

class RetType: public Node{
public:
    RetType(Node* type):Node(type->value){};
};

class EnumDecl: public Node{
public:
    vector<string> enumerators;

    EnumDecl(Node* id, EnumeratorList* lst):Node(id->value), enumerators(lst->enumerators){
        TYPES.push_back("enum "+id->value);
        enums.push_back(Enum(id->value, lst->enumerators));
    }

};

//class Enums: public Node{
//
//};

class FuncDecl: public Node{
//public:
//    vector<string> types;
//
//    FuncDecl(RetType* retType, Node* ID, Formals* args){
//        value = ID->value;
//        types = args.types;
//        types.push_back(retType->value);
//    }
};

class Funcs: public Node{

};

class Program : public Node{
public:

    Program(){
        SymbolTable* global = new SymbolTable();
        const vector<string> temp = {"string", "void"};
        Entry* print = new Entry("print", temp, offsetsStack.top());
        offsetsStack.top()++;
        const vector<string> temp2 = {"int", "void"};//to do arg of byte also
        Entry* printi = new Entry("printi", temp2, offsetsStack.top());
        offsetsStack.top()++;
        global->lines.push_back(print);
        global->lines.push_back(printi);
        tablesStack.push(global);
        offsetsStack.push(0);
    }
};




//#define YYSTYPE yystype
#endif //CLASSES_HPP
