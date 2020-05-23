#ifndef _SYMBOL_TABLE
#define _SYMBOL_TABLE

#include "Node.hpp"
#include <vector>
#include "SyntaxExceptions.hpp"
#include "hw3_output.hpp"
using namespace output;

typedef std::vector<std::pair<Node*>, int> Scope;



class GlobalSymbolTable {
    std::vector<Scope> scope_stack;
    std::vector<Function*> functions;
    std::vector<int> offsets;

    public:
    GlobalSymbolTable() {
        offsets.push_back(0);
    }
    void insertVarible(Node* var);
    void addNewScope();
    Scope popScope();
    void insertFunction (Function* func);
    void endGlobalScope();

};

#endif