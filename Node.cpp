#include "Node.hpp"

//======================= General ==============================
ostream& operator<<(ostream& os, TypeN type) {
    os << to_string(type);
    return os;
}

string to_string(TypeN type) {
    switch (type)
    {
        case TypeN::INT: return "int"; break;
        case TypeN::VOID: return "void"; break;
        case TypeN::BYTE: return "byte"; break;
        case TypeN::STRING: return "string"; break;
    }
    // TypeN::BOOL
    return "bool";
}

//=======================Node===================================
Node::Node() : name(""), type(TypeN::VOID) {}

Node::Node(string& s) : name(s), type(TypeN::VOID) { }

Node::Node(string& name, TypeN type) : name(name), type(type) {}

string Node::getName() const {
    return name;
}

TypeN Node::getType() const {
    return type;
}

void Node::addChild(Node* child) {
    children.push_back(child);
}

vector<string> Node::_getInfo() const{
    vector<string> info;
    info.push_back(_getClassName());
    info.push_back(name);
    info.push_back(to_string(type));
    
    for(Node* child : children) {
        vector<string> child_info = child->_getInfo();
        info.insert(info.end(), child_info.begin(), child_info.end());
    }
    return info;
}

string Node::_getClassName() const {
    return "Node";
}


