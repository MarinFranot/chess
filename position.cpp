#include <iostream>
#include <string>

#include "position.h"




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

  uint64_t* bishopMagicNbs = new uint64_t[64];
  uint64_t* rookMagicNbs = new uint64_t[64];
  int* bishopShifts = new int[64];
  int* rookShifts = new int[64];

  uint64_t* pawnMoves = Tools::getPiecesMovesMask(PAWN);
  uint64_t* knightMoves = Tools::getPiecesMovesMask(KNIGHT);
  uint64_t** bishopMoves = Tools::getTable(false, bishopMagicNbs, bishopShifts);
  uint64_t** rookMoves = Tools::getTable(true, rookMagicNbs, rookShifts);
  uint64_t* kingMoves = Tools::getPiecesMovesMask(KING);

  uint64_t* rankMasks = Tools::getLineMasks(true);
  uint64_t* colMasks = Tools::getLineMasks(false);
  uint64_t* diagNEMasks = Tools::getDiagMasks(true);
  uint64_t* diagSEMasks = Tools::getDiagMasks(false);

  bool whiteMove;
  pieceType* myPieces = new pieceType[64];
  pieceType* ennemyPieces = new pieceType[64];
  uint64_t myPiecesMask = 0;
  uint64_t ennemyPiecesMask = 0;
  uint64_t myPawnsMask = 0;
  uint64_t ennemyPawnsMask = 0;
  uint64_t myControl = 0;
  uint64_t ennemyControl = 0;

  uint64_t pins;
  uint64_t* pinsMasks = new uint64_t[64];
  int myKingPos;
  int ennemyKingPos;
  Move* legalMoves = new Move[Tools::MAX_LEGAL_MOVES];
  int halfMovesSinceReset = 0;
  int moveCounter = 0;
  int enPassantPossible = -1;
  SaveList* previousPos = new SaveList();

  bool possible00[2] = {false, false}; //black, white
  bool possible000[2] = {false, false};

  uint64_t getNEDiag(int pos){
    return diagNEMasks[14-(pos/8-pos%8+7)];
  }
  uint64_t getSEDiag(int pos){
    return diagSEMasks[pos/8+pos%8];
  }


  uint64_t lookupBishop(int pos){
    uint64_t mask = (myPiecesMask|ennemyPiecesMask) & (getNEDiag(pos)|getSEDiag(pos)) & ~(1ULL<<pos);
    return bishopMoves[pos][(mask*bishopMagicNbs[pos])>>bishopShifts[pos]];
  }
  uint64_t lookupRook(int pos){
    uint64_t mask = (myPiecesMask|ennemyPiecesMask) & (rankMasks[pos%8]|colMasks[pos/8]) & ~(1ULL<<pos);
    return rookMoves[pos][(mask*rookMagicNbs[pos])>>rookShifts[pos]];
  }
  uint64_t lookupQueen(int pos){
    return lookupBishop(pos) | lookupRook(pos);
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
        uint64_t mask;
        bool isRank = false;
        if (dir == NORTH || dir == EAST){
          mask = dir==NORTH ? lookupRook(index) & colMasks[index%8] : lookupRook(index) & rankMasks[index/8];
          isRank = true;
        } else if (dir == NORTH_EAST || dir == NORTH_WEST){
          mask = dir==NORTH_EAST ? lookupBishop(index) & getNEDiag(index) : lookupBishop(index) & getSEDiag(index);
        }

        uint64_t originalMask = mask;
        while(mask){
          int indexSearsh = Tools::getFirstBitIndex(mask);
          mask &= (mask - 1);
          pieceType pinner = myPieces[indexSearsh];
          if (pinner==QUEEN || (pinner==ROOK && isRank) || (pinner==BISHOP && !isRank)){
            pins |= 1ULL << indexSearsh;
            pinsMasks[indexSearsh] = originalMask;
            break;
          }
        }
      }
    }
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
      res = getControl ? kingMoves[pos] : kingMoves[pos] & ~(myPiecesMask|ennemyControl);
    }
    return res;
  }


  void updateAllMoves(){
    myControl = 0;
    int index = 0;
    uint64_t mask = myPiecesMask;
    std::cout << "myPiecesMask : " << myPiecesMask << std::endl;
    while(mask){
      int from = Tools::getFirstBitIndex(mask);
      mask &= (mask - 1);
      pieceType piece = myPieces[from];
      uint64_t moves = getMoves(piece, from, whiteMove, false);
      myControl |= getMoves(piece, from, whiteMove, true);
      while(moves){
        int to = Tools::getFirstBitIndex(moves);
        moves &= (moves - 1);
        bool capture = (ennemyPiecesMask >> to) & 1ULL;
        pieceType captured = capture ? ennemyPieces[to] : EMPTY;
        legalMoves[index] = Move(from, to, capture, captured);
        index++;
      }
    }
    legalMoves[index] = Move();
  }

  void init(std::string fen){
    long unsigned int index = 0;
    int indexPos = 56;

    for (int i=0; i<64; i++){
      myPieces[i] = EMPTY;
      ennemyPieces[i] = EMPTY;
      pinsMasks[i] = 0;
    }

    pieceType* pieceMap = new pieceType[26];
    pieceMap['p'-'a'] = pieceType(PAWN);
    pieceMap['n'-'a'] = pieceType(KNIGHT);
    pieceMap['b'-'a'] = pieceType(BISHOP);
    pieceMap['r'-'a'] = pieceType(ROOK);
    pieceMap['q'-'a'] = pieceType(QUEEN);
    pieceMap['k'-'a'] = pieceType(KING);

    std::string board = fen.substr(0, fen.find(" "));
    while (index < board.length()){
      char c = fen[index];
      if (c == '/') {
        indexPos -= 16;
      } else if ('0' <= c && c <= '9') {
        int nb = c - '0';
        indexPos += nb;
      } else if (('a'<=c && c<='z') || ('A'<=c && c<='Z')){
        bool isPieceWhite = 'A'<=c && c<='Z';
        c = isPieceWhite ? (char)(c + 'a' - 'A') : c;
        pieceType piece = pieceMap[c-'a'];

        if (isPieceWhite) {
          myPieces[indexPos] = piece;
          myPiecesMask |= 1ULL << indexPos;
          if (piece == KING) {
            myKingPos = indexPos;
          } else if (piece == PAWN) {
            myPawnsMask |= 1ULL << indexPos;
          }
        } else {
          ennemyPieces[indexPos] = piece;
          ennemyPiecesMask |= 1ULL << indexPos;
          if (piece == KING) {
            ennemyKingPos = indexPos;
          } else if (piece == PAWN) {
            ennemyPawnsMask |= 1ULL << indexPos;
          }
        }
        indexPos++;
      }
      index++;
    }

    whiteMove = fen[index+1] == 'w';
    index += 3;
    while(true){
      if (fen[index] == 'K') {
        possible00[1] = true;
      } else if (fen[index] == 'Q') {
        possible000[1] = true;
      } else if (fen[index] == 'k') {
        possible00[0] = true;
      } else if (fen[index] == 'q') {
        possible000[0] = true;
      } else if (fen[index] == '-') {
        index++;
        break;
      } else {
        break;
      }
      index++;
    }

    index++;
    if (fen[index] == '-'){
      enPassantPossible = -1;
    } else {
      enPassantPossible = Tools::toIndexPos(fen.substr(index, index+1));
      index++;
    }
    std::string movesCountString = fen.substr(index+2, fen.length());
    std::string halfCounterString = movesCountString.substr(0, movesCountString.find(" "));
    halfMovesSinceReset = std::stoi(halfCounterString);

  
    std::string moveCounterString = movesCountString.substr(movesCountString.find(" ")+1, movesCountString.length());
    moveCounter = std::stoi(moveCounterString);

    updatePins();
    updateAllMoves();

    delete[] pieceMap;
  }

  void free(){
    delete[] bishopMagicNbs;
    delete[] rookMagicNbs;
    delete[] bishopShifts;
    delete[] rookShifts;
    delete[] pawnMoves;
    delete[] knightMoves;
    delete[] kingMoves;
    delete[] rankMasks;
    delete[] colMasks;
    delete[] diagNEMasks;
    delete[] diagSEMasks;
    delete[] pinsMasks;
    delete[] legalMoves;
    for (int i=0; i<64; i++){
      delete[] bishopMoves[i];
      delete[] rookMoves[i];
    }
    delete[] bishopMoves;
    delete[] rookMoves;
    delete[] previousPos;
    delete[] myPieces;
    delete[] ennemyPieces;
  }


  void swap(){
    whiteMove = !whiteMove;
    std::swap(myPieces, ennemyPieces);
    std::swap(myPiecesMask, ennemyPiecesMask);
    std::swap(myPawnsMask, ennemyPawnsMask);
    std::swap(myControl, ennemyControl);
    std::swap(myKingPos, ennemyKingPos);
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

    SavePos current = SavePos(move, myControl, ennemyControl, pins, pinsMasks, legalMoves, possible00, possible000, enPassantPossible, halfMovesSinceReset);
    previousPos->add(current);

    swap();

    //update control / possible moves
    updateAllMoves();
  }

  
  void undoMove() {
    swap();

    SavePos current = previousPos->pop();
    myControl = current.myControl;
    ennemyControl = current.ennemyControl;
    pins = current.pins;
    pinsMasks = current.pinsMasks;
    legalMoves = current.legalMoves;
    possible00[0] = current.possible00[0];
    possible00[1] = current.possible00[1];
    possible000[0] = current.possible000[0];
    possible000[1] = current.possible000[1];
    enPassantPossible = current.enPassantPossible;
    halfMovesSinceReset = current.halfMovesSinceReset;

    Move move = current.currentMove;
    int from = move.getFrom();
    int to = move.getTo();
    myPieces[from] = myPieces[to];
    myPieces[to] = EMPTY;
    myPiecesMask = (myPiecesMask & ~(1ULL<<to)) | (1ULL<<from);
    if (myPieces[from] == PAWN){
      myPawnsMask = (myPawnsMask & ~(1ULL<<to)) | (1ULL<<from);
    } else if (myPieces[from] == KING) {
      myKingPos = from;
    }

    pieceType captured = move.getCapturedPiece();
    ennemyPieces[to] = captured;
    bool capture = move.isCapture();
    ennemyPiecesMask = (ennemyPiecesMask & ~(1ULL<<to)) | (capture << to);
    if (capture && captured == PAWN) {
      ennemyPawnsMask |= 1ULL<<to;
    }
  }

  int getAllComb(int depth) {
    if (depth==0) {
      return 1;
    } else {
      int res = 0;
      int index = 0;
      Move move = legalMoves[index];
      while (move.value != 0) {
        std::cout << "value : " << move.value << std::endl;
        movePiece(move);
        res += getAllComb(depth - 1);

        index++;
        move = legalMoves[index];
      }
      return res;
    }
  }
}


