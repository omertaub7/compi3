
#ifndef _NODE
#define _NODE

#include <sstream>
#include <vector>
#include <string>
#include <memory>

using std::string;
using std::ostream;
using std::vector;
using std::to_string;


// scoped enum, to not interfere with the tokens
enum class TypeN { VOID, BOOL, INT, BYTE, STRING };

ostream& operator<<(ostream&, TypeN);
string to_string(TypeN);

//====================== Node ===========================================
class Node {
private:
    string name;
    TypeN type;
    vector<Node*> children; 

public:
    Node();
    Node(const string& s); 
    Node(TypeN type);
    Node(const string& name, TypeN);
    virtual ~Node() = default;
    virtual string getName() const;
    virtual TypeN getType() const;
    virtual void addChild(Node* child);

    // for testing only
    virtual vector<string> _getInfo() const;
    virtual string _getClassName() const;
};

//==========================Num=======================================
class Num : public Node {
public:
    int value;
    Num(const string& s) : value(stoi(s)) {}
};

//===========================ID===========================================
class Id : public Node {
public:
    Id(const string& name) : Node(name) {}
};

// ========================= Exp ========================================
class Exp: public Node {
public:
    Exp(TypeN type) : Node(type) {}
};

//========================== Type =======================================
class Type : public Node {
public:
    Type(TypeN type) : Node(type) {}
};

//========================== Exp List ====================================
class ExpList : public Node {
public:
    vector<Exp*> expList;
    ExpList(Exp*);
    ExpList(Exp*, ExpList*);
};

//========================== Call ======================================
class Call : public Node {
public:
    Call(TypeN type) : Node(type) {}
};
//========================= Statement =================================
class Statement : public Node {};

//========================= Statements ================================
class Statements : public Node {};

//===========================FormalDecl================================
class FormalDecl : public Node {
public:
    FormalDecl(Type* type) : Node(type->getType()) {}
};

//===========================FormalsList================================
class FormalsList : public Node {
public:
    vector<TypeN> argTypes;
    FormalsList(FormalDecl*);
    FormalsList(FormalDecl*, FormalsList*);
};

//===========================Formals================================
class Formals : public Node {
public:
    vector<TypeN> argTypes;
    Formals() = default;
    Formals(FormalsList* pFormalsList) : argTypes(pFormalsList->argTypes) {}
};

//===========================RetType================================
class RetType : public Node {
public:
    RetType() : Node(TypeN::VOID) {}
    RetType(Type* pType) : Node(pType->getType()) {}
};

//===========================FuncDecl================================
class FuncDecl : public Node {
public:
    vector<TypeN> argTypes;
    FuncDecl(RetType* pRetType, Formals* pFormals) : Node(pRetType->getType()), argTypes(pFormals->argTypes)
    {}
    FuncDecl(RetType* pRetType, ID* id, Formals* pFormals) : Node(id->getName(), pRetType->getType()), argTypes(pFormals->argTypes)
    {}
};

//===========================Funcs=================================
class Funcs : public Node {};

//===========================Program================================
class Program : public Node {};

#endif // _NODE