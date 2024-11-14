#include <iostream>
#include "FuncA.h"

int main() {
    FuncA func;
    std::cout << "Result (with argument): " << func.calculate(10) << std::endl;
    return 0;
}