#include "codeGenerationUtils.hpp"
#include <sstream>
int global_var_counter = 0;
int global_string_counter = 0;
int global_label_counter = 0;

string newTemp(string base) {
    return ("%" + base + to_string(global_var_counter++));
}

string newString(string base) {
    return ("@." + base + to_string(global_string_counter++));
}
string newLabel(string base) {
    return ("_" + base + to_string(global_label_counter++));
}
