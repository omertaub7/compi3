#include "codeGenerationUtils.hpp"
int global_var_counter = 0;
string newTemp() {
    return ("%v" + std::to_string(global_var_counter++));
}