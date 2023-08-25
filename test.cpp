#include <iostream>


int main(){
    int x = 0;

    int i = 0;

    x |= 1<<i;

    std::cout << "x = " << x << std::endl;

    return 0;
}