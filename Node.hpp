
#ifndef _NODE
#define _NODE

#include <vector>
#include <string>
#include <memory>
#include <cstdlib>
#include <cstring>

#define YYSTYPE Node*

using namespace std;


// scoped enum, to not interfere with the tokens
enum class TypeN {VOID, BOOL, INT, BYTE, STRING};

ostream& operator<<(ostream&, TypeN);
string to_string(TypeN);

//=========================Value ========================================
// This is for holding the value of the variable
// here all the semantic checks will go to

/*
class Value {
    TypeN type;
    int intValue;
    char byteValue;
    bool boolValue;
    string stringValue;

public:
    Value() : type(TypeN::VOID) {}
    Value(int i) : type(TypeN::INT), intValue(i) {}
    Value(char c) : type(TypeN::BYTE), byteValue(c) {}
    Value(bool b) : type(TypeN::BOOL), boolValue(b) {}
    Value(string s) : type(TypeN::STRING), stringValue(s) {}

    // TODO: add, assign, ...
};
*/

//====================== Node ===========================================

class Node {
private:
    string name;
    TypeN type;
    vector<Node*> children; 

public:
    Node();
    Node(string name, TypeN);
    virtual ~Node() = default;
    virtual string getName() const;
    virtual TypeN getType() const;
    virtual void addChild(Node* child);
    // for testing only
    virtual vector<string> _getInfo() const;
    virtual string _getClassName() const;
};

class Num : public Node {
public:
    int value;
    Num(const char* x) : value(atoi(x)) {}
};

// ========================= Exp ========================================
/* can be created from the following rules:
    Exp -> ( Exp )
    Exp -> Exp BINOP Exp
    Exp -> ID
    Exp -> Call
    Exp -> NUM
    Exp -> NUM B
    Exp -> STRING
    Exp -> TRUE
    Exp -> FALSE
    Exp -> NOT Exp
    Exp -> Exp AND Exp
    Exp -> Exp OR Exp
    Exp -> Exp RELOP Exp
*/
/*
class Exp: public Node {
    Value value;

public:
    Exp(Value value);
    getValue();
};
*/


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
*/



#endif // _NODE