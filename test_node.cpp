/*
#include "Node.hpp"
#include <iostream>
using namespace std;

int main() {
    // Node test 1
    Node n;
    Node n1("x", TypeN::INT);
    cout <<"n name:" << n.getName() << " type: " << n.getType() <<endl;
    cout <<"n1 name:" << n1.getName() << " type: " << n1.getType() <<endl;

    // Node test 2, inserting
    cout << "adding children: " <<endl;
    for (int i = 0; i < 10; i++) {
        string name = "var" + to_string(i);
        TypeN type = TypeN::VOID;
        if (i % 2 == 0){
            type = TypeN::INT;
        }
        cout <<"(" <<name << ", " << type <<"), ";
        // testing with new operator and casting
        // Node* new_n = new Node(name, type);
        // n.addChild(new_n);
        // ^^ This will not work, compilation error

        n.addChild(make_shared<Node>(name, type));
    }
    cout <<endl;
    
    cout <<"printing childrens " <<endl;
    vector<string> info = n._getInfo();
    for (string msg : info) {
        cout << msg <<", ";
    }
    cout <<endl;

    // Node test 3, herarchy
    cout <<"nested nodes: " <<endl;
    shared_ptr<Node> root = make_shared<Node>("root", TypeN::VOID);
    for (int i = 0; i < 2; i++) {
        shared_ptr<Node> new_node = make_shared<Node>(to_string(i), TypeN::INT);
        for (int j = 0; j < 2; j++) {
            string name = to_string(i) + "_" + to_string(j);
            shared_ptr<Node> new_node2 = make_shared<Node>(name, TypeN::BOOL);
            new_node->addChild(new_node2);
        }
        root->addChild(new_node);
    }

    cout <<"printing childrens " <<endl;
    info = root->_getInfo();
    for (string msg : info) {
        cout << msg <<", ";
    }
    cout <<endl;

}
*/