#include <iostream>
#include <fstream>
#include <string>

#include "position.h"
#include "tools.h"



namespace Position {
  extern const int boardSize = 8;
  extern const int EAST = 1;
  extern const int WEST = -1;
  extern const int NORTH = 8;
  extern const int SOUTH = -8;
  extern const int NORTH_EAST = 9;
  extern const int SOUTH_EAST = -7;
  extern const int NORTH_WEST = 7;
  extern const int SOUTH_WEST = -9;
  uint64_t rim = 0xFF818181818181FF;

  uint64_t* pawnMoves;
  uint64_t* knightMoves;
  uint64_t* bishopMoves[64];
  uint64_t* rookMoves[64];
  uint64_t* queenMoves[64];
  uint64_t* kingMoves;

  uint64_t* bishopMagicNbs;
  uint64_t* rookMagicNbs;
  int* bishopShifts;
  int* rookShifts;

  uint64_t* rankMasks;
  uint64_t* colMasks;
  uint64_t* diagNEMasks;
  uint64_t* diagSEMasks;

  bool whiteMove;
  pieceType myPieces[64];
  pieceType ennemyPieces[64];
  uint64_t myPiecesMask;
  uint64_t ennemyPiecesMask;
  uint64_t myPawnsMask;
  uint64_t ennemyPawnsMask;

  uint64_t pins;
  int myKingPos;
  int ennemyKingPos;


  //generate masks for all positions of low range pieces
  uint64_t* getPiecesMovesMask(pieceType piece){
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
        int newPos = move+pos;
        if ((newPos>0) & (newPos<64)){
          if (!((move%8 < 8/2)^(newPos%8 > pos%8))){
            moves |= 1ULL<<newPos;
          }
        }
      }
      res[pos] = moves;
    }
    return res;
  }

  uint64_t* getLineMask(bool isRank){
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

  uint64_t* getDiagMask(bool isUpRight){
    uint64_t* res = new uint64_t[15];
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
      magicNbs[pos] = getNb(line);
      std::getline(file, line);
      shifts[pos] = getNb(line);

      table[pos] = new uint64_t[maxi];
      for (int j=0; j<nbComb; j++){
        std::getline(file, line);
        int idx = getNb(line);
        uint64_t mask = std::stoi(line.substr(line.find(" ")+1, line.length()));
        table[pos][idx] = mask;
      }
      pos++;
    }
    file.close();
    return table;
  }

  
  uint64_t getNEDiag(int pos){
    return diagNEMasks[14-(pos/8-pos%8+7)];
  }
  uint64_t getSEDiag(int pos){
    return diagSEMasks[pos/8+pos%8];
  }

  uint64_t lookupBishop(int pos){
    uint64_t mask = (myPiecesMask|ennemyPiecesMask) & (getNEDiag(pos)|getSEDiag(pos)) & ~(1ULL<<pos) & ~rim;
    return bishopMoves[pos][(mask*bishopMagicNbs[pos])>>bishopShifts[pos]];
  }
  uint64_t lookupRook(int pos){
    uint64_t mask = (myPiecesMask|ennemyPiecesMask) & (rankMasks[pos%8]|colMasks[pos/8]) & ~(1ULL<<pos) & ~rim;
    return rookMoves[pos][(mask*rookMagicNbs[pos])>>rookShifts[pos]];
  }
  uint64_t lookupQueen(int pos){
    return lookupBishop(pos) | lookupRook(pos);
  }


  uint64_t getMoves(pieceType piece, int pos, bool isWhite, bool getControl){
    uint64_t res = 0;
    if (piece == PAWN){
      int dir = whiteMove ? 1 : -1;
      uint64_t up1 = 1ULL<<(pos+8*dir);
      if (getControl){
        res = pawnMoves[pos] & ~rankMasks[pos%8];
      } else {
        res = pawnMoves[pos] & ~(myPiecesMask & ennemyPiecesMask & up1);
        if (pos/8 == 1 && (((myPiecesMask & ennemyPiecesMask)>>(pos+8*dir))&1ULL)){
          res &= ~(1ULL << (pos+16*dir));
        }
        res &= ennemyPiecesMask & up1;
      }
    } else if (piece == KNIGHT){
      res = getControl ? knightMoves[pos] : knightMoves[pos] & ~myPiecesMask;
    } else if (piece == BISHOP){
      res = getControl ? lookupBishop(pos) : lookupBishop(pos) & ~myPiecesMask;
    } else if (piece == ROOK){
      res = getControl ? lookupRook(pos) : lookupRook(pos) & ~myPiecesMask;
    } else if (piece == QUEEN){
      res = getControl ? lookupQueen(pos) : lookupQueen(pos) & ~myPiecesMask;
    } else if (piece == KING){
      res = getControl ? kingMoves[pos] : kingMoves[pos] & ~(myPiecesMask|ennemyPiecesMask);
    }
    return res;
  }


  void updatePins(){
    pins = 0;
    uint64_t queen = lookupQueen(ennemyKingPos);
    while(queen){
      int index = Tools::getFirstBitIndex(queen);
      queen &= (queen - 1);
      pieceType pinned = ennemyPieces[index];
      if (pinned != EMPTY){
        int dir = abs(Tools::getDir(ennemyKingPos, index));
        
      }
    }
  }

  void movePiece(Move move){
    int from = move.getFrom();
    int to = move.getTo();
    
    myPieces[to] = myPieces[from];
    myPieces[from] = EMPTY;
    myPiecesMask = (myPiecesMask & ~(1ULL<<from)) | (1ULL<<to);
    if (myPieces[to] == PAWN){
      myPawnsMask = (myPawnsMask & ~(1ULL<<from)) | (1ULL<<to);
    }

    if (move.isCapture()){
      ennemyPieces[to] = EMPTY;
      ennemyPiecesMask &= ~(1ULL<<to);
      if (move.getCapturedPiece() == PAWN){
        ennemyPawnsMask &= ~(1ULL<<to);
      }
    }

    updatePins();

  }

}


