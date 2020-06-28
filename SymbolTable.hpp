#ifndef _SYMBOL_TABLE
#define _SYMBOL_TABLE

#include "Node.hpp"
#include <vector>
#include "Exceptions.hpp"
#include "hw3_output.hpp"
#include <cassert>
using namespace output;



typedef typename std::pair <Node, int> VarOffset;
typedef typename std::vector<VarOffset> Scope;

class GlobalSymbolTable {

    std::vector<Scope> scope_stack;
    std::vector<FuncDecl> functions;
    std::vector<int> offsets;
    TypeN currentReturnType;

public:
    GlobalSymbolTable() {
        offsets.push_back(-1);
        FuncDecl print = FuncDecl("print", TypeN::VOID, 0);
        FuncDecl printi = FuncDecl("printi", TypeN::VOID, 1);
        functions.push_back(print);
        functions.push_back(printi);
        currentReturnType =  TypeN::VOID;
    }
    ~GlobalSymbolTable() = default; // No need to do clear, the vector distructor already does that

    /*Symbol Table Modifiers*/
    void insertVarible(string name, TypeN type);
    void addNewScope();
    void popScope();
//    void addNewFunctionScope(); // never used
    void insertFunction (RetType* t, Id* id, Formals* args);
    void endGlobalScope();
    
    /*Symbol Table Checkers*/
    TypeN getSymbolType(Node* symbol); //Throws Undef
    bool checkSymbolIsFunction(Node* symbol);
    vector<std::pair<string,TypeN>> getFunctionArgs(Node* pId);
    TypeN getFuncRetType(string name);
    TypeN getCurrentReturnType();
    void setCurrentReturnType(TypeN t);

    int getVaribleOffset(string name);

    bool checkNameExists(string name);
    bool checkVarExists(string name);
    bool checkFunctionExists(string name);

};

#endif