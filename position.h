#pragma once

#include <cstring>

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
    
    SavePos(Move move, uint64_t myControl, uint64_t ennemyControl, uint64_t pins, uint64_t* pinsMasks, Move* legalMoves, bool possible00[2], bool possible000[2], int enPassantPossible, int halfMovesSinceReset){
      this->myControl = myControl;
      this->ennemyControl = ennemyControl;
      this->pins = pins;
      this->pinsMasks = pinsMasks;
      this->possible00[0] = possible00[0];
      this->possible00[1] = possible00[1];
      this->possible000[0] = possible000[0];
      this->possible000[1] = possible000[1];
      this->enPassantPossible = enPassantPossible;
      this->halfMovesSinceReset = halfMovesSinceReset;
      this->currentMove = move;
      //copy legalMoves array
      this->legalMoves = new Move[Tools::MAX_LEGAL_MOVES];
      std::memcpy(this->legalMoves, legalMoves, Tools::MAX_LEGAL_MOVES*sizeof(Move));
    }
    SavePos(){}
  };

  struct SaveList {
    SavePos save;
    SaveList* next;

    SaveList(SavePos position) {
      this->save = position;
      this->next = NULL;
    }
    SaveList() {
      this->next = NULL;
    }

    void add(SavePos position) {
      this->next = this;
      this->save = position;
    }

    SavePos pop() {
      SavePos res = this->save;
      this->save = this->next->save;
      this->next = this->next->next;
      //delete this->next;
      return res;
    }
  };


  void init(std::string fen);
  void movePiece(Move move);
  void free();
  int getAllComb(int depth);
  
}