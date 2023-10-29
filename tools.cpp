#include <iostream>
#include <cmath>
#include <fstream>
#include <string>

#include "tools.h"


namespace Tools {
  const int MAX_LEGAL_MOVES = 219;

  int getLastBitIndex(uint64_t x){
    int res = std::log2(x & -x);
    return res;
  }
  int getFirstBitIndex(uint64_t x){
      return std::log2(x);
  }

  int getCol(int pos){
    return pos%8;
  }
  int getRank(int pos){
    return pos/8;
  }
  int getPos(int col, int rank){
    return rank*8 + col;
  }

  int getDir(int from, int to){
    int deltaX = to%8 - from%8;
    int deltaY = to/8 - from/8;
    return (deltaX + 8*deltaY) / std::max(abs(deltaX), abs(deltaY));
  }

  int toIndexPos(std::string square) {
    char col = square[0];
    char rank = square[1];
    return (rank - '1')*8 + (col - 'a');
  }

  std::string toSquare(int pos) {
    char col = 'a' + pos%8;
    char rank = '1' + pos/8;
    return std::string(1, col) + std::string(1, rank);
  }
  int squareToInt(std::string str) {
    return (str[1] - '1')*8 + (str[0] - 'a');
  }



  uint64_t* getLineMasks(bool isRank){
    uint64_t* res = new uint64_t[8];
    for (int i=0; i<8; i++){
      uint64_t line = 0;
      for (int j=0; j<8; j++){
        line |= 1ULL<<(isRank ? i*8+j : i+j*8);
      }
      res[i] = line;
    }
    return res;
  }

  uint64_t* getDiagMasks(bool isUpRight){
    uint64_t* res = new uint64_t[15];
    for (int i=0; i<15; i++) {
      res[i] = 0;
    }
    for(int i=0; i<64; i++){
      int idx = isUpRight ? 14-(i/8-i%8+7) : i/8+i%8;
      res[idx] |= 1ULL<<i;
    }
    return res;
  }

  uint64_t** getTable(bool isRook, uint64_t* magicNbs, int* shifts){
    std::ifstream file(isRook ? "rookTable.txt" : "bishopTable.txt");
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file.");
    }

    std::string line;
    std::getline(file, line);
    uint64_t** table = new uint64_t*[64];
    for (int i = 0; i < 64; i++) {
      table[i] = nullptr;
      magicNbs[i] = 777;
      shifts[i] = 777;
    }
    int pos = 0;
    auto getNb = [](std::string line){
      
      return std::stoi(line.substr(0, line.find(" ")));
    };
     while (std::getline(file, line)) {

      std::getline(file, line);
      int nbComb = getNb(line);

      std::getline(file, line);
      int maxi = getNb(line);

      std::getline(file, line);
      uint64_t magicNb = std::stoull(line.substr(0, line.find(" ")));
      magicNbs[pos] = magicNb;

      std::getline(file, line);
      shifts[pos] = getNb(line);

      table[pos] = new uint64_t[maxi+1];
      for (int i=0; i<=maxi; i++){
        table[pos][i] = 777;
      }
      
      for (int j=0; j<nbComb; j++){
        std::getline(file, line);
        int idx = getNb(line);
        uint64_t mask = std::stoull(line.substr(line.find(" ")+1, line.length()));
        table[pos][idx] = mask;
      }
      pos++;
      
    }
    file.close();
    return table;
  }

    //generate masks for all positions of low range pieces
  uint64_t* getPiecesMovesMask(pieceType piece, bool white){
    int piecesAdd[3][8] = {{7, 8, 9, 16, 8, 8, 8, 8}, //pawn
      {-6, -10, -15, -17, 6, 10, 15, 17}, //knight
      {-7, -8, -9, -1, 1, 7, 8, 9}}; //king
    int add[8];
    int idx = piece==KING ? 2 : piece;
    std::copy(std::begin(piecesAdd[idx]), std::end(piecesAdd[idx]), std::begin(add));

    uint64_t* res = new uint64_t[64];
    for (int pos=0; pos<64; pos++){
      uint64_t moves = 0;
      for (int move : add){
        if (piece==PAWN && !white){
          move = -move;
        }
        int newPos = pos+move;
        if ((newPos>=0) & (newPos<64)){
          if (!(((move+64)%8 < 8/2)^(newPos%8 >= pos%8))){
            moves |= 1ULL<<newPos;
          }
        }
      }
      res[pos] = moves;
    }

    return res;
  }

}