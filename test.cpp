#include <iostream>
#include <cstring>
#include <unordered_set>
#include <cmath>

void f(int* x, int size){
    for (int i=0; i<size; i++){
        x[i]++;
    }
}



int main() {
    
    int x[] = {1,2,3,4,5};
    int size = 5;

    f(x, size);

    for (int i=0; i<size; i++){
        std::cout << x[i] << std::endl;
    }



    return 0;
}
