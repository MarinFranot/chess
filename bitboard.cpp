#include <iostream>
#include "position.h"




namespace Bitboard{

    uint64_t getRookMask(int pos){
        uint64_t mask = 0;
        int col = Position::getCol(pos);
        int rank = Position::getRank(pos);

        for (int x = 1; x < 8-1; x++){
            mask |= 1 << (x*8 + col); 
            mask |= 1 << (rank*8 + x);
        }
        mask &= ~(1 << pos);

        return mask;
    }


    


}