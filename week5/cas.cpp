#include <iostream>

int main(){
    int original[3] = {0,1,1};
    int expected = 0;
    std::cout << original[1] << std::endl;
    std::cout << __atomic_compare_exchange_n(&original[1], &expected, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) << std::endl;
    std::cout << original[1] << std::endl;
    std::cout << expected << std::endl;
}