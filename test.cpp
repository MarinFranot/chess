#include <iostream>
#include <cstring>
#include <unordered_set>
#include <cmath>

int f(int x, int &y){
    y = 7;
    return x*x;
}



int main() {
    
    int x = 7;
    int y = 1 + log2(x+1);

    std::cout << y << std::endl;


    return 0;
}
