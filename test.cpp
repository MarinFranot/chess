#include <iostream>




void f(int x){
    int y = 2;
    auto g = [&y](){y++;};

    
    g();
    g();
    std::cout << "y: " << y << std::endl;

    std::cout << x << std::endl;
}










int main(){
    uint64_t x = 0b101;

    int i = 2;

    bool bit_i = (x >> i) & 1;

    std::cout << bit_i << std::endl;

}