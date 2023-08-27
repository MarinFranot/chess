#include "position.h"


namespace Position {
    const int boardSize = 8;
    const int east = 1;
    const int west = -east;
    const int north = boardSize;
    const int south = -north;
    const int northEast = north + east;
    const int southEast = south + east;
    const int northWest = north + west;
    const int southWest = south + west;

    int getCol(int pos){
        return pos%boardSize;
    }

    int getRank(int pos){
        return pos/boardSize;
    }

    int getPos(int col, int rank){
        return rank*boardSize + col;
    }
}


