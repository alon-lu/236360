//
// Created by Liad on 18/12/2019.
//todo: 1)Tests for Numeric Slide of arithmetic exp's
//      2)where to declere the print functions(that are present in the symbol table from the start)
//      

#include "classes.hpp"


/// global variables and functions
string currFucn;
int currFuncArgs;
regPool pool;
CodeBuffer &buffer = CodeBuffer::instance();
vector<shared_ptr<SymbolTable>> tablesStack;
vector<int> offsetsStack;
vector<shared_ptr<EnumTable>> enumsStack;//will hold all the enums that were defined
vector<string> TYPES = {"VOID", "INT", "BYTE", "BOOL", "STRING"};

void endFunc() {
    currFucn = "";
    currFuncArgs=0;
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

/// gets a type(BOOL,BYTE,ENUM,INT) and returns the matching llvm type
string get_LLVM_Type(string type) {
    //the only avialble types are BOOL,BYTE,INT,ENUM
    if (type == "BOOL") {
        return "i2";
    } else if (type == "BYTE") {
        return "i8";
    } else return "i32"; //INT and ENUM fall here
}

void enterArguments(Formals *fm) {
//    for (int i = fm->formals.size() - 1; i >= 0; i--) {
//        auto temp = shared_ptr<Entry>(new Entry(fm->formals[i]->value, fm->formals[i]->type,
//                                                0 - (fm->formals.size() - i)));
    for (int i = 0; i < fm->formals.size(); i++) {
        auto temp = shared_ptr<Entry>(new Entry(fm->formals[i]->value, fm->formals[i]->type, -i - 1));
        tablesStack.back()->lines.push_back(temp);
    }
}
///EXP implamtation

Exp::Exp(Node *terminal, string str) : Node(terminal->value) {
    this->startLabel = buffer.genLabel();
    this->type = "";
    if (str.compare("num") == 0) {
        type = "INT";
        this->reg = pool.getReg();
        buffer.emit(this->reg + " = " + terminal->value);
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
    this->startLabel = buffer.genLabel();
    this->type = "";
    if (exp->type != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    this->type = "BOOL";
    this->falseList = exp->trueList;
    this->trueList = exp->falseList;
}


Exp::Exp(Exp *left, Node *op, Exp *right, string str) {
    this->startLabel = buffer.genLabel();
    this->type = "";
    if ((left->type.compare("BYTE") == 0 ||
         left->type.compare("INT") == 0) &&
        (right->type.compare("BYTE") == 0 ||
         right->type.compare("INT") == 0)) {// both operands must be numbers

        if (str.compare("RELOPL") == 0 || str.compare("RELOPN") == 0) {
            this->type = "BOOL";
            string isize = "i8";
            if (left->type == "INT" || right->type == "INT") {
                isize = "i32";
            }
            string relop;
            if (op->value.compare("==") == 0) {
                relop = "eq";
            } else if (op->value.compare("!=") == 0) {
                relop = "ne";
            } else if (op->value.compare("<") == 0) {
                relop = "slt";
                if (isize == "i8") {
                    relop = "ult";
                }
            } else if (op->value.compare(">") == 0) {
                relop = "sgt";
                if (isize == "i8") {
                    relop = "ugt";
                }
            } else if (op->value.compare("<=") == 0) {
                relop = "sle";
                if (isize == "i8") {
                    relop = "ule";
                }
            } else if (op->value.compare(">=") == 0) {
                relop = "sge";
                if (isize == "i8") {
                    relop = "uge";
                }
            }
            buffer.emit(this->reg + " = icmp " + relop + " " + isize + " " + left->reg + ", " + right->reg);
            int loc = buffer.emit("br i1 " + this->reg + ", label @, label @");
            trueList = buffer.makelist(pair<int, BranchLabelIndex>(loc, FIRST));
            falseList = buffer.makelist(pair<int, BranchLabelIndex>(loc, SECOND));
        }
        if (str.compare("ADD") == 0 || str.compare("MUL") == 0) {
            this->type = "BYTE";
            string isize = "i8";
            if (left->type == "INT" || right->type == "INT") {
                this->type = "INT";
                isize = "i32";
            }
            this->reg = pool.getReg();
            string operation;
            if (op->value.compare("+") == 0) {
                operation = "add";
            } else if (op->value.compare("-") == 0) {
                operation = "sub";
            } else if (op->value.compare("*") == 0) {
                operation = "mul";
            } else if (op->value.compare("/") == 0) {
                buffer.emit("%cond = icmp eq i32 %" + right->reg + ", 0");
                buffer.emit("br i1 %cond, lablel %zeroflag, label %dodiv");
                buffer.emit("zeroflag: ");//todo: jump to function
                buffer.emit("dodiv:");
                operation = "sdiv";
            }
            buffer.emit(this->reg + " = " + operation + " " + isize + " " + left->reg + ", " + right->reg);


        }
    } else if ((left->type.compare("BOOL") == 0 &&
                right->type.compare("BOOL") == 0)) {//both are bool
        //handiling AND OR
        this->type = "BOOL";

        if (str.compare("AND") == 0 || str.compare("OR") == 0) {
            string boolop;
            if (op->value.compare("AND") == 0) {
                buffer.bpatch(left->trueList, right->startLabel);
                this->falseList = buffer.merge(right->falseList, left->falseList);
                this->trueList = right->trueList;
            } else if (op->value.compare("OR") == 0) {
                buffer.bpatch(left->falseList, right->startLabel);
                this->trueList = buffer.merge(right->trueList, left->trueList);
                this->falseList = right->falseList;
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
    this->startLabel = buffer.genLabel();
    value = exp->value;
    type = exp->type;
    boolVal = exp->boolVal;
}

Exp::Exp(Type *type, Exp *exp) {//cant see type because it is announced later
    this->startLabel = buffer.genLabel();
    this->type = "";
    if (exp->type.compare(0, 5, "enum ") == 0) {//exp type is enum
        if (type->value == "INT") {//casting into int
            value = exp->value;
            this->type = "INT";
        }
    }
}

string loadVariable(int offset, string type) {
    //%val = load i32* %ptr
    string reg = pool.getReg();
    string ptrReg = pool.getReg();
    if (offset >= 0) {
        buffer.emit(
                "%" + ptrReg + " = getelementptr inbounds [ 50 x i32]* %stack, i32 0, i32 " + to_string(offset));
    }
    else{
//                %t5= add offset+sizeofargs
        buffer.emit(
                "%" + ptrReg + " = getelementptr inbounds [ "+to_string(currFuncArgs)+" x i32]* %args, i32 0, i32 " +
                to_string(currFuncArgs+offset));
    }
    buffer.emit("%" + reg + "= load i32* %" + ptrReg);
    string idType = get_LLVM_Type(type);
    string dataReg = reg;
    if (idType != "i32") {
        //%X= trunc i32 %Y to i8
        dataReg = pool.getReg();
        buffer.emit("%" + dataReg + " = trunc i32 %" + reg + " to " + idType);
    }
    return dataReg;

}

Exp::Exp(Node *ID) {
    this->startLabel = buffer.genLabel();
    this->type = "";
    for (int i = tablesStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j < tablesStack[i]->lines.size(); ++j) {
            if (tablesStack[i]->lines[j]->name == ID->value) {
                this->value = ID->value;
                this->type = tablesStack[i]->lines[j]->types.back();
                this->reg = loadVariable(tablesStack[i]->lines[j]->offset, this->type);
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
    this->startLabel = buffer.genLabel();
    this->type = call->value;
}

Exp::Exp(Exp *exp, string str) {//checking for if
    this->startLabel = buffer.genLabel();
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
            string argsDecl = "(";// will hold the arg decel string structured as such (i32,i32)
            string args = "(";//will hold the passed args themselves (i32 %x,i32 %y)
            if (i->types.size() == 1 + list->expList.size()) {//checking the number of arguments
                for (int j = 0; j < list->expList.size(); j++) {
                    args += get_LLVM_Type(i->types[j]) + "%" + list->expList[j].reg + ",";
                    argsDecl += get_LLVM_Type(i->types[j]) + ",";
                    if (list->expList[j].type == "BYTE" && i->types[j] == "INT") {
                        continue;
                    }
                    if (list->expList[j].type != i->types[j]) {
                        i->types.pop_back();
                        output::errorPrototypeMismatch(yylineno, i->name, i->types);
                        exit(0);
                    }
                }
                args.back() = ')';
                argsDecl.back() = ')';
                this->value = i->types.back();//TODO handle strings
                string funcName = ID->value;
                string retType = get_LLVM_Type(this->value);
                this->reg = pool.getReg();
                buffer.emit("%" + reg + " = call " + retType + " " + argsDecl + "* @" + funcName + args);
                /// this is a call with no parameters
                ///     %call = call i32 (i32,i32)* @foo(i32 %whhh,i32 %whhh)

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
                string funcName = ID->value;
                string retType = get_LLVM_Type(this->value);
                this->reg = pool.getReg();
                buffer.emit("%" + reg + " = call " + retType + " ()* @" + funcName + "()");
                /// this is a call with no parameters
                ///            %call = call i32 ()* @foo()

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
    if (args->formals.size() != 0) {// this will be used for the symbol table
        for (int i = 0; i < args->formals.size(); i++) {
            types.push_back(args->formals[i]->type);
        }
    } else {
        types.emplace_back("VOID");
    }
    types.emplace_back(retType->value);//emplacing the return type
    auto temp = shared_ptr<Entry>(new Entry(this->value, this->types, 0));
    tablesStack.back()->lines.push_back(temp);
    currFucn = ID->value;
    currFuncArgs= args->formals.size();
    string argString = ("(");//will be printed in the llvm command
///symbol table finished, starting to emit the LLVM
    if (args->formals.size() != 0) {//this is for the LLVM command
        for (int i = 0; i < args->formals.size(); i++) {//the only avialble types are BOOL,BYTE,INT,ENUM
            argString += get_LLVM_Type(args->formals[i]->type) + ",";  //using this for to make the argString
        }
        argString.back() = ')'; // args is "(type name,type name)"
    } else {//no args
        argString.append(")");// args is "()"
    }
    string retTypeString = get_LLVM_Type(retType->value);
    buffer.emit("define " + retTypeString + " @" + this->value + argString);
    //  define i32 @foo(i32,i32)

    ///initializing args and stack

    buffer.emit("%stack = alloca [50 x i32]");
    buffer.emit("%args = alloca [" + to_string(args->formals.size()) + " x i32]");//%args= alloca [10 x i32]
    int size=args->formals.size();
    for (int i = 0; i <size; i++) {
        string ptrReg = pool.getReg();//gets a new register to hold the ptr
        //%ptr = getelementptr inbounds[10 x i32]* %args, i32 0, i32 0
        //this is the syntax from class %first = getelementptr [10 x i32],[10 x i32]* %MyArr, i32 0, i32 0
        buffer.emit(
                "%" + ptrReg + " = getelementptr inbounds [" + to_string(size) + " x i32]* %args, i32 0, i32 " +
                to_string(currFuncArgs-i-1));//               4
        string dataReg = to_string(i);//                    0
        string argtype = get_LLVM_Type(args->formals[i]->type);
        if (argtype != "i32") {
            //%X = zext i8 %t3 to i32
            dataReg = pool.getReg();
            buffer.emit("%" + dataReg + "=zext " + argtype + " %" + to_string(i) + " to i32");
        }
        //store i32 %t3, i32* %ptr
        buffer.emit("store i32 %" + dataReg + ", i32* %" + ptrReg);
    }

}

//void pushStack(string reg){
//    buffer.emit("")
//}

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
    this->reg = pool.getReg();
    buffer.emit("%" + this->reg + " = add i32 0,0");//%reg= add i8 r3, r3
    string ptr = pool.getReg();
    buffer.emit("%" + ptr + " = getelementptr inbounds[50 x i32]* %stack, i32 0, i32 " + to_string(offset));
    string expType = get_LLVM_Type(type->value);
    string dataReg = reg;
    if (expType != "i32") {
        //%X = zext i8 %t3 to i32
        dataReg = pool.getReg();
        buffer.emit("%" + dataReg + "=zext " + expType + " %" + reg + " to i32");
    }
    buffer.emit("store i32 %" + dataReg + "i32* %" + ptr);
    //%ptr = getelementptr inbounds[10 x i32]* %args, i32 0, i32 0
    //store i32 %t3, i32* %ptr

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
    this->reg = pool.getReg();
    buffer.emit("%" + this->reg + " = add i32 0,0");//%reg= add i8 r3, r3
    string ptr = pool.getReg();
    buffer.emit("%" + ptr + " = getelementptr inbounds[50 x i32]* %stack, i32 0, i32 " + to_string(offset));
    string dataReg = reg;
    buffer.emit("store i32 %" + dataReg + "i32* %" + ptr);
    //%ptr = getelementptr inbounds[10 x i32]* %args, i32 0, i32 0
    //store i32 %t3, i32* %ptr
}

//int x= y;
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

    this->reg = pool.getReg();
    buffer.emit("%" + this->reg + " = add i32 0,%" + exp->reg);//%reg= add i8 %r3, %r3
    string ptr = pool.getReg();
    buffer.emit("%" + ptr + " = getelementptr inbounds[50 x i32]* %stack, i32 0, i32 " + to_string(offset));
    string dataReg = reg;
    buffer.emit("store i32 %" + dataReg + "i32* %" + ptr);
    //%ptr = getelementptr inbounds[10 x i32]* %args, i32 0, i32 0
    //store i32 %t3, i32* %ptr
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
    ///emitting code
    this->reg = pool.getReg();
    buffer.emit("%" + this->reg + " = add i32 0," + data);//%reg= add i8 r3, r3
    string ptr = pool.getReg();
    buffer.emit("%" + ptr + " = getelementptr inbounds[50 x i32]* %stack, i32 0, i32 " + to_string(offset));
    string dataReg = reg;
    buffer.emit("store i32 %" + dataReg + "i32* %" + ptr);
    //%ptr = getelementptr inbounds[10 x i32]* %args, i32 0, i32 0
    //store i32 %t3, i32* %ptr
}

string doEmitting(string data, string type, int offset) {
    ///emitting code
    string reg = pool.getReg();
    string datareg = data;
    string argtype = get_LLVM_Type(type);
    if (argtype != "i32") {
        //%X = zext  i8 %t6 to i32
        datareg = pool.getReg();
        buffer.emit("%" + datareg + "=zext " + argtype + " %" + data + " to i32");
    }
    buffer.emit("%" + reg + " = add i32 0,%" + datareg);//%reg= add i8 r3, r3
    string ptr = pool.getReg();
    if (offset >= 0) {
        buffer.emit(
                "%" + ptr + " = getelementptr inbounds [ 50 x i32]* %stack, i32 0, i32 " + to_string(offset));
    }
    else{
        buffer.emit(
                "%" + ptr + " = getelementptr inbounds [ "+to_string(currFuncArgs)+" x i32]* %args, i32 0, i32 " +
                to_string(currFuncArgs+offset));
    }
    buffer.emit("store i32 %" + reg + "i32* %" + ptr);
    return reg;
    //%ptr = getelementptr inbounds[10 x i32]* %args, i32 0, i32 0
    //store i32 %t3, i32* %ptr
}

//x= 15
Statement::Statement(Node *id, Exp *exp) {
    for (int i = tablesStack.size() - 1; i >= 0; i--) {
        for (int j = 0; j <
                        tablesStack[i]->lines.size(); ++j) {//finding in symbol table
            if (tablesStack[i]->lines[j]->name == id->value) {
                if (tablesStack[i]->lines[j]->types.size() ==
                    1) {//making sure this is not a function
                    if ((tablesStack[i]->lines[j]->types[0] == "INT" && exp->type == "BYTE") ||
                        tablesStack[i]->lines[j]->types[0] == exp->type) {//checking types
                        data = exp->value;
                        this->reg = doEmitting(exp->reg, exp->type, tablesStack[i]->lines[j]->offset);
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

//%X = trunc i32 257 to i8
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



/*target triple = "i386-pc-linux-gnu"

@.str = private constant [15 x i8] c"hello, world!\0A\00"

define i32 @foo(i32, i32) {
%args= alloca [10 x i32]
%ptr = getelementptr inbounds[10 x i32]* %args, i32 0, i32 0
%t3= add i8 5,2
%X = zext i8 %t3 to i32

store i32 %X, i32* %ptr

%val = load i32* %ptr
  %str = getelementptr inbounds [15 x i8]* @.str, i32 0, i32 %val
  %call = call i32 (i8*, ...)* @printf(i8* %str)
ret i32 1
}

define i32 @main() {
entry:
  %whhh= add i32 0,1
  %call = call i32 (i32,i32)* @foo(i32 %whhh,i32 %whhh)
  ret i32 1
}

declare i32 @printf(i8*, ...)

*/