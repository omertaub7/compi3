#ifndef _SYMBOL_TABLE
#define _SYMBOL_TABLE

#include "Node.hpp"
#include <vector>
#include "Exceptions.hpp"
#include "hw3_output.hpp"
using namespace output;


typedef typename std::vector<std::pair<Node*, int >> Scope;

class GlobalSymbolTable {
    std::vector<Scope*> scope_stack;
    std::vector<FuncDecl*> functions;
    std::vector<int> offsets;
    std::vector<Node*> localNodeArr;
    TypeN currentReturnType;
    public:
    GlobalSymbolTable() {
        offsets.push_back(0);
        FuncDecl* print = new FuncDecl("print", TypeN::VOID, 0);
        FuncDecl* printi = new FuncDecl("printi", TypeN::VOID, 1);
        functions.push_back(print);
        functions.push_back(printi);
        currentReturnType =  TypeN::VOID;
    }
    ~GlobalSymbolTable() { 
        scope_stack.clear();
        functions.clear();
        offsets.clear();
        localNodeArr.clear();
    }
    /*Symbol Table Modifiers*/
    void insertVarible(string name, TypeN type);
    void addNewScope();
    void popScope();
    void addNewFunctionScope();
    void insertFunction (RetType* t, Id* id, Formals* args);
    void endGlobalScope();
    
    /*Symbol Table Checkers*/
    TypeN getSymbolType(Node* symbol); //Throws Undef
    bool checkSymbolIsFunction(Node* symbol);
    vector<std::pair<string,TypeN>> getFunctionArgs(Node* pId);
    TypeN getFuncRetType(string name);
    TypeN getCurrentReturnType();
    void setCurrentReturnType(TypeN t);
};

#endif