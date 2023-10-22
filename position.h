#pragma once


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
  

  enum pieceType {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, EMPTY};

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

  uint64_t* getPiecesMovesMask(pieceType piece);
  uint64_t* getLineMask(bool isRank);
  uint64_t* getDiagMask(bool isUpRight);
  uint64_t** getTable(bool isRook, uint64_t* magicNbs, uint64_t* shifts);
  int getCol(int pos);
  int getRank(int pos);
  int getPos(int col, int rank);
  
  
}