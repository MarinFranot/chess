#include <iostream>
#include <string>
#include <cstring>
#include <fstream>


#include "typesdef.h"
#include "position.h"
#include "search.h"
#include "tools.h"

namespace Chess {
namespace UCI {

  Chess::Position::Move createMove(Chess::Position::Pos pos, const char* line);
  void runUCI();
}
}