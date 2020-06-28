#ifndef EX5_CODE_GEN_UTILS
#define EX5_CODE_GEN_UTILS

#include <string>
#include "bp.hpp"
using std::string;
using std::to_string;


// creates a new temporary variable identifier
string newTemp(string base = "v");
// creates a new globel string identifier 
string newString(string base = "s");
// creates a new label
string newLabel(string base = "l");
#endif
