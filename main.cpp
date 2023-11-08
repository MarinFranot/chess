#include <iostream>
#include <cmath>
#include <chrono>
#include <string>




#include "position.h"
#include "bitboard.h"
#include "search.h"
#include "tools.h"



int main(int argc, char* argv[]) {
  std::string fenInit = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  Chess::Position::init(fenInit);
  Chess::Search::init();
  


  if (argc >= 2 && strcmp(argv[1], "writeTable") == 0) {
    bool isRook = false;
    uint64_t obj = isRook ? std::pow(2, 14)-1 : 1023;
    Chess::Bitboard::generateLongTable(obj, isRook);

    Chess::Position::free();
    Chess::Search::free();
    return 0;
  }
  else if (argc >= 2 && strcmp(argv[1], "perf") == 0) {
    const std::string red = "\033[1;31m";
    const std::string green = "\033[1;32m";
    const std::string reset = "\033[0m";

    //std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    int correct[7] = {1, 20, 400, 8902, 197281, 4865609, 119060324};

    //std::string fen = "rnb1kb1r/ppp1pppp/5n2/q7/2B5/2N2N2/PPPP1PPP/R1BQK2R b KQkq - 5 5";
    //int correct[6] = {1, 42, 1504, 58284, 2022360, 77186084};

    //std::string fen = "r3k2r/pp2bp1p/4bP1p/2p5/1nBp4/5N2/PPP2P1P/R3K2R w KQkq - 0 14";
    //int correct[6] = {1, 32, 1027, 30977, 971930, 29282336};

    auto start = std::chrono::high_resolution_clock::now();

    int depth = 6;
    int nbComb = Chess::Position::getAllComb(depth, depth);
    if (nbComb != correct[depth]) {
      std::cout << red << "WRONG" << reset << " Number of combinations for depth " << depth << " -> " << nbComb << std::endl;
      std::cout << "Expected : " << correct[depth] << std::endl;
    }else {
      std::cout << green << "OK" << reset << " Number of combinations for depth " << depth << " : " << nbComb << std::endl;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start);
    std::cout << "Time taken by function: " << duration.count() << " milliseconds" << std::endl;

    Chess::Position::free();
    Chess::Search::free();
    return 0;
  }
  else if (argc >= 2 && strcmp(argv[1], "search") == 0) {
    int depth = 8;
    int alpha = -100000;
    int beta = 100000;
    int nbEval = 0;

    auto start = std::chrono::high_resolution_clock::now();
    int score = Chess::Search::search(depth, depth, alpha, beta, nbEval);
    Chess::Position::Move best = Chess::Search::getBest();
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start);
    std::cout << "Time taken by function: " << duration.count() << " milliseconds" << std::endl;
    std::cout << "Score : " << score << std::endl;
    std::cout << "Best move : " << best.toString() << std::endl;
    std::cout << "Number of evaluations : " << nbEval << std::endl;

    Chess::Position::free();
    Chess::Search::free();
    return 0;
  }
  
  return 0;
}

