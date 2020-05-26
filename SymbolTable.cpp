#include "SymbolTable.hpp"

void GlobalSymbolTable::insertVarible(Node* var) {
    //No Shadowing, first make sure that the new var does not exist in previous scopes

    for (const Scope* s : scope_stack) { //all scopes loop
        for (cont std::pair<Node*, int> p : s) { //Current scope loop
            if ((p.first())->getName() == var->getName()) {
                throw errorDefException(var->getName());
            }
        }
    }
    int offset = offsets.back();
    (scope_stack.back())->push_back(std::pair<Node*, int>(var, offset+1)); //Push to the end of current scope with offset+1
    offsets.pop_back(); //Update last offset to be incremented by 1
    offsets.push_back(offset+1);
}

void GlobalSymbolTable::addNewScope() {
    int offset = offsets.back();
    scope_stack.push_back(new Scope());
    offsets.push_back(offset);
}

Scope GlobalSymbolTable::popScope() {
    //Handle scope and offset stacks - pop from the top of the stack <==> end of the vector
    Scope* s = scope_stack.back();
    scope_stack.pop_back();
    offsets.pop_back();
    //Print endScope line from output
    output::endScope();

    for (std::pair<Node*, int>& symbol : s) {
        output::printID((s.first())->getName(), s.second(), (s.first())->getType());
    }

    delete s;

}

void GlobalSymbolTable::insertFunction(retType* t, ID* id, Formals* args) {
    for (FuncDecl* f : functions) {
        if (f->getName() == id->getName()) {
            throw errorDefException(func->getName());
        }
    }
    FuncDecl* func = new FuncDecl(t, id, args);
    functions.push_back(func);
    Scope* function_scope = new Scope();
    for (int i = 0; i < args.size(); i++) {
        function_scope->push_back(args[i], -1*(i+1));
    }
    scope_stack.push_back(function_scope);
}

#define FUNCTIONS_OFFSET 0
void SymbolTable::endGlobalScope() {
    bool found_main = false;
    for (Function* func : functions) {
        if ((func->getName() == "main") && ((func->argTypes).size()==0 /*Main Has no Args*/)) {
            found_main = true;
            break;
        }
    }
    
    if (false == found_main) {
        throw errorMainMissingException();
    }
    output::endScope();
    for (Function* func : functions) {
        output::printID(func->getName(), FUNCTIONS_OFFSET, output::makeFunctionType(func->getRetType(), func->getArgList()));
        delete func;
    }
}


Node* GlobalSymbolTable::getSymbolType(Node* symbol) {
    //Go on all varibles in avaliable Scopes
    for (const Scope* s : scope_stack) { //all scopes loop
        for (cont std::pair<Node*, int> p : s) { //Current scope loop
            if ((p.first())->getName() == symbol->getName()) {
                return (p.first())->getType();
            }
        }
    }
    //Check if function
    for (Function* func : functions) {
        if (func->getName() == symbol->getName()) {
            return func->getType();
        }
    }

    throw errorUndefException(symbol->getName()); //Symbol is not defined as function or varible

}

bool GlobalSymbolTable::checkSymbolIsFunction(Node* symbol) {
    for (Function* func : functions) {
        if (func->getName() == symbol->getName()) {
            return true;
        }
    }
    return false;
}

vector<TypeN> GlobalSymbolTable::getFunctionArgs(FuncDecl* symbol) {
    for (Function* func : functions) {
        if (func->getName() == symbol->getName()) {
            return func->argTypes;
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