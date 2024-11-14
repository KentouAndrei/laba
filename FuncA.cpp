#include "FuncA.h"

double FuncA::calculate(int n) {
    double sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += 1.0 / (1 << i);  // Использование битового сдвига для вычисления 1/(2^i)
    }
    return sum;
}