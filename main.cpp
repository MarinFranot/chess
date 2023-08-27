#include <iostream>
#include "position.h"
#include "bitboard.h"



int main() {
    int pos = 28;

    int piecesPos[] = {30};
    uint64_t pieces = 0;
    for (int piece : piecesPos){
        pieces |= 1<<piece;
    }

    uint64_t mask = Bitboard::getRookMask(pos);

    uint64_t control = Bitboard::getMoves(pos, mask, pieces);

    std::cout << control;

    return 0;
}
