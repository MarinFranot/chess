#include <iostream>
#include <cmath>
#include <chrono>
#include <string>
#include <cstring>
#include <fstream>

#include "position.h"
#include "bitboard.h"
#include "search.h"
#include "tools.h"
#include "uci.h"



int main(int argc, char* argv[]) {

  //generate the tables
  if (argc >= 2 && strcmp(argv[1], "writeTable") == 0) {
    bool isRook = false;
    uint64_t obj = isRook ? std::pow(2, 14)-1 : 1023;
    Chess::Bitboard::generateLongTable(obj, isRook);

    Chess::Position::free();
    Chess::Search::free();
    return 0;
  }
  //test the performance for generating all combinations
  else if (argc >= 2 && strcmp(argv[1], "perf") == 0) {
    Chess::Position::Pos pos = Chess::Position::Pos();
    Chess::Position::init(pos, fenInit);
    Chess::Search::init();
    const std::string red = "\033[1;31m";
    const std::string green = "\033[1;32m";
    const std::string reset = "\033[0m";

    int correct[7] = {1, 20, 400, 8902, 197281, 4865609, 119060324};




    std::cout << "start perft" << std::endl;


    auto start = std::chrono::high_resolution_clock::now();

    int nbthreads = 1;
    int depth = 6;
    int nbComb = Chess::Position::getAllComb(pos, depth, depth, nbthreads, false);
    if (nbComb != correct[depth]) {
      std::cout << red << "WRONG" << reset << " Number of combinations for depth " << depth << " -> " << nbComb << std::endl;
      std::cout << "Expected : " << correct[depth] << std::endl;
    }else {
      std::cout << green << "OK" << reset << " Number of combinations for depth " << depth << " : " << nbComb << std::endl;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start);
    std::cout << "Time taken by function: " << duration.count() << " milliseconds" << std::endl;

    pos.freeHistory();
    Chess::Position::free();
    Chess::Search::free();

    return 0;
  }
  // play chess with the uci protocol
  else {
    Chess::UCI::runUCI();
  }
  
  return 0;
}

