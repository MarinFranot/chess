#include <iostream>
#include "position.h"
#include "bitboard.h"



int main(int argc, char* argv[]) {

    uint64_t pieces = std::stoull(argv[1]);
    int pos = 0;

    uint64_t mask = Bitboard::getRookMask(pos);

    uint64_t control = Bitboard::getMoves(pos, mask, pieces);

    std::cout << control;

    return 0;
}
