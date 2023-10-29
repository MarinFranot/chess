#pragma once

#include <cstring>
#include <stack>

#include "typesdef.h"
#include "tools.h"

namespace Position {
  extern const int boardSize;
  extern const int EAST;
  extern const int WEST;
  extern const int NORTH;
  extern const int SOUTH;
  extern const int NORTH_EAST;
  extern const int SOUTH_EAST;
  extern const int NORTH_WEST;
  extern const int SOUTH_WEST;


  struct Move {
    uint64_t value;
    Move(uint64_t initialValue) {
      value = initialValue;
    }
    Move(int from, int to, bool capture, pieceType captured=EMPTY){
      value = from | (to << 6);
      value |= capture << 12;
      value |= static_cast<uint64_t>(captured) << 13;
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

    std::string toString(){
      std::string capture = isCapture() ? " x " : " ";
      return Tools::toSquare(getFrom()) + capture + Tools::toSquare(getTo());
    }
  };

  struct SavePos {
    uint64_t myControl;
    uint64_t ennemyControl;
    uint64_t pins;
    uint64_t* pinsMasks;
    Move* legalMoves;
    bool possible00[2];
    bool possible000[2];
    int enPassantPossible;
    int halfMovesSinceReset;
    Move currentMove;
    bool check;
    int checkers[2];
    uint64_t blockCheck;
    
    SavePos(Move move, uint64_t myControl, uint64_t ennemyControl, uint64_t pins, uint64_t* pinsMasks, Move* legalMoves, bool possible00[2], bool possible000[2], int enPassantPossible, int halfMovesSinceReset,
          bool check, int checkers[2], uint64_t blockCheck){
      this->myControl = myControl;
      this->ennemyControl = ennemyControl;
      this->pins = pins;
      this->possible00[0] = possible00[0];
      this->possible00[1] = possible00[1];
      this->possible000[0] = possible000[0];
      this->possible000[1] = possible000[1];
      this->enPassantPossible = enPassantPossible;
      this->halfMovesSinceReset = halfMovesSinceReset;
      this->currentMove = move;
      this->check = check;
      this->checkers[0] = checkers[0];
      this->checkers[1] = checkers[1];
      this->blockCheck = blockCheck;
      
      //copy legalMoves array
      this->legalMoves = new Move[Tools::MAX_LEGAL_MOVES];
      std::memcpy(this->legalMoves, legalMoves, Tools::MAX_LEGAL_MOVES*sizeof(Move));
      this->pinsMasks = new uint64_t[64];
      std::memcpy(this->pinsMasks, pinsMasks, 64*sizeof(uint64_t));
    }
    SavePos() {
      this->legalMoves = new Move[Tools::MAX_LEGAL_MOVES];
      this->pinsMasks = new uint64_t[64];
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
      this->enPassantPossible = other.enPassantPossible;
      this->halfMovesSinceReset = other.halfMovesSinceReset;
      this->currentMove = other.currentMove;
      
      // Allocate new memory and copy data for legalMoves and pinsMasks
      this->legalMoves = new Move[Tools::MAX_LEGAL_MOVES];
      std::memcpy(this->legalMoves, other.legalMoves, Tools::MAX_LEGAL_MOVES * sizeof(Move));
      this->pinsMasks = new uint64_t[64];
      std::memcpy(this->pinsMasks, other.pinsMasks, 64 * sizeof(uint64_t));
    }


    ~SavePos() {
      if (this->legalMoves != NULL) {
        delete[] this->legalMoves;
      }
      if (this->pinsMasks != NULL) {
        delete[] this->pinsMasks;
      }

    }
  };

  void printBitboard(uint64_t bitboard);
  uint64_t getMoves(pieceType piece, int pos, bool isWhite, bool getControl);
  void init(std::string fen);
  void movePiece(Move move);
  void free();
  int getAllComb(int initialDepth, int depth=0, bool print=false);
  
}