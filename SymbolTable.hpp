#ifndef _SYMBOL_TABLE
#define _SYMBOL_TABLE

#include "Node.hpp"
#include <vector>
#include "SyntaxExceptions.hpp"
#include "hw3_output.hpp"
using namespace output;

typedef std::vector<std::pair<Node*>, int> Scope;



class GlobalSymbolTable {
    std::vector<Scope*> scope_stack;
    std::vector<FuncDecl*> functions;
    std::vector<int> offsets;
    TypeN currentReturnType;
    public:
    GlobalSymbolTable() {
        offsets.push_back(0);
        currentReturnType =  TypeN::VOID;
    }
    /*Symbol Table Modifiers*/
    void insertVarible(Node* var);
    void addNewScope();
    Scope popScope();
    void addNewFunctionScope();
    void insertFunction (retType* t, ID* id, Formals* args);
    void endGlobalScope();
    
    /*Symbol Table Checkers*/
    TypeN getSymbolType(Node* symbol); //Throws Undef
    bool checkSymbolIsFunction(Node* symbol);
    vector<TypeN> getFunctionArgs(FuncDecl* Func);
    TypeN getCurrentReturnType();
    void setCurrentReturnType(TypeN t);
};

#endif