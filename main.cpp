#include <iostream>
#include "position.h"
#include "bitboard.h"



int main() {
    int pos = 9;

    uint64_t mask = Bitboard::getRookMask(pos);

    std::cout << mask;

    return 0;
}
