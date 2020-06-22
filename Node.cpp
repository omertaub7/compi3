#include "Node.hpp"

//======================= General ==============================
ostream& operator<<(ostream& os, TypeN type) {
    os << to_string(type);
    return os;
}

string to_string(TypeN type) {
    switch (type)
    {
        case TypeN::INT: return "INT"; break;
        case TypeN::VOID: return "VOID"; break;
        case TypeN::BYTE: return "BYTE"; break;
        case TypeN::STRING: return "STRING"; break;
    }
    // TypeN::BOOL
    return "BOOL";
}

//=======================Node===================================
Node::Node() : name(""), type(TypeN::VOID) {}

Node::Node(const string& s) : name(s), type(TypeN::VOID) { }

Node::Node(TypeN type) : name(""), type(type) {}

Node::Node(const string& name, TypeN type) : name(name), type(type) {}

string Node::getName() const {
    return name;
}

TypeN Node::getType() const {
    return type;
}


//========================== Exp List ====================================
ExpList::ExpList(Exp* pExp) {
    expList.push_back(pExp);
}

ExpList::ExpList(Exp* pExp, ExpList* pExpList) {
    expList.push_back(pExp);
    expList.insert(expList.end(), pExpList->expList.begin(), pExpList->expList.end());
}

//===========================FormalsList================================
FormalsList::FormalsList(FormalDecl* pFormalDecl) {
    argTypes.push_back(std::pair<string, TypeN>(pFormalDecl->getName(), pFormalDecl->getType()));
}

FormalsList::FormalsList(FormalDecl* pFormalDecl, FormalsList* pFormalsList) {
    argTypes.push_back(std::pair<string, TypeN>(pFormalDecl->getName(), pFormalDecl->getType()));
    argTypes.insert(argTypes.end(), pFormalsList->argTypes.begin(), pFormalsList->argTypes.end());
}