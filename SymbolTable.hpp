#include "Node.hpp"
#include <vector>

typedef std::vector<std::pair<Node*>, int> Scope;

class FuncSymbolTable {
    

}

class GlobalSymbolTable {
    std::vector<std::vector<Node*>> scope_stack;
    std::vector<Node*> functions;
    std::vector<int> offsets;

    public:
    
    void insertVarible(Node* var);
    void addNewtScope();
    Scope popScope();
    void insertFunction (Node* func);
    void endGlobalScope();

}