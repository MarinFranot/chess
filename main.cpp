#include <iostream>
#include <cmath>
#include <chrono>


#include "position.h"
#include "bitboard.h"


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
    bool isRook = false;
    uint64_t obj = isRook ? std::pow(2, 14)-1 : 1023;
    Bitboard::generateLongTable(obj, isRook);
  } else if (argument == "perf") {

    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    int correct[6] = {1, 20, 400, 8902, 197281, 4865609};
    Position::init(fen);

    /*Position::Move  move = Position::Move(Tools::squareToInt("e2"), Tools::squareToInt("e4"), false);
    Position::movePiece(move);
    Position::Move  move2 = Position::Move(Tools::squareToInt("d7"), Tools::squareToInt("d5"), false);
    Position::movePiece(move2);
    Position::Move  move3 = Position::Move(Tools::squareToInt("f1"), Tools::squareToInt("b5"), false);
    Position::movePiece(move3);*/


    auto start = std::chrono::high_resolution_clock::now();

    int depth = 5;
    int nbComb = Position::getAllComb(depth, depth);
    if (nbComb != correct[depth]) {
      std::cout << "WRONG number of combinations for depth " << depth << " -> " << nbComb << std::endl;
      std::cout << "Expected : " << correct[depth] << std::endl;
    }else {
      std::cout << "OK Number of combinations for depth " << depth << " : " << nbComb << std::endl;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start);
    std::cout << "Time taken by function: " << duration.count() << " milliseconds" << std::endl;


    Position::free();
  }
  else {
    std::cout << "Error: invalid argument " << argv[1] << std::endl;
  }

  return 0;
}

