#pragma once

#include <random>
#include <unordered_map>


#include "position.h"
#include "typesdef.h"

namespace Chess {
namespace Search {

  void init();
  void free();
  int search(int initDepth, int depth, int alpha, int beta, int &nbEval);
  int eval();
  Position::Move getBest();


}
}