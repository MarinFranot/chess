#pragma once

#include "position.h"
#include "typesdef.h"

namespace Search {


  int search(int depth, int alpha, int beta);
  int eval();
  Position::Move getBest();


}