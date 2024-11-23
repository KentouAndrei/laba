#include <iostream>
#include "FuncA.h"
int CreateHTTPserver();
int main() {
    FuncA func;
    std::cout << "Result (with argument): " << func.calculate(10) << std::endl;
    
    CreateHTTPserver();
    
    return 0;
}