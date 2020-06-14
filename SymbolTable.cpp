#include "SymbolTable.hpp"
#include <iostream>
using std::cout;
using std::endl;

void GlobalSymbolTable::insertVarible(string id, TypeN type) {
    //No Shadowing, first make sure that the new var does not exist in previous scopes
    for (Scope& s : scope_stack) { //all scopes loop
        for (const VarOffset& p : s) { //Current scope loop
            if ((p.first).getName() == id) {
                throw errorDefException(id);
            }
        }
    }
    assert(offsets.size() > 0);
    int offset = offsets.back();
    Node n = Node(id, type);
    (scope_stack.back()).push_back(VarOffset(n, offset+1)); //Push to the end of current scope with offset+1
    assert(offsets.size() > 0);
    offsets.pop_back(); //Update last offset to be incremented by 1
    offsets.push_back(offset+1);
}

void GlobalSymbolTable::addNewScope() {
    assert(offsets.size() > 0);
    int offset = offsets.back();
    scope_stack.push_back(Scope());
    offsets.push_back(offset);
}

void GlobalSymbolTable::popScope() {
    //Handle scope and offset stacks - pop from the top of the stack <==> end of the vector
    Scope& s = scope_stack.back();
    
    //Print endScope line from output
    output::endScope();

    for (VarOffset& symbol : s) {
        string name = symbol.first.getName();
        TypeN type = symbol.first.getType();
        int offset = symbol.second;
        output::printID(name, offset, to_string(type));
    }

    scope_stack.pop_back();
    assert(offsets.size() > 0);
    offsets.pop_back();
}

void GlobalSymbolTable::insertFunction(RetType* t, Id* id, Formals* args) {
    for (FuncDecl& f : functions) {
        if (f.getName() == id->getName()) {
            throw errorDefException(id->getName());
        }
    }
    FuncDecl func = FuncDecl(t, id, args);
    functions.push_back(func);
    Scope function_scope = Scope();
    auto& argTypes = args->argTypes;
    for (int i = 0; i < argTypes.size(); i++) {
        string name = argTypes[i].first;
        TypeN type = argTypes[i].second;
        Node n = Node(name, type);
        function_scope.push_back(VarOffset(n, -1*(i+1)));
    }
    scope_stack.push_back(function_scope);
    offsets.push_back(-1);
}

#define FUNCTIONS_OFFSET 0
void GlobalSymbolTable::endGlobalScope() {
    bool found_main = false;
    for (FuncDecl& func : functions) {
        if ((func.getName() == "main") && (func.argTypes.size()==0) && func.getType() == TypeN::VOID)  {
            found_main = true;
            break;
        }
    }
    
    if (false == found_main) {
        throw errorMainMissingException();
    }
    output::endScope();
    for (FuncDecl& func : functions) {
        vector<string> argTypeNames;
        for (std::pair<string, TypeN>& t : func.argTypes) {
            argTypeNames.push_back(to_string(t.second));
        }
        output::printID(func.getName(), FUNCTIONS_OFFSET, output::makeFunctionType(to_string(func.getType()), argTypeNames));
    }
}


TypeN GlobalSymbolTable::getSymbolType(Node* symbol) {
    //Go on all varibles in avaliable Scopes
    for (const Scope& s : scope_stack) { //all scopes loop
        for (const VarOffset& p : s) { //Current scope loop
            if ((p.first).getName() == symbol->getName()) {
                return (p.first).getType();
            }
        }
    }
    //Check if function
    for (FuncDecl& func : functions) {
        if (func.getName() == symbol->getName()) {
            return func.getType();
        }
    }

    throw errorUndefException(symbol->getName()); //Symbol is not defined as function or varible

}

bool GlobalSymbolTable::checkSymbolIsFunction(Node* symbol) {
    for (FuncDecl& func : functions) {
        if (func.getName() == symbol->getName()) {
            return true;
        }
    }
    return false;
}

vector<std::pair<string,TypeN>> GlobalSymbolTable::getFunctionArgs(Node* symbol) {
    for (FuncDecl& func : functions) {
        if (func.getName() == symbol->getName()) {
            return func.argTypes;
        }
    }
    throw errorUndefFuncException(symbol->getName());
    
}


TypeN GlobalSymbolTable::getCurrentReturnType() {
    return currentReturnType;
}
    
void GlobalSymbolTable::setCurrentReturnType(TypeN t) {
    currentReturnType = t;
}

TypeN GlobalSymbolTable::getFuncRetType(string name) {
    for (FuncDecl& func : functions) {
        if (func.getName() == name) {
            return func.getType();
        }
    }
    throw errorUndefFuncException(name);
}