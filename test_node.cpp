#include "Node.hpp"
#include <iostream>
using namespace std;

int main() {
    Node n;
    Node n1("x", "int");
    cout <<"n name:" << n.getName() << " type: " << n.getType() <<endl;
    cout <<"n1 name:" << n1.getName() << " type: " << n1.getType() <<endl;
}