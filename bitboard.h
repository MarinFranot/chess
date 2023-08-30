#pragma once
#include <iostream>

namespace Bitboard {

    uint64_t getRookMask(int pos);
    uint64_t getMoves(int pos, uint64_t pieces, bool restriction = false);
    uint64_t* getAllPiecesComb(int pos, int &arrsize);
    uint64_t getMagicNumber(uint64_t* picecsComb, int arrsize, int &maxTab);
    bool isUnique(uint64_t* picecsComb, int arrsize);
    uint64_t** getRookTable(uint64_t* magicNumbers);
    
}