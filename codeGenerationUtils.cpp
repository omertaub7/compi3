#include "codeGenerationUtils.hpp"
#include <sstream>
int global_var_counter = 0;
int global_stack_counter = 0;
int global_string_counter = 0;

std::map<string, string> string_global_identifiers;

string newTemp() {
    return ("%v" + std::to_string(global_var_counter++));
}

string newStack() {
    return ("%s" + std::to_string(global_stack_counter++));
}

string getCurrentStack() {
    return ("%s" + std::to_string(global_stack_counter));
}

void add_global_string(string name) {
    std::stringstream code;
    code << "@.str";
    code << std::to_string(global_string_counter++);
    string_global_identifiers[name] = code.str();
}

string get_global_string(string name) {
    return string_global_identifiers[name];
}