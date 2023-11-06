#pragma once
#include <iostream>

namespace Chess {
namespace Bitboard {

  uint64_t getMoves(int pos, uint64_t pieces, bool isRook, bool restriction=false);
  uint64_t* getAllPiecesComb(int pos, int &arrsize, bool isRook);
  uint64_t getMagicNumber(uint64_t* picecsComb, int arrsize, int &maxTab, uint64_t &shift, uint64_t obj);
  bool isUnique(uint64_t* picecsComb, int arrsize);
  void generateLongTable(uint64_t obj, bool isRook);
  
}
}