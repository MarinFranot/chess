#pragma once


namespace Position {
    extern const int boardSize;
    extern const int east;
    extern const int west;
    extern const int north;
    extern const int south;
    extern const int northEast;
    extern const int southEast;
    extern const int northWest;
    extern const int southWest;

    int getCol(int pos);
    int getRank(int pos);
    int getPos(int col, int rank);
    
}