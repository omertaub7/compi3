#ifndef EX5_CODE_GEN_UTILS
#define EX5_CODE_GEN_UTILS

#include <string>
using std::string;
using std::to_string;


// creates a new temporary variable identifier
string newTemp();
// creates a new globel string identifier 
string newString();
// creates a new label
string newLabel();
#endif
