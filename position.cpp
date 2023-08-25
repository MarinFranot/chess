#include "position.h"


namespace Position {
    const int boardSize = 8;

    int getCol(int pos){
        return pos%boardSize;
    }

    int getRank(int pos){
        return pos/boardSize;
    }
}


