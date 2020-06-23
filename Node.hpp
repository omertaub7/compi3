
#ifndef _NODE
#define _NODE

#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include "bp.hpp"
using std::string;
using std::ostream;
using std::vector;
using std::to_string;


// scoped enum, to not interfere with the tokens
enum class TypeN { VOID, BOOL, INT, BYTE, STRING };
enum class LogicOp { AND, OR};
enum class RelOp { LT, LEQ, GT, GEQ, EQ, NEQ}; // <, >, >=, <=, ==, !=
enum class BinOp {PLUS, MINUS, MUL, DIV}; // +,-,*,/
typedef vector<pair<int, BranchLabelIndex>> BackpatchList;
ostream& operator<<(ostream&, TypeN);
string to_string(TypeN);

//====================== Node ===========================================
class Node {
private:
    string name;
    TypeN type;

public:
    Node();
    Node(const string& s); 
    Node(TypeN type);
    Node(const string& name, TypeN);
    virtual ~Node() = default;
    virtual string getName() const;
    virtual TypeN getType() const;
};

//==========================Num=======================================
class Num : public Node {
public:
    int value;
    Num(const string& s) : value(stoi(s)) {}
};

//==========================String=======================================
class String : public Node {
    public:
    string s;
    String(const string& s) : s(s) {}
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

//===========================ID===========================================
class Id : public Node {
public:
    Id(const string& name) : Node(name) {}
};

// ========================= Exp ========================================
class Exp: public Node {
public:
    // this is the place that the value will be stored, how to address it
    string place;
    int n_bytes;
    // places to backpatch with the address that 
    BackpatchList falselist;
    BackpatchList truelist;
    Exp(TypeN type, string place) : Node(type), place(place) {}
    Exp(TypeN type) : Node(type) {}
    // copy everithing except of the childeren & name
    Exp(const Exp& exp) : Node(exp.getType()), place(exp.place), n_bytes(exp.n_bytes),
        falselist(exp.falselist), truelist(exp.truelist) {}
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
class Statement : public Node {
public:
    BackpatchList continueList;
    BackpatchList breakList;
    BackpatchList nextlist;
};

//========================= Statements ================================
class Statements : public Node {
public:
    BackpatchList continueList;
    BackpatchList breakList;
    BackpatchList nextlist;
};

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

//===========================Markers===============================
class M : public Node {
public:
    string label;
};

class N : public Node {
public:
    BackpatchList nextlist;
};

#endif // _NODE