//
// Created by Liad on 18/12/2019.
//
#include "classes.hpp"


/// global variables and functions
string currFucn;
vector<shared_ptr<SymbolTable>> tablesStack;
vector<int> offsetsStack;
vector<shared_ptr<EnumTable>> enumsStack;//will hold all the enums that were defined
vector<string> TYPES = {"VOID", "INT", "BYTE", "BOOL", "STRING"};

void endFunc() {
    currFucn = "";
}

int loopCount = 0;

void inLoop() {
    loopCount++;
}

void outLoop() {
    loopCount--;
}

void openScope() {
    auto newScope = shared_ptr<SymbolTable>(new SymbolTable);
    tablesStack.emplace_back(newScope);
    auto newEnumScope = shared_ptr<EnumTable>(new EnumTable);
    enumsStack.emplace_back(newEnumScope);
    offsetsStack.push_back(offsetsStack.back());
}

void closeScope() {
    output::endScope();
    auto scope = tablesStack.back();
    for (auto i:scope->lines) {//printing all the variables(not enumDef) and functions
        if (i->types.size() == 1) {
            output::printID(i->name, i->offset, i->types[0]);//variables
        } else {
            auto retVal = i->types.back();
            i->types.pop_back();
            if (i->types.front() == "VOID") {
                i->types.pop_back();
            }

            output::printID(i->name, i->offset, output::makeFunctionType(retVal,
                                                                         i->types));//functions
        }
    }
    auto enumScope = enumsStack.back();
    for (int j = 0; j < enumScope->enumLines.size(); ++j) {
        output::printEnumType(enumScope->enumLines[j]->name, enumScope->enumLines[j]->values);
    }
    while (scope->lines.size() != 0) {
        scope->lines.pop_back();
    }
    tablesStack.pop_back();
    offsetsStack.pop_back();
    while (enumScope->enumLines.size() != 0) {
        enumScope->enumLines.pop_back();
    }
    enumsStack.pop_back();
}

void endProgram() {
    auto global = tablesStack.front()->lines;
    bool mainFound = false;
    for (int i = 0; i < global.size(); ++i) {
        if (global[i]->name == "main") {
            if (global[i]->types.size() == 2) {
                if (global[i]->types[0] == "VOID" && global[i]->types[1] == "VOID") {
                    mainFound = true;
                }
            }
        }
    }
    if (!mainFound) {//no main function
        output::errorMainMissing();
        exit(0);
    }
    closeScope();
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


void enterArguments(Formals *fm) {
//    for (int i = fm->formals.size() - 1; i >= 0; i--) {
//        auto temp = shared_ptr<Entry>(new Entry(fm->formals[i]->value, fm->formals[i]->type,
//                                                0 - (fm->formals.size() - i)));
    for (int i =0; i < fm->formals.size(); i++) {
        auto temp = shared_ptr<Entry>(new Entry(fm->formals[i]->value, fm->formals[i]->type,-i-1));
        tablesStack.back()->lines.push_back(temp);
    }
}
///EXP implamtation

Exp::Exp(Node *terminal, string str) : Node(terminal->value) {
    this->type = "";
    if (str.compare("num") == 0) {
        type = "INT";
    }
    if (str.compare("STRING") == 0) {
        type = "STRING";
    }
    if (str.compare("BOOL") == 0) {
        type = "BOOL";
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
        type = "BYTE";
    }
}

Exp::Exp(Node *Not, Exp *exp) {
    this->type = "";
    if (exp->type != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    this->type = "BOOL";
    boolVal = !boolVal;
}

Exp::Exp(Exp *left, Node *op, Exp *right, string str) {
    this->type = "";
    if ((left->type.compare("BYTE") == 0 ||
         left->type.compare("INT") == 0) &&
        (right->type.compare("BYTE") == 0 ||
         right->type.compare("INT") == 0)) {// both operands must be numbers

//            int ileft = stoi(left->value), iright = stoi(right->value);
        if (str.compare("RELOPL") == 0 || str.compare("RELOPN") == 0) {
            this->type = "BOOL";
//                if (op->value.compare("==") == 0) {
//                    boolVal = (ileft == iright ? true : false);
//                } else if (op->value.compare("!=") == 0) {
//                    boolVal = (ileft != iright ? true : false);
//                } else if (op->value.compare("<") == 0) {
//                    boolVal = (ileft < iright ? true : false);
//                } else if (op->value.compare(">") == 0) {
//                    boolVal = (ileft > iright ? true : false);
//                } else if (op->value.compare("<=") == 0) {
//                    boolVal = (ileft <= iright ? true : false);
//                } else if (op->value.compare(">=") == 0) {
//                    boolVal = (ileft >= iright ? true : false);
//                }
        }
        if (str.compare("ADD") == 0 || str.compare("MUL") == 0) {
            this->type = "BYTE";
            if (left->type == "INT" || right->type == "INT") {
                this->type = "INT";
            }
//                if (op->value.compare("+") == 0) {
//                    value = to_string(ileft + iright);
//                } else if (op->value.compare("-") == 0) {
//                    value = to_string(ileft - iright);
//                } else if (op->value.compare("*") == 0) {
//                    value = to_string(ileft * iright);
//                } else if (op->value.compare("/") == 0) {
//                    value = to_string(ileft / iright);
//                }
        }
    } else if ((left->type.compare("BOOL") == 0 &&
                right->type.compare("BOOL") == 0)) {//both are bool
        //handiling AND OR
        bool bleft = true, bright = true;
        this->type = "BOOL";
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
        } else {
            output::errorMismatch(yylineno);
            exit(0);
        }
    } else {
        output::errorMismatch(yylineno);
        exit(0);
    }
}

Exp::Exp(Exp *exp) {
    value = exp->value;
    type = exp->type;
    boolVal = exp->boolVal;
}

Exp::Exp(Type *type, Exp *exp) {//cant see type because it is announced later
    this->type = "";
    if (exp->type.compare(0, 5, "enum ") == 0) {//exp type is enum
        if (type->value == "INT") {//casting into int
            value = exp->value;
            this->type = "INT";
        }
    }
}

Exp::Exp(Node *ID) {
    this->type = "";
    for (int i = tablesStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j < tablesStack[i]->lines.size(); ++j) {
            if (tablesStack[i]->lines[j]->name == ID->value) {
                this->value = ID->value;
                this->type = tablesStack[i]->lines[j]->types.back();
                return;
            }
        }
    }
    for (int i = enumsStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
            for (int k = 0; k < enumsStack[i]->enumLines[j]->values.size(); ++k) {
                if (enumsStack[i]->enumLines[j]->values[k] == ID->value) {
                    this->value = ID->value;
                    string XX = "enum";
                    this->type = XX + " " + enumsStack[i]->enumLines[j]->name;
                    return;
                }
            }
        }
    }
    output::errorUndef(yylineno, ID->value);
    exit(0);
}

Exp::Exp(Call *call) {
    this->type = call->value;
}

Exp::Exp(Exp *exp, string str) {//checking for if
    if (exp->type != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    value = exp->value;
    type = exp->type;
    boolVal = exp->boolVal;
}


EnumType::EnumType(Node *Enum, Node *id) {
    for (int i = enumsStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
            if (enumsStack[i]->enumLines[j]->name == id->value) {
                value = Enum->value + " " + id->value;
                return;
            }
        }
    }
    output::errorUndefEnum(yylineno, id->value);
    exit(0);
}

Call::Call(Node *ID, ExpList *list) {
    auto global = tablesStack.front()->lines;
    for (auto i:global) {
        if (i->name == ID->value) {// id found
            if (i->types.size() == 1) {
                output::errorUndefFunc(yylineno, ID->value);
                exit(0);
            }
            if (i->types.size() == 1 + list->expList.size()) {//checking the number of arguments
                for (int j = 0; j < list->expList.size(); j++) {
                    if (list->expList[j].type == "BYTE" && i->types[j] == "INT") {
                        continue;
                    }
                    if (list->expList[j].type != i->types[j]) {
                        i->types.pop_back();
                        output::errorPrototypeMismatch(yylineno, i->name, i->types);
                        exit(0);
                    }
                }
                this->value = i->types.back();
                return;//if we got here without errors, we found our function
            } else {
                i->types.pop_back();
                output::errorPrototypeMismatch(yylineno, i->name, i->types);
                exit(0);
            }
        }
    }
    output::errorUndefFunc(yylineno, ID->value);//if we found our function we're not supposed to get here
    exit(0);
}

Call::Call(Node *ID) {
    auto global = tablesStack.front()->lines;
    for (auto i:global) {
        if (i->name == ID->value) {
            if (i->types.size() == 1) {
                output::errorUndefFunc(yylineno, ID->value);
                exit(0);
            }
            if (i->types.size() == 2) {
                this->value = i->types.back();
                return;//if we got here without errors, we found our function
            } else {
                vector<string> temp = {""};
                output::errorPrototypeMismatch(yylineno, i->name, temp);
                exit(0);
            }
        }
    }
    output::errorUndefFunc(yylineno, ID->value);//if we found our function we're not supposed to get here
    exit(0);
}


bool FormalDecl::checkingTypes(string str) {
    for (int i = enumsStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
            int len = str.length();
            if (str.compare(5, len - 5,
                            enumsStack[i]->enumLines[j]->name) == 0) {
                return true;
            }
        }
    }
    return false;
}

EnumDecl::EnumDecl(Node *id, EnumeratorList *lst) {
    if (identifierExists(id->value)) {
        output::errorDef(yylineno, id->value);
        exit(0);
    }
    for (int i = 0; i < lst->enumerators.size(); ++i) {
        if (identifierExists(lst->enumerators[i]) || lst->enumerators[i] == id->value) {
            output::errorDef(yylineno, lst->enumerators[i]);
            exit(0);
        }
        for (int j = i + 1; j < lst->enumerators.size(); ++j) {
            if (lst->enumerators[i] == lst->enumerators[j]) {
                output::errorDef(yylineno, lst->enumerators[i]);
                exit(0);
            }
        }
    }
    this->value = id->value;
    enumerators = vector<string>(lst->enumerators);
    auto enumline = shared_ptr<Enum>(new Enum(id->value, lst->enumerators));
    enumsStack.back()->enumLines.emplace_back(enumline);
}

FuncDecl::FuncDecl(RetType *retType, Node *ID, Formals *args) {
    if (identifierExists(ID->value)) {
        output::errorDef(yylineno, ID->value);
        exit(0);
    }
    for (int i = 0; i < args->formals.size(); ++i) {
        if (identifierExists(args->formals[i]->value) || args->formals[i]->value == ID->value) {
            output::errorDef(yylineno, args->formals[i]->value);
            exit(0);
        }
        for (int j = i + 1; j < args->formals.size(); ++j) {
            if (args->formals[i]->value == args->formals[j]->value) {
                output::errorDef(yylineno, args->formals[i]->value);
                exit(0);
            }
        }
    }
    value = ID->value;
    if (args->formals.size() != 0) {
        for (int i = 0; i < args->formals.size(); i++) {
            types.push_back(args->formals[i]->type);
        }
    } else {
        types.emplace_back("VOID");
    }
    types.emplace_back(retType->value);

    auto temp = shared_ptr<Entry>(new Entry(this->value, this->types, 0));
    tablesStack.back()->lines.push_back(temp);
    currFucn = ID->value;
}

///Statemant implamtation

Statement::Statement(Type *type, Node *id) {
//checking if the id already defined
    if (identifierExists(id->value)) {
        output::errorDef(yylineno, id->value);
        exit(0);
    }
    int offset = offsetsStack.back()++;
    auto temp = shared_ptr<Entry>(new Entry(id->value, type->value, offset));
    tablesStack.back()->lines.emplace_back(temp);
    data = "type id";
}

Statement::Statement(EnumType *enumType, Node *id) {
    bool enumID_Found = false;
    for (int i = enumsStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
            int len = enumType->value.length();
            if (enumType->value.compare(5, len - 5,
                                        enumsStack[i]->enumLines[j]->name) == 0) {
                enumID_Found = true;
                break;
            }
        }
    }
    if (enumID_Found == false) {
        output::errorUndefEnum(yylineno, enumType->value.substr(5,
                                                                enumType->value.size() -
                                                                5));
        exit(0);
    }
    if (identifierExists(id->value)) {
        output::errorDef(yylineno, id->value);
        exit(0);
    }
    data = "enumType id";
    int offset = offsetsStack.back()++;
    auto temp = shared_ptr<Entry>(new Entry(id->value, enumType->value, offset));
    tablesStack.back()->lines.emplace_back(temp);
}

Statement::Statement(Type *type, Node *id, Exp *exp) {
    //checking if the id already defined
    if (exp->type != type->value) {
        if (type->value != "INT" || exp->type != "BYTE") {
            output::errorMismatch(yylineno);
            exit(0);
        }
    }
    if (identifierExists(id->value)) {
        output::errorDef(yylineno, id->value);
        exit(0);
    }
    data = exp->value;
    int offset = offsetsStack.back()++;
    auto temp = shared_ptr<Entry>(new Entry(id->value, type->value, offset));
    tablesStack.back()->lines.emplace_back(temp);
}

Statement::Statement(EnumType *enumType, Node *id, Exp *exp) {
    bool flag = false;
    vector<string> enumVals;
    for (int i = enumsStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j < enumsStack[i]->enumLines.size(); ++j) {
            int len = enumType->value.length();
            if (enumType->value.compare(5, len - 5,
                                        enumsStack[i]->enumLines[j]->name) ==
                0) {
                flag = true;
                enumVals = enumsStack[i]->enumLines[j]->values;
                break;
            }
        }
    }

    if (flag == false) {
        output::errorUndefEnum(yylineno, enumType->value.substr(5,
                                                                enumType->value.size() -
                                                                5));
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
        output::errorUndefEnumValue(yylineno, id->value);
        exit(0);
    }

    int offset = offsetsStack.back()++;
    auto temp = shared_ptr<Entry>(new Entry(id->value, enumType->value, offset));
    tablesStack.back()->lines.emplace_back(temp);
}

Statement::Statement(Node *id, Exp *exp) {
    for (int i = tablesStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j <
                        tablesStack[i]->lines.size(); ++j) {//finding in symbol table
            if (tablesStack[i]->lines[j]->name == id->value) {
                if (tablesStack[i]->lines[j]->types.size() ==
                    1) {//making sure this is not a function
                    if ((tablesStack[i]->lines[j]->types[0] == "INT" && exp->type== "BYTE") ||
                    tablesStack[i]->lines[j]->types[0] == exp->type) {//checking types
                        data = exp->value;
                        return;
                    } else {
                        if (tablesStack[i]->lines[j]->types[0].compare(0, 4, "enum") == 0) {
                            output::errorUndefEnumValue(yylineno, id->value);
                            exit(0);
                        }
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
    output::errorUndef(yylineno,
                       id->value);//id not found in the symbol table
    exit(0);
}

Statement::Statement(string retType) {
    for (int i = tablesStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j <
                        tablesStack[i]->lines.size(); ++j) {//finding in symbol table
            if (tablesStack[i]->lines[j]->name == currFucn) {
                int size = tablesStack[i]->lines[j]->types.size();
                if (tablesStack[i]->lines[j]->types[size - 1] ==
                    retType) {//checking types
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

Statement::Statement(Exp *exp) {
    if (exp->type == "VOID") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    string retType = exp->type;
    for (int i = tablesStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j <
                        tablesStack[i]->lines.size(); ++j) {//finding in symbol table
            if (tablesStack[i]->lines[j]->name == currFucn) {
                int size = tablesStack[i]->lines[j]->types.size();
                if (tablesStack[i]->lines[j]->types[size - 1] ==
                    retType) {//checking types
                    data = exp->value;
                    return;
                } else if (retType == "BYTE" && tablesStack[i]->lines[j]->types[size - 1] == "INT") {
                    data = exp->value; //allowing the case of retType to be byte in case it was int
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

Statement::Statement(Node *word) {
    if (loopCount == 0) {
        if (word->value == "break") {
            output::errorUnexpectedBreak(yylineno);
            exit(0);
        }
        output::errorUnexpectedContinue(yylineno);
        exit(0);
    }
    data = "this was a break or continue";
}

Program::Program() {
    shared_ptr<SymbolTable> global = shared_ptr<SymbolTable>(new SymbolTable);
    auto globalEnum = shared_ptr<EnumTable>(new EnumTable());
    const vector<string> temp = {"STRING", "VOID"};
    auto print = shared_ptr<Entry>(new Entry("print", temp, 0));
    const vector<string> temp2 = {"INT", "VOID"};//to do arg of byte also
    auto printi = shared_ptr<Entry>(new Entry("printi", temp2, 0));
    global->lines.emplace_back(print);
    global->lines.emplace_back(printi);
    tablesStack.emplace_back(global);
    enumsStack.emplace_back(globalEnum);
    offsetsStack.emplace_back(0);
}


