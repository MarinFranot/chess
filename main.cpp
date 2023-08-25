#include <iostream>
#include "position.h"
#include "bitboard.h"



int main() {
    int pos = 20;

    uint64_t mask = Bitboard::getRookMask(pos);

    std::cout << mask;

    return 0;
}
