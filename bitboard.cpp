#include <iostream>
#include "position.h"


namespace Bitboard{

    uint64_t getRookMask(int pos){
        uint64_t mask = 0;
        int col = Position::getCol(pos);
        int rank = Position::getRank(pos);

        for (int x = 1; x < 7; x++){
            mask |= 1ULL << (x*8 + col); 
            mask |= 1ULL << (rank*8 + x);
        }
        mask &= ~(1ULL << pos);

        return mask;
    }


    uint64_t getMoves(int pos, uint64_t mask, uint64_t pieces){
        uint64_t res = 0;
        int ways[] = {Position::north, Position::south, Position::east, Position::west};
        
        for (int way : ways){
            

            auto getFlagStop = [way, pieces](int x){
                int col = Position::getCol(x);
                int rank = Position::getRank(x);
                bool flagstop =   (col<=0 && way==Position::west) || 
                     (col>=7 && way==Position::east) || (rank<=0 && way==Position::south) || (rank>=7 && way==Position::north);
                return flagstop;
            };

            bool flagstop = getFlagStop(pos);
            int x = pos + way;
            
            while(!flagstop){
                res |= 1ULL << x;
                
                flagstop = getFlagStop(x) || ((pieces >> x) & 1ULL);
                x += way;
                if (flagstop){
                    break;
                }
            }
        }
        return res;
    }


}