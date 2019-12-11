#ifndef CLASSES_HPP
#define CLASSES_HPP

#include <string>
#include <vector>
#include <stack>

using namespace std;

vector<string> TYPES = {"VOID", "INT", "BYTE", "BOOL", "STRING"};

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
};
static stack<SymbolTable*> tablesStack;
static stack<int> offsetsStack;
static vector<Enum> enums;

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
        //erorrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
    }
};

class Type: public Node{
    Type(Node* type):Node(type->value){};
}

;class ExpList: public Node{

};class Call: public Node{

};class Statement: public Node{

};class Statements: public Node{

};class Enumerator: public Node{

};class EnumeratorList: public Node{

};class FormalDecl: public Node{

};class FormalsList: public Node{

};class Formals: public Node{

}
;class RetType: public Node{
public:
    RetType(Node* type):Node(type->value){};
}
;class EnumDecl: public Node{

};class Enums: public Node{

};

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
        const vector<string> temp = {"STRING", "VOID"};
        Entry* print = new Entry("print", temp, offsetsStack.top());
        offsetsStack.top()++;
        const vector<string> temp2 = {"INT", "VOID"};
        Entry* printi = new Entry("printi", temp2, offsetsStack.top());
        offsetsStack.top()++;
        global->lines.push_back(print);
        global->lines.push_back(printi);
        tablesStack.push(global);
        offsetsStack.push(0);
    }
};



//static void init(){
//    SymbolTable* global = new SymbolTable;
//    offsetsStack.push(0);
//    tablesStack.push(global);
//}
//
//static Node* enumerator(Node* id){
//    SymbolTable* global = new SymbolTable;
//    offsetsStack.push(0);
//    tablesStack.push(global);
//}
//
//static Node* enumDecel(){
//    SymbolTable* global = new SymbolTable;
//    offsetsStack.push(0);
//    tablesStack.push(global);
//}

//#define YYSTYPE yystype
#endif //CLASSES_HPP
