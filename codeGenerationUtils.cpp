#include "codeGenerationUtils.hpp"
#include <sstream>
int global_var_counter = 0;
int global_string_counter = 0;

string newTemp() {
    return ("%v" + to_string(global_var_counter++));
}

string newString() {
    return ("@.s" + to_string(global_string_counter++));
}
