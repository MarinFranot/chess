#include <iostream>
#include <cmath>

#include "tools.h"


namespace Tools {

  int getLastBitIndex(uint64_t x){
    return std::log2(x & -x);
  }
  int getFirstBitIndex(uint64_t x){
      return std::log2(x);
  }

  int getCol(int pos){
    return pos%8;
  }
  int getRank(int pos){
    return pos/8;
  }
  int getPos(int col, int rank){
    return rank*8 + col;
  }

  int getDir(int from, int to){
    int deltaX = to%8 - from%8;
    int deltaY = to/8 - from/8;
    return (deltaX + 8*deltaY) / std::max(deltaX, deltaY);
  }

}