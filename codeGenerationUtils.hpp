#ifndef EX5_CODE_GEN_UTILS
#define EX5_CODE_GEN_UTILS

#include <string>
#include "bp.hpp"
#include <map>
using namespace std;


string newTemp();
string newStack();
string getCurrentStack();
void add_global_string(string name);
string get_global_string(string name);
#endif
