#include "Node.hpp"

Node::Node() : name(""), type("") {}

Node::Node(string name, string type) : name(name), type(type) {}

string Node::getName() const {
    return name;
}

string Node::getType() const {
    return type;
}

void Node::addChild(shared_ptr<Node> child) {
    children.push_back(child);
}

