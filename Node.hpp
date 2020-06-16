
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
enum class LogicOp { AND, OR};
enum class RelOp { LT, LEQ, GT, GEQ}; // <, >, >=, <=
enum class EqOp {EQ, NEQ};// ==, !=
enum class BinOp {PLUS, MINUS, MUL, DIV}; // +,-,*,/
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

//==========================String=======================================
class StringNode : public Node {
    public:
    string s_value;
    StringNode(const string& s) : s_value(s) {}
};

//==========================Operators=======================================
class BinOperator : public Node {
    public:
    BinOp op;
    BinOperator(BinOp o) : op(o) {}
};

class RelOperator : public Node {
    public:
    RelOp op;
    RelOperator(RelOp o) : op(o) {}
};

class EqualOperator : public Node {
    public:
    EqOp op;
    EqualOperator(EqOp o) : op(o) {}
};

//===========================ID===========================================
class Id : public Node {
public:
    Id(const string& name) : Node(name) {}
};

// ========================= Exp ========================================
class Exp: public Node {
public:
    int value;
    string s_value;
    bool b_value;
    string storage_reg;
    Exp(TypeN type) : Node(type) {storage_reg = "";}
    Exp(TypeN type, int value) : Node (type), value(value) { s_value = ""; b_value = false; storage_reg = "";}
    Exp(TypeN type, string s) : Node(type), s_value(s) {value = 0; b_value = false; storage_reg = "";}
    Exp(TypeN type, bool flag) : Node(type), b_value(flag) {value = 0; s_value = ""; storage_reg = "";}
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
    FormalDecl(Type* type, Id* id) : Node(id->getName(), type->getType()) {}
};

//===========================FormalsList================================
class FormalsList : public Node {
public:
    vector<std::pair<string,TypeN> > argTypes;
    FormalsList(FormalDecl*);
    FormalsList(FormalDecl*, FormalsList*);
};

//===========================Formals================================
class Formals : public Node {
public:
    vector<std::pair<string,TypeN>> argTypes;
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
    vector<std::pair<string, TypeN>> argTypes;
    FuncDecl(string name, TypeN t, int num) : Node(name, t) {
        if (num == 0) {
            argTypes.push_back(std::pair<string,TypeN>("", TypeN::STRING));
        } else {
            argTypes.push_back(std::pair<string,TypeN>("", TypeN::INT));
        }
    }
    FuncDecl(RetType* pRetType, Formals* pFormals) : Node(pRetType->getType()), argTypes(pFormals->argTypes)
    {}
    FuncDecl(RetType* pRetType, Id* i, Formals* pFormals) : Node(i->getName(), pRetType->getType()), argTypes(pFormals->argTypes)
    {}
};

//===========================Funcs=================================
class Funcs : public Node {};

//===========================Program================================
class Program : public Node {};

#endif // _NODE