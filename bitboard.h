#pragma once
#include <iostream>

namespace Bitboard {

    uint64_t getRookMask(int pos);
    uint64_t getMoves(int pos, uint64_t mask, uint64_t pieces);
    
}