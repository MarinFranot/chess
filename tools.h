#pragma once

#include <iostream>


namespace Tools {
  
  int getLastBitIndex(uint64_t x);
  int getFirstBitIndex(uint64_t x);
  int getCol(int pos);
  int getRank(int pos);
  int getPos(int col, int rank);
  int getDir(int from, int to);
}