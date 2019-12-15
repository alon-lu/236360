#ifndef CLASSES_HPP
#define CLASSES_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include "hw3_output.hpp"

extern int yylineno;
using namespace std;
string currFucn;

//Single entry for symbol table
class Entry {
public:
    string name;
    vector<string> types;
    int offset;

    Entry(string name, vector<string> types, int offset) {
        name.assign(name);
        types = types;
        offset = offset;
    }

    Entry(string name, string types, int offset) {
        name.assign(name);
        this->types.emplace_back(types);
        offset = offset;
    }
};

class SymbolTable {
public:
    vector<Entry *> lines;//scope table
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
    vector<Enum *> enumLines;//scope table
    EnumTable() = default;
};


static vector<SymbolTable *> tablesStack;
static vector<int> offsetsStack;
static vector<EnumTable *> enumsStack;//will hold all the enums that were defined
static vector<string> TYPES = {"void", "int", "byte", "bool", "string"};

void openScope() {
    auto newScope = new SymbolTable;
    tablesStack.emplace_back(newScope);
    auto newEnumScope = new EnumTable;
    enumsStack.emplace_back(newEnumScope);
    offsetsStack.push_back(offsetsStack.back());
}

void closeScope() {
    output::endScope();
    SymbolTable *scope = tablesStack.back();
    for (auto i:scope->lines) {//printing all the variables(not enumDef) and functions
        if (i->types.size() == 1) {
            output::printID(i->name, i->offset, i->types[0]);//variables
        } else {
            auto retVal = i->types.back();
            i->types.pop_back();
            output::printID(i->name, i->offset, output::makeFunctionType(retVal, i->types));//functions
        }
    }
    EnumTable *enumScope = enumsStack.back();
    for (auto i:enumScope->enumLines) {
        output::printEnumType(i->name, i->values);
    }

    while (scope->lines.size() != 0) {
        Entry *s = scope->lines.back();
        delete (s);
        scope->lines.pop_back();
    }
    delete (scope);
    tablesStack.pop_back();

    while (enumScope->enumLines.size() != 0) {
        Enum *s = enumScope->enumLines.back();
        delete (s);
        enumScope->enumLines.pop_back();
    }
    delete (enumScope);
    enumsStack.pop_back();
}


bool identifierExists(string str) {
    for (int i = tablesStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j < tablesStack[i]->lines.size(); ++j) {
            if (tablesStack[i]->lines[j]->name == str)
                return true;
        }
    }
    for (int i = enumsStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
            if (enumsStack[i]->enumLines[j]->name == str) {
                return true;
            }
            for (auto k:enumsStack[i]->enumLines[j]->values) {
                if (k == str) {
                    return true;
                }
            }
        }
    }
    return false;
}

class Node {
public:
    string value;

    Node(string str) {
        value.assign(str);
    }

    Node() {
        value = "";
    }
};

class Type : public Node {
    Type(Node *type) : Node(type->value) {};
};

class Exp : public Node {
public:
    string type;
    bool boolVal;

    Exp(Node *terminal, string str) : Node(terminal->value) {
        if (str.compare("num") == 0) {
            type = "int";
        }
        if (str.compare("string") == 0) {
            type = "string";
        }
        if (str.compare("bool") == 0) {
            type = "bool";
            if (value.compare("true") == 0) {
                boolVal = true;
            } else {
                boolVal = false;
            }
        }
        if (str.compare("B") == 0) {
            if (stoi(terminal->value) > 255) {
                output::errorByteTooLarge(yylineno, terminal->value);
                exit(0);
            }
            type = "byte";
        }
    };

    Exp(Node *Not, Exp *exp) {
        if (exp->type != "bool") {
            output::errorMismatch(yylineno);
            exit(0);
        }
        boolVal = !boolVal;
    }

    ///handels RELOP,MUL,ADD,OR,AND
    Exp(Exp *left, Node *op, Exp *right, string str) {
        if ((left->type.compare("byte") == 0 || left->type.compare("int") == 0) &&
            (right->type.compare("byte") == 0 || right->type.compare("int") == 0)) {// both operands must be numbers

            int ileft = stoi(left->value), iright = stoi(right->value);//handiling RELOP,mul and add
            if (str.compare("RELOPL") == 0 || str.compare("RELOPN") == 0) {
                if (op->value.compare("==") == 0) {
                    boolVal = (ileft == iright ? true : false);
                } else if (op->value.compare("!=") == 0) {
                    boolVal = (ileft != iright ? true : false);
                } else if (op->value.compare("<") == 0) {
                    boolVal = (ileft < iright ? true : false);
                } else if (op->value.compare(">") == 0) {
                    boolVal = (ileft > iright ? true : false);
                } else if (op->value.compare("<=") == 0) {
                    boolVal = (ileft <= iright ? true : false);
                } else if (op->value.compare(">=") == 0) {
                    boolVal = (ileft >= iright ? true : false);
                }
            }
            if (str.compare("ADD") == 0 || str.compare("MUL") == 0) {
                if (op->value.compare("+") == 0) {
                    value = to_string(ileft + iright);
                } else if (op->value.compare("-") == 0) {
                    value = to_string(ileft - iright);
                } else if (op->value.compare("*") == 0) {
                    value = to_string(ileft * iright);
                } else if (op->value.compare("/") == 0) {
                    value = to_string(ileft / iright);
                }
            }
        } else if ((left->type.compare("bool") == 0 && right->type.compare("bool") == 0)) {//both are bool
            //handiling AND OR
            bool bleft = true, bright = true;
            if (left->value.compare("false") == 0)
                bleft = false;
            if (right->value.compare("false") == 0)
                bright = false;

            if (str.compare("AND") == 0 || str.compare("OR") == 0) {
                if (op->value.compare("AND") == 0) {
                    if (bleft && bright)
                        value = "true";
                    else value = "false";
                } else if (op->value.compare("OR") == 0) {
                    if (bleft || bright)
                        value = "true";
                    else value = "false";
                }
            }
        } else {
            output::errorMismatch(yylineno);
            exit(0);
        }
    }

    ///check if the string is lost or not
    Exp(Exp *exp) {
        value = exp->value;
        type = exp->type;
        boolVal = exp->boolVal;
    }

    Exp(Type *type, Exp *exp) {//cant see type because it is announced later
        if (exp->type.compare(0, 5, "enum ")) {//exp type is enum
            if (type->value == "int") {//casting into int
                value = exp->value;
                this->type = "int";
            }
        }
    }

    Exp(Node *ID) {
        for (int i = tablesStack.size() - 1; i >= 0; i--) {
            for (int j = 0; j < tablesStack[i]->lines.size(); ++j) {
                if (tablesStack[i]->lines[j]->name == ID->value) {
                    this->value = ID->value;
                    return;
                }
            }
        }
        output::errorUndef(yylineno, ID->value);
        exit(0);
    }

};

class EnumType : public Node {
    EnumType(Node *Enum, Node *id) {

        for (int i = enumsStack.size() - 1; i >= 0; i--) {
            for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
                if (enumsStack[i]->enumLines[j]->name == id->value) {
                    value = Enum->value + id->value;
                    return;
                }
            }
        }
        output::errorUndefEnum(yylineno, id->value);
    }


};

class ExpList : public Node {
public:
    vector<Exp> expList;

    ExpList(Exp *exp) {
        expList.emplace_back(exp);
    }

    ExpList(ExpList *expList, Exp *exp) {
        this->expList = vector<Exp>(expList->expList);
        this->expList.emplace_back(exp);
    }
};

class Call : public Node {


};

class Enumerator : public Node {
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
        if (checkingTypes(type) == false) {
            std::cout << "NOT SUPPOSE TO HAPPENED!!" << std::endl;
        }
    }

    FormalDecl(EnumType *t, Node *id) : Node(id->value), type(t->value) {
        if (checkingTypes(type) == false) {
            output::errorUndefEnum(yylineno, type);
        }
    }

    bool checkingTypes(string str) {
        for (int i = enumsStack.size() - 1; i >= 0; i--) {
            for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
                int len = enumsStack[i]->enumLines[j]->name.length();
                if (str.compare(5, len - 5, enumsStack[i]->enumLines[j]->name) == 0) {
                    return true;
                }
            }
        }
        return false;
    }


};

class FormalsList : public Node {
public:
    vector<FormalDecl *> formals;

    //The value of FormalsList is empty!!!
    FormalsList(FormalDecl *formal) {
        formals.emplace_back(formal);
    }

    FormalsList(FormalsList *flist, FormalDecl *formal) {
        formals = vector<FormalDecl *>(flist->formals);
        formals.emplace_back(formal);
    }
};

class Formals : public Node {
    vector<FormalDecl *> formals;

    Formals(FormalsList *flist) {
        formals = vector<FormalDecl *>(flist->formals);
    }

};

class RetType : public Node {
public:
    RetType(Node *type) : Node(type->value) {};
};

class EnumDecl : public Node {
public:
    vector<string> enumerators;

    EnumDecl(Node *id, EnumeratorList *lst) {
        if (identifierExists(id->value)) {
            output::errorDef(yylineno, id->value);
            exit(0);
        }
        this->value = id->value;
        enumerators = vector<string>(lst->enumerators);
        auto enumline = new Enum(id->value, lst->enumerators);
        enumsStack.back()->enumLines.emplace_back(enumline);
    }

};

//class Enums: public Node{
//
//};

class FuncDecl : public Node {
//public:
//    vector<string> types;
//
//    FuncDecl(RetType* retType, Node* ID, Formals* args){
//        value = ID->value;
//        types = args.types;
//        types.emplace_back(retType->value);
//    }
};
class Statements;

class Statement : public Node {
public:
    string data;

    Statement(Type *type, Node *id) {
        //checking if the id already defined

        if (identifierExists(id->value)) {
            output::errorDef(yylineno, id->value);
            exit(0);
        }
        int offset = *offsetsStack.end()++;
        auto temp = new Entry(id->value, type->value, offset);
        tablesStack.back()->lines.emplace_back(temp);
        data="type id";
    }

    Statement(EnumType *enumType, Node *id) {
        bool flag = false;
        for (int i = enumsStack.size() - 1; i >= 0; i--) {
            for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
                int len = enumsStack[i]->enumLines[j]->name.length();
                if (id->value.compare(5, len - 5, enumsStack[i]->enumLines[j]->name) == 0) {
                    flag = true;
                    break;
                }
            }
        }
        if (flag == false) {
            output::errorUndefEnum(yylineno, enumType->value.substr(5, enumType->value.size() - 5));
            exit(0);
        }
        if (identifierExists(id->value)) {
            output::errorDef(yylineno, id->value);
            exit(0);
        }
        data="enumType id";
        int offset = *offsetsStack.end()++;
        auto temp = new Entry(id->value, enumType->value, offset);
        tablesStack.back()->lines.emplace_back(temp);
    }

    Statement(Type *type, Node *id, Exp *exp) {
        //checking if the id already defined
        if (exp->type != type->value) {
            if (type->value != "int" || exp->type != "byte") {
                output::errorMismatch(yylineno);
            }
        }
        if (identifierExists(id->value)) {
            output::errorDef(yylineno, id->value);
            exit(0);
        }
        data = exp->value;
        int offset = *offsetsStack.end()++;
        auto temp = new Entry(id->value, type->value, offset);
        tablesStack.back()->lines.emplace_back(temp);
    }


    Statement(EnumType *enumType, Node *id, Exp *exp) {
        bool flag = false;
        vector<string> enumVals;
        for (int i = enumsStack.size() - 1; i >= 0; i--) {
            for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
                int len = enumsStack[i]->enumLines[j]->name.length();
                if (enumType->value.compare(5, len - 5, enumsStack[i]->enumLines[j]->name) == 0) {
                    flag = true;
                    enumVals = enumsStack[i]->enumLines[j]->values;
                    break;
                }
            }
        }

        if (flag == false) {
            output::errorUndefEnum(yylineno, enumType->value.substr(5, enumType->value.size() - 5));
            exit(0);
        }
        if (identifierExists(id->value)) {
            output::errorDef(yylineno, id->value);
            exit(0);
        }
        data = "null";
        for (int k = 0; k < enumVals.size(); ++k) {
            if (enumVals[k] == exp->value)
                data = to_string(k);
            break;
        }

        if (data == "null") {
            for (int i = enumsStack.size() - 1; i >= 0; i--) {
                for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
                    for (auto k:enumsStack[i]->enumLines[j]->values) {
                        if (k == exp->value) {
                            output::errorUndefEnumValue(yylineno, id->value);
                            exit(0);
                        }
                    }
                }
            }
            output::errorUndef(yylineno, exp->value);
            exit(0);
        }

        int offset = *offsetsStack.end()++;
        auto temp = new Entry(id->value, enumType->value, offset);
        tablesStack.back()->lines.emplace_back(temp);
    }

    Statement(EnumDecl *enumDecl) {
        value = "was enumdecl";
    }

    Statement(Node *id, Exp *exp) {
        for (int i = tablesStack.size() - 1; i >= 0; i--) {
            for (int j = 0; j < tablesStack[i]->lines.size(); ++j) {//finding in symbol table
                if (tablesStack[i]->lines[j]->name == id->value) {
                    if (tablesStack[i]->lines[j]->types.size() == 1) {//making sure this is not a function
                        if (tablesStack[i]->lines[j]->types[0] == exp->type) {//checking types
                            data = exp->value;
                            return;
                        } else {
                            output::errorMismatch(yylineno);
                            exit(0);
                        }
                    } else {
                        output::errorUndef(yylineno, id->value);
                        exit(0);
                    }
                }
            }
        }
        output::errorUndef(yylineno, id->value);//id not found in the symbol table
        exit(0);
    }

    Statement(string retType) {
        for (int i = tablesStack.size() - 1; i >= 0; i--) {
            for (int j = 0; j < tablesStack[i]->lines.size(); ++j) {//finding in symbol table
                if (tablesStack[i]->lines[j]->name == currFucn) {
                    int size = tablesStack[i]->lines[j]->types.size();
                    if (tablesStack[i]->lines[j]->types[size - 1] == retType) {//checking types
                        data = "ret val of void";
                        return;
                    } else {
                        output::errorMismatch(yylineno);
                        exit(0);
                    }
                }
            }
        }
        output::errorUndef(yylineno, "this is crazy");//should no reach this
        exit(0);
    }

    Statement(Exp *exp) {
        string retType = exp->type;
        for (int i = tablesStack.size() - 1; i >= 0; i--) {
            for (int j = 0; j < tablesStack[i]->lines.size(); ++j) {//finding in symbol table
                if (tablesStack[i]->lines[j]->name == currFucn) {
                    int size = tablesStack[i]->lines[j]->types.size();
                    if (tablesStack[i]->lines[j]->types[size - 1] == retType) {//checking types
                        data = exp->value;
                        return;
                    } else {
                        output::errorMismatch(yylineno);
                        exit(0);
                    }
                }
            }
        }
        output::errorUndef(yylineno, "this is crazy2");//should no reach this
        exit(0);
    }

    Statement(Statements* st){
        data="this was a block";
    }
};


class Statements : public Node {

};

class Funcs : public Node {

};


class Program : public Node {
public:

    Program() {
        SymbolTable *global = new SymbolTable();
        const vector<string> temp = {"string", "void"};
        auto print = new Entry("print", temp, offsetsStack.back());
        *(offsetsStack.end())++;
        const vector<string> temp2 = {"int", "void"};//to do arg of byte also
        auto printi = new Entry("printi", temp2, offsetsStack.back());
        *(offsetsStack.end())++;
        global->lines.emplace_back(print);
        global->lines.emplace_back(printi);
        tablesStack.emplace_back(global);
        offsetsStack.emplace_back(0);
    }
};




//#define YYSTYPE yystype
#endif //CLASSES_HPP
