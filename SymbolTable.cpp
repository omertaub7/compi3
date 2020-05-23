#include "SymbolTable.hpp"

void SymbolTable::insertVarible(Node* var) {
    //No Shadowing, first make sure that the new var does not exist in previous scopes

    for (const Scope& s : scope_stack) { //all scopes loop
        for (cont std::pair<Node*, int> p : s) { //Current scope loop
            if (p.getName() == var.getName()) {
                throw errorDef(p.getName());
            }
        }
    }
    int offset = offsets.back();
    scope_stack.back().push_back(std::pair<Node*, int>(var, offset+1)); //Push to the end of current scope with offset+1
    offsets.pop_back(); //Update last offset to be incremented by 1
    offsets.push_back(offset+1);
}

void SymbolTable::addNewScope() {
    int offset = offsets.back();
    scope_stack.push_back(Scope());
    offsets.push_back(offset);
}

Scope SymbolTable::popScope() {
    //Handle scope and offset stacks - pop from the top of the stack <==> end of the vector
    Scope s = scope_stack.back();
    scope_stack.pop_back();
    offsets.pop_back();
    //Print endScope line from output
    output::endScope();

    for (std::pair<Node*, int>& symbol : s) {
        output::printID((s.first())->getName(), s.second(), (s.first())->getType());
    }

}

void SymbolTable::insertFunction (Function* func) {
    functions.push_back(func);
    std::vector<Node*> args = func->getArgList();
    Scope function_scope;
    for (int i = 0; i < args.size(); i++) {
        function_scope.push_back(args[i], -1*(i+1));
    }
    scope_stack.push_back(function_scope);
}

#define FUNCTIONS_OFFSET 0
void SymbolTable::endGlobalScope() {
    bool found_main = false;
    for (Function* func : functions) {
        if (func->getName() == "main") {
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
    }
}