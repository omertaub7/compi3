
#ifndef _NODE
#define _NODE

#include <vector>
#include <string>
#include <memory>

using namespace std;

class Node {
private:
    string name;
    string type;
    vector<shared_ptr<Node>> children; 

public:
    Node();
    Node(string name, string type);
    virtual ~Node() = default;
    string getName() const;
    string getType() const;
    void addChild(shared_ptr<Node> child);
};

/*
class Program: public Node {
public:
    Program();
};

class Funcs: public Node {
public:
    Funcs();
};

class FuncDecl: public Node {
public:
    FuncDecl();
};

class RetType: public Node {
public:
    RetType();
};

class Formals: public Node {
    Formals();
};

class FormalsList: public Node {
public:
    FormalsList();
};

class FormalDecl: public Node {
public: 
    FormalsDecl()
};

class Statements: public Node {
public:
    Statements();
};

class Statement: public Node {
public:
    Statement();
};

class Call: public Node {
public:
    Call();
};

class ExpList: public Node {
public:
    ExpList();
};

class Type: public Node {
public:
    Type();
};

class Exp: public Node {
public:
    Exp();
};
*/

#endif // _NODE