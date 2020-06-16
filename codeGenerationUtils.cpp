#include "codeGenerationUtils.hpp"
int global_var_counter = 0;
int global_stack_counter = 0;
string newTemp() {
    return ("%v" + std::to_string(global_var_counter++));
}

string newStack() {
    return ("%s" + std::to_string(global_stack_counter++));
}