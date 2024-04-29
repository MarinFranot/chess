#pragma once

#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

#include "typesdef.h"
#include "tools.h"

namespace Chess {
namespace Position {
  extern const int EAST;
  extern const int WEST;
  extern const int NORTH;
  extern const int SOUTH;
  extern const int NORTH_EAST;
  extern const int SOUTH_EAST;
  extern const int NORTH_WEST;
  extern const int SOUTH_WEST;

  // move object with details
  struct Move {
    uint64_t value;
    int score = -1000000;
    Move(uint64_t initialValue) {
      value = initialValue;
    }
    Move(int from, int to, bool capture, pieceType captured=EMPTY, bool enPassant=false,
     bool is00=false, bool is000=false, bool isPromotion=false, pieceType promotionPiece=EMPTY){
      value = from | (to << 6);
      value |= capture << 12;
      value |= static_cast<uint64_t>(captured) << 13;
      value |= enPassant << 16;
      value |= is00 << 17;
      value |= is000 << 18;
      value |= isPromotion << 19;
      value |= static_cast<uint64_t>(promotionPiece) << 20;
    }
    Move(){
      value = 0;
    }

    int getFrom(){
      return value & 0b111111ULL;
    }
    int getTo(){
      return (value >> 6) & 0b111111ULL;
    }
    bool isCapture(){
      return (value >> 12) & 1ULL;
    }
    pieceType getCapturedPiece(){
      return static_cast<pieceType>((value >> 13) & 0b111ULL);
    }
    bool isEnPassant(){
      return (value >> 16) & 1ULL;
    }
    bool is00(){
      return (value >> 17) & 1ULL;
    }
    bool is000(){
      return (value >> 18) & 1ULL;
    }
    bool isPromotion(){
      return (value >> 19) & 1ULL;
    }
    pieceType getPromotion(){
      return static_cast<pieceType>((value >> 20) & 0b111ULL);
    }

    std::string toString(){
      std::string prom = isPromotion() ? pieceStr[getPromotion()] : "";
      return Tools::toSquare(getFrom()) + Tools::toSquare(getTo()) + prom;
    }
    std::string getDetails() {
      std::string res = toString();
      res += isCapture() ? " x " : "";
      res += isEnPassant() ? " en passant " : "";
      res += is00() ? " 0-0 " : "";
      res += is000() ? " 0-0-0 " : "";
      res += isPromotion() ? " prom " : "";
      return res;
    }

  };

  // previous position variables
  struct SavePos {
    uint64_t myControl;
    uint64_t ennemyControl;
    uint64_t pins;
    uint64_t* pinsMasks;
    Move* legalMoves;
    bool possible00[2];
    bool possible000[2];
    int enPassant;
    int halfMovesSinceReset;
    Move currentMove;
    bool check;
    int checkers[2];
    uint64_t blockCheck;
    int nbLegalMoves;

    SavePos() : myControl(0), ennemyControl(0), pins(0), possible00{false, false}, possible000{false, false},
     enPassant(-1), halfMovesSinceReset(0), currentMove(Move()), check(false), checkers{-1, -1}, blockCheck(UINT64_MAX), nbLegalMoves(0) {
      legalMoves = new Move[Tools::MAX_LEGAL_MOVES];
      pinsMasks = new uint64_t[64];
      for(int i=0; i<Tools::MAX_LEGAL_MOVES; i++){
        legalMoves[i] = Move();
        if (i<64){
          pinsMasks[i] = 0;
        }
      }
    }
    SavePos(const SavePos& other) {
      // Perform a deep copy of the data members
      this->myControl = other.myControl;
      this->ennemyControl = other.ennemyControl;
      this->pins = other.pins;
      this->possible00[0] = other.possible00[0];
      this->possible00[1] = other.possible00[1];
      this->possible000[0] = other.possible000[0];
      this->possible000[1] = other.possible000[1];
      this->enPassant = other.enPassant;
      this->halfMovesSinceReset = other.halfMovesSinceReset;
      this->currentMove = other.currentMove;
      this->checkers[0] = other.checkers[0];
      this->checkers[1] = other.checkers[1];
      this->check = other.check;
      this->blockCheck = other.blockCheck;
      this->nbLegalMoves = other.nbLegalMoves;

      
      // Allocate new memory and copy data for legalMoves and pinsMasks
      this->legalMoves = new Move[Tools::MAX_LEGAL_MOVES];
      std::memcpy(this->legalMoves, other.legalMoves, Tools::MAX_LEGAL_MOVES * sizeof(Move));
      this->pinsMasks = new uint64_t[64];
      std::memcpy(this->pinsMasks, other.pinsMasks, 64 * sizeof(uint64_t));
    }


    ~SavePos() {
      delete[] this->legalMoves;
      delete[] this->pinsMasks;
    }
  };

  // position variables
  struct Pos {
    pieceType* myPieces;
    pieceType* ennemyPieces;
    uint64_t myPiecesMask;
    uint64_t ennemyPiecesMask;
    uint64_t myPawnsMask;
    uint64_t ennemyPawnsMask;
    int myKingPos;
    int ennemyKingPos;
    SavePos** history;
    SavePos* currentPos;
    bool whiteMove;
    int indexHistory;
    int historySize = 100;

    Pos() {
      myPieces = new pieceType[64];
      ennemyPieces = new pieceType[64];
      for (int i=0; i<64; i++){
        myPieces[i] = EMPTY;
        ennemyPieces[i] = EMPTY;
      }
      currentPos = new SavePos();
      myPiecesMask = 0;
      ennemyPiecesMask = 0;
      myPawnsMask = 0;
      ennemyPawnsMask = 0;
      myKingPos = 0;
      ennemyKingPos = 0;
      whiteMove = true;
      indexHistory = 0;
      history = new SavePos*[historySize];
    }
    Pos(const Pos& other) {
      // Perform a deep copy of the data members
      this->myPawnsMask = other.myPawnsMask;
      this->ennemyPawnsMask = other.ennemyPawnsMask;
      this->myKingPos = other.myKingPos;
      this->ennemyKingPos = other.ennemyKingPos;
      this->whiteMove = other.whiteMove;
      this->myPiecesMask = other.myPiecesMask;
      this->ennemyPiecesMask = other.ennemyPiecesMask;
      this->currentPos = new SavePos(*other.currentPos);
      this->indexHistory = other.indexHistory;

      this->myPieces = new pieceType[64];
      std::memcpy(this->myPieces, other.myPieces, 64 * sizeof(pieceType));
      this->ennemyPieces = new pieceType[64];
      std::memcpy(this->ennemyPieces, other.ennemyPieces, 64 * sizeof(pieceType));

      //copy history
      this->history = new SavePos*[historySize];
      for (int i=0; i<historySize; i++){
        this->history[i] = other.history[i];
      }

    }

    ~Pos() {
      delete[] this->myPieces;
      delete[] this->ennemyPieces;
      delete this->currentPos;
      delete[] this->history;
    }

    void pushHistory() {
      history[indexHistory] = new SavePos(*currentPos);
      indexHistory = (indexHistory + 1) % historySize;
    }
    void popHistory() {
      delete currentPos;
      indexHistory = (indexHistory - 1) % historySize;
      currentPos = history[indexHistory];
    }
    void freeHistory() {
      for(int i=0; i<indexHistory%historySize; i++) {
        delete history[i];
      }
    }

  };


  void printBitboard(uint64_t bitboard);
  uint64_t getMoves(Pos &pos, pieceType piece, int from, bool isWhite, bool getControl);
  Pos init(Pos &pos, std::string fen);
  void movePiece(Pos &pos, Move move);
  void undoMove(Pos& pos);
  void free();
  int getAllComb(Pos &pos, int initialDepth, int depth=0, const int nbThreads=1, bool multi=false, int mini=0, int maxi=Tools::MAX_LEGAL_MOVES);
  void printPos(Pos &pos);
  
}
}