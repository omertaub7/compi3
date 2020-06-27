#include "codeGenerationUtils.hpp"
#include <sstream>
int global_var_counter = 0;
int global_string_counter = 0;
int global_label_counter = 0;

string newTemp() {
    return ("%v" + to_string(global_var_counter++));
}

string newString() {
    return ("@.s" + to_string(global_string_counter++));
}
string newLabel() {
    return ("l" + to_string(global_label_counter++));
}
