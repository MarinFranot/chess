#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "typesdef.h"


namespace Tools {
  extern const int MAX_LEGAL_MOVES;
  
  int getLastBitIndex(uint64_t x);
  int getFirstBitIndex(uint64_t x);
  int getCol(int pos);
  int getRank(int pos);
  int getPos(int col, int rank);
  int getDir(int from, int to);
  int toIndexPos(std::string square);
  std::string toSquare(int pos);
  int squareToInt(std::string str);
  bool startswith(const char *pre, const char *str);
  bool endsWith(const char* line, const char* suffix);
  bool isMove(const char* line);
  pieceType toPieceType(char c);
  
  uint64_t* getPiecesMovesMask(pieceType piece, bool white=true);
  uint64_t* getLineMasks(bool isRank);
  uint64_t* getDiagMasks(bool isUpRight);
  uint64_t** getTable(bool isRook, uint64_t* magicNbs, int* shifts);
}
