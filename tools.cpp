#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <cstring>

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

  bool endsWith(const char* line, const char* suffix) {
    size_t lineLength = strlen(line);
    size_t suffixLength = strlen(suffix);

    if (lineLength < suffixLength) {
        return false;
    }

    return strcmp(line + lineLength - suffixLength, suffix) == 0;
  }

  bool isMove(const char* line) {
    auto isValid = [](const char* subline) {
      bool valid = (subline[0] >= 'a' && subline[0] <= 'h') &&
                   (subline[1] >= '1' && subline[1] <= '8') &&
                   (subline[2] >= 'a' && subline[2] <= 'h') &&
                   (subline[3] >= '1' && subline[3] <= '8');
      return valid;
    };
    size_t lineLength = strlen(line);
    if (lineLength < 6) {
      return false;
    }
    const char* subline = line + lineLength - 6;
    
    if (subline[0] == ' ') {
      return isValid(subline + 1);
    } else {
      char p = subline[5];
      return isValid(subline) && (p == 'q' || p == 'r' || p == 'b' || p == 'n');
    }
  }
  pieceType toPieceType(char c) {
    switch (c) {
      case 'q':
        return QUEEN;
      case 'r':
        return ROOK;
      case 'b':
        return BISHOP;
      case 'n':
        return KNIGHT;
      default:
        return EMPTY;
    }
  }


  /////GENERATION////////

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


  //get the masks for all positions of the long range pieces from a file
  uint64_t** getTable(bool isRook, uint64_t* magicNbs, int* shifts){
    const char* filename = isRook ? "rookTable.txt" : "bishopTable.txt";
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the file");
    }

    char line[256];
    fgets(line, sizeof(line), file);
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
     while (fgets(line, sizeof(line), file)) {

      fgets(line, sizeof(line), file);
      std::string lineString(line);
      int nbComb = getNb(lineString);

      fgets(line, sizeof(line), file);
      lineString = std::string(line);
      int maxi = getNb(lineString);

      fgets(line, sizeof(line), file);
      lineString = std::string(line);
      uint64_t magicNb = std::stoull(lineString.substr(0, lineString.find(" ")));
      magicNbs[pos] = magicNb;

      fgets(line, sizeof(line), file);
      lineString = std::string(line);
      shifts[pos] = getNb(lineString);

      table[pos] = new uint64_t[maxi+1];
      for (int i=0; i<=maxi; i++){
        table[pos][i] = 777;
      }
      
      for (int j=0; j<nbComb; j++){
        fgets(line, sizeof(line), file);
        lineString = std::string(line);
        int idx = getNb(lineString);
        uint64_t mask = std::stoull(lineString.substr(lineString.find(" ")+1, lineString.length()));
        table[pos][idx] = mask;
      }
      pos++;
      
    }
    fclose(file);
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