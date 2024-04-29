#pragma once

#include <random>
#include <unordered_map>
#include <algorithm>


#include "position.h"
#include "typesdef.h"

namespace Chess {
namespace Search {

  void init();
  void free();
  void clearTranspositionTable();
  int search(Position::Pos &pos, int initDepth, int depth, int alpha, int beta, int &nbEval);
  int eval(Position::Pos &pos);
  Position::Move getBest();


}
}