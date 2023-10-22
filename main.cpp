#include <iostream>
#include "position.h"
#include "bitboard.h"
#include <cmath>


int main(int argc, char* argv[]) {
  std::string argument = argv[1];
  uint64_t objMagic = std::pow(2, 14)-1;
  if (argument == "getMoves"){
    uint64_t pieces = std::stoull(argv[1]);
    int pos = 0;
    uint64_t control = Bitboard::getMoves(pos, pieces, true);
    std::cout << control;

  } else if (argument == "getTab"){
    int pos = 28;
    int arrsize = 0;
    uint64_t* tab = Bitboard::getAllPiecesComb(pos, arrsize, true);

    int maxTab = 0;
    uint64_t shift = 0;
    uint64_t magicNb = Bitboard::getMagicNumber(tab, arrsize, maxTab, shift, objMagic);

    std::cout << "magicNb result : " << magicNb << std::endl;

    delete[] tab;
  } else if (argument == "writeTable"){
    Bitboard::generateLongTable(objMagic, true);
  }
  else {
    std::cout << "Error: invalid argument " << argv[1] << std::endl;
  }

  return 0;
}

