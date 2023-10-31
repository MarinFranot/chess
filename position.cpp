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
  uint64_t corners = 0x8100000000000081;

  uint64_t* rankMasks = Tools::getLineMasks(true);
  uint64_t* colMasks = Tools::getLineMasks(false);
  uint64_t* diagNEMasks = Tools::getDiagMasks(true);
  uint64_t* diagSEMasks = Tools::getDiagMasks(false);

  uint64_t* bishopMagicNbs = new uint64_t[64];
  uint64_t* rookMagicNbs = new uint64_t[64];
  int* bishopShifts = new int[64];
  int* rookShifts = new int[64];

  uint64_t* pawnMoves[2] = {Tools::getPiecesMovesMask(PAWN, false), Tools::getPiecesMovesMask(PAWN, true)};
  uint64_t* knightMoves = Tools::getPiecesMovesMask(KNIGHT);
  uint64_t** bishopMoves = Tools::getTable(false, bishopMagicNbs, bishopShifts);
  uint64_t** rookMoves = Tools::getTable(true, rookMagicNbs, rookShifts);
  uint64_t* kingMoves = Tools::getPiecesMovesMask(KING);

  bool whiteMove = true;
  pieceType* myPieces = new pieceType[64];
  pieceType* ennemyPieces = new pieceType[64];
  uint64_t myPiecesMask = 0;
  uint64_t ennemyPiecesMask = 0;
  uint64_t myPawnsMask = 0;
  uint64_t ennemyPawnsMask = 0;
  uint64_t myControl = 0;
  uint64_t ennemyControl = 0;

  uint64_t pins = 0;
  uint64_t* pinsMasks = new uint64_t[64];
  int myKingPos = 0;
  int ennemyKingPos = 0;
  Move* legalMoves = new Move[Tools::MAX_LEGAL_MOVES];
  int halfMovesSinceReset = 0;
  int moveCounter = 0;
  int enPassant = -1;
  std::stack<SavePos> previousPos;
  bool check = false;
  int checkers[2];
  uint64_t blockCheck = UINT64_MAX;

  bool possible00[2] = {false, false}; //black, white
  bool possible000[2];// = {false, false};

  void printBitboard(uint64_t b) {
    for (int i=0; i<8; i++){
      for (int j=0; j<8; j++){
        std::cout << ((b>>(i*8+j))&1ULL);
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;

  }

  uint64_t getNEDiag(int pos){
    return diagNEMasks[14-(pos/8-pos%8+7)];
  }
  uint64_t getSEDiag(int pos){
    uint64_t res = diagSEMasks[pos/8+pos%8];
    return res;
  }
  uint64_t getRankMask(int pos){
    return rankMasks[pos/8];
  }
  uint64_t getColMask(int pos){
    return colMasks[pos%8];
  }

  bool getWhiteMove() {
    return whiteMove;
  }
  Move* getLegalMoves() {
    return legalMoves;
  }
  bool getCheck() {
    return check;
  }
  pieceType* getMyPieces() {
    return myPieces;
  }
  pieceType* getEnnemyPieces() {
    return ennemyPieces;
  }


  uint64_t lookupBishop(int pos){
    uint64_t mask = (myPiecesMask|(ennemyPiecesMask &~(1ULL<<ennemyKingPos))) & (diagNEMasks[14-(pos/8-pos%8+7)]|diagSEMasks[pos/8+pos%8]) & ~(1ULL<<pos) & ~rim;
    uint64_t index = (mask*bishopMagicNbs[pos])>>(bishopShifts[pos]);
    return bishopMoves[pos][index];
  }
  uint64_t lookupRook(int pos){
    uint64_t restrict = rim & ~((getRankMask(63) & -static_cast<uint64_t>(pos/8==7)) | (getRankMask(0) & -static_cast<uint64_t>(pos/8==0)) |
     (getColMask(7) & -static_cast<uint64_t>(pos%8==7)) | (getColMask(0) & -static_cast<uint64_t>(pos%8==0))) ;
    uint64_t mask = (myPiecesMask|(ennemyPiecesMask &~(1ULL<<ennemyKingPos))) & (getRankMask(pos)|getColMask(pos)) & ~(1ULL<<pos) & ~(restrict | corners);	
    return rookMoves[pos][(mask*rookMagicNbs[pos])>>rookShifts[pos]];
  }
  uint64_t lookupQueen(int pos){
    return lookupBishop(pos) | lookupRook(pos);
  }

  //update pins and discover checks
  void updatePins(){
    pins = 0;
    uint64_t queen = lookupQueen(ennemyKingPos);
    while(queen){
      int index = Tools::getLastBitIndex(queen);
      queen &= (queen - 1);
      pieceType pinned = ennemyPieces[index];
      if (pinned != EMPTY){
        int dir = abs(Tools::getDir(ennemyKingPos, index));
        uint64_t mask=0;
        bool isRank = false;
        if (dir == NORTH || dir == EAST){
          mask = dir==NORTH ? lookupRook(index) & getColMask(index) : lookupRook(index) & getRankMask(index);
          isRank = true;
        } else if (dir == NORTH_EAST || dir == NORTH_WEST){
          mask = dir==NORTH_EAST ? lookupBishop(index) & getNEDiag(index) : lookupBishop(index) & getSEDiag(index);
        }

        uint64_t originalMask = mask;
        while(mask!=0){
          int indexSearsh = Tools::getLastBitIndex(mask);
          mask &= (mask - 1);
          pieceType pinner = myPieces[indexSearsh];
          if (pinner==QUEEN || (pinner==ROOK && isRank) || (pinner==BISHOP && !isRank)){
            pins |= 1ULL << index;
            pinsMasks[index] = originalMask;
            break;
          }
        }
      }
      //update checks
      //std::cout << check << checkers[0] << checkers[1] << std::endl;
      if (myPiecesMask & (1ULL<<index) && index!=checkers[0]){
        pieceType piece = myPieces[index];
        int dir = abs(Tools::getDir(ennemyKingPos, index));
        if (dir == NORTH || dir == EAST){
          if (piece == ROOK || piece == QUEEN){
            checkers[check] = index;
            check = true;
          }
        } else if (dir == NORTH_EAST || dir == NORTH_WEST){
          if (piece == BISHOP || piece == QUEEN){
            checkers[check] = index;
            check = true;
          }
        }
      }
    }
  }

  void updateChecks() {
    blockCheck = 0;
    if (check) {
      if (checkers[1] == -1) {
        int target = checkers[0];
        
        if (myPieces[target]==ROOK || myPieces[target]==QUEEN || myPieces[target]==BISHOP) {
          int dir = Tools::getDir(target, ennemyKingPos);
          //std::cout << "dir : " << dir << " King : " << ennemyKingPos << " target " << target << std::endl;
          for (int i=target; i!=ennemyKingPos; i+=dir) {
            blockCheck |= 1ULL<<i;
          }
        } else {
          blockCheck |= 1ULL<<target;
        }
      }
    } else {
      blockCheck = UINT64_MAX;
    }
  }

  void swap(){
    whiteMove = !whiteMove;
    std::swap(myPieces, ennemyPieces);
    std::swap(myPiecesMask, ennemyPiecesMask);
    std::swap(myPawnsMask, ennemyPawnsMask);
    std::swap(myControl, ennemyControl);
    std::swap(myKingPos, ennemyKingPos);
  }

  
  uint64_t getMoves(pieceType piece, int pos, bool isWhite, bool getControl){
    uint64_t res = 0;
    uint64_t pin = ((pins>>pos) & 1ULL) ? pinsMasks[pos] : UINT64_MAX;
    if (piece == PAWN){
      int dir = whiteMove ? 1 : -1;
      if (getControl){
        res = pawnMoves[whiteMove][pos] & ~getColMask(pos);
      } else {
        uint64_t col = getColMask(pos);
        res = pawnMoves[whiteMove][pos] & ~((myPiecesMask | ennemyPiecesMask) & col);
        uint64_t up2 = 1ULL<<(pos+16*dir);
        if ((pos/8!=1) & whiteMove || (pos/8!=6) & !whiteMove) {
          res &= ~up2;
        } else if (((myPiecesMask | ennemyPiecesMask)>>(pos+8*dir))&1ULL){
          res &= ~up2;
        }
        bool passant = enPassant >= 0;
        res = ((res & col) | (res & (ennemyPiecesMask| ((1ULL<<enPassant)*passant)))) & blockCheck & pin;
      }
      
    } else if (piece == KNIGHT){
      res = getControl ? knightMoves[pos] : knightMoves[pos] & ~myPiecesMask & blockCheck &pin;
    } else if (piece == BISHOP){
      res = getControl ? lookupBishop(pos) : lookupBishop(pos) & ~myPiecesMask & blockCheck &pin;
    } else if (piece == ROOK){
      res = getControl ? lookupRook(pos) : lookupRook(pos) & ~myPiecesMask & blockCheck &pin;
    } else if (piece == QUEEN){
      res = getControl ? lookupQueen(pos) : lookupQueen(pos) & ~myPiecesMask & blockCheck &pin;
    } else if (piece == KING){
      if (getControl) {
        res = kingMoves[pos];
      } else {
        res = kingMoves[pos] & ~(myPiecesMask|ennemyControl);
        if (possible00[whiteMove]) {
          uint64_t mask00 = whiteMove ? 0x60ULL : 0x6000000000000000ULL;
          if (!check && !((myPiecesMask | ennemyPiecesMask | ennemyControl) & mask00)) {
            res |= 1ULL<<(pos+2);
          }
        }
        if (possible000[whiteMove]){
          uint64_t mask000 = whiteMove ? 0xCULL : 0xC00000000000000ULL;
          uint64_t blockerMask = (myPiecesMask | ennemyPiecesMask) & (mask000 | (1ULL << (pos-3)));
          if (!check && !(ennemyControl & mask000) && !blockerMask) {
            res |= 1ULL << (pos-2);
          }
        }
      }
    }
    return res;
  }


  void updateAllMoves(){
    int index = 0;
    uint64_t mask = myPiecesMask;
    while(mask){
      int from = Tools::getLastBitIndex(mask);
      mask &= (mask - 1);
      pieceType piece = myPieces[from];
      uint64_t moves = getMoves(piece, from, whiteMove, false);
      //myControl |= getMoves(piece, from, whiteMove, true);
      while(moves){
        int to = Tools::getLastBitIndex(moves);
        moves &= (moves - 1);
        bool capture = (ennemyPiecesMask >> to) & 1ULL;
        
        pieceType captured = ennemyPieces[to];
        if (piece==PAWN) {
          if (to == enPassant) {
            legalMoves[index] = Move(from, to, true, PAWN, true);
          } else if (to/8==0 || to/8==7) {
            legalMoves[index] = Move(from, to, capture, captured, false, false, false, true, QUEEN);
            legalMoves[index+1] = Move(from, to, capture, captured, false, false, false, true, ROOK);
            legalMoves[index+2] = Move(from, to, capture, captured, false, false, false, true, BISHOP);
            legalMoves[index+3] = Move(from, to, capture, captured, false, false, false, true, KNIGHT);
            index += 3;
          } else {
            legalMoves[index] = Move(from, to, capture, captured);
          }
        } else if (piece==KING && abs(from-to)==2) {
          bool is00 = to>from;
          legalMoves[index] = Move(from, to, false, EMPTY, false, is00, !is00);
        } else {
          legalMoves[index] = Move(from, to, capture, captured);
        }
        index++;
      }
    }
    legalMoves[index] = Move();
  }

  void updateControl() {
    myControl = 0;
    uint64_t mask = myPiecesMask;
    while(mask){
      int from = Tools::getLastBitIndex(mask);
      mask &= (mask - 1);
      pieceType piece = myPieces[from];
      myControl |= getMoves(piece, from, whiteMove, true);
    }
  }


  void init(std::string fen){
    long unsigned int index = 0;
    int indexPos = 56;

    for (int i=0; i<64; i++){
      myPieces[i] = EMPTY;
      ennemyPieces[i] = EMPTY;
      pinsMasks[i] = 0;
    }
    checkers[0] = 0;
    checkers[1] = 1;

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
    if (!whiteMove) {
      swap();
      whiteMove = false;
    }
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
      enPassant = -1;
    } else {
      enPassant = Tools::toIndexPos(fen.substr(index, index+1));
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
    delete[] pawnMoves[0];
    delete[] pawnMoves[1];
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
    delete[] myPieces;
    delete[] ennemyPieces;
  }

  void updateCastling(int fromRook, int toRook) {
    myPieces[toRook] = myPieces[fromRook];
    myPieces[fromRook] = EMPTY;
    myPiecesMask = (myPiecesMask & ~(1ULL<<fromRook)) | (1ULL<<toRook);
  }

  void movePiece(Move move){
    auto removeCastling = [](int posRook, bool white) {
      if (posRook%8 == 0) {
        possible000[white] = false;
      } else if (posRook%8 == 7) {
        possible00[white] = false;
      }
    };

    SavePos current = SavePos(move, myControl, ennemyControl, pins, pinsMasks, legalMoves, possible00, possible000, enPassant, halfMovesSinceReset, check, checkers, blockCheck);
    previousPos.push(current);
    
    int from = move.getFrom();
    int to = move.getTo();
    
    pieceType piece = myPieces[from];
    myPieces[to] = piece;
    myPieces[from] = EMPTY;
    myPiecesMask = (myPiecesMask & ~(1ULL<<from)) | (1ULL<<to);
    enPassant = -1;
    if (piece == PAWN){
      if (move.isPromotion()) {
        pieceType promotion = move.getPromotion();
        myPieces[to] = promotion;
        myPawnsMask &= ~(1ULL<<from);
      } else {
        myPawnsMask = (myPawnsMask & ~(1ULL<<from)) | (1ULL<<to);
        int dir = whiteMove ? 1 : -1;
        enPassant = (abs(to-from) == 16) ? from+8*dir : -1;
      }
    } else if (piece == KING) {
      myKingPos = to;
      possible000[whiteMove] = false;
      possible00[whiteMove] = false;

      if (move.is00()) {
        updateCastling(from+3, to-1);
      } else if (move.is000()) {
        updateCastling(from-4, to+1);
      }
    } else if (piece == ROOK) {
      removeCastling(from, whiteMove);
    }

    if (move.isCapture()){
      int pop = move.isEnPassant() ? (to + (whiteMove ? -8 : 8)) : to;
      ennemyPieces[pop] = EMPTY;
      ennemyPiecesMask &= ~(1ULL<<pop);
      pieceType captured = move.getCapturedPiece();
      if (captured == PAWN){
        ennemyPawnsMask &= ~(1ULL<<pop);
      } else if (captured == ROOK) {
        
        removeCastling(to, !whiteMove);
      }
    }

    uint64_t control = getMoves(piece, to, whiteMove, true);
    check = control & (1ULL<<ennemyKingPos);
    checkers[1] = -1;
    if (check){
      checkers[0] = to;
    }

    updateControl();
    updatePins();
    updateChecks();
    swap();
    updateAllMoves();
  }

  
  void undoMove() {
    swap();
    
    SavePos current = previousPos.top();
    previousPos.pop();

    myControl = current.myControl;
    ennemyControl = current.ennemyControl;
    pins = current.pins;
    
    std::memcpy(pinsMasks, current.pinsMasks, 64*sizeof(uint64_t));
    std::memcpy(legalMoves, current.legalMoves, Tools::MAX_LEGAL_MOVES*sizeof(Move));
    possible00[0] = current.possible00[0];
    possible00[1] = current.possible00[1];
    possible000[0] = current.possible000[0];
    possible000[1] = current.possible000[1];
    enPassant = current.enPassant;
    halfMovesSinceReset = current.halfMovesSinceReset;
    check = current.check;
    checkers[0] = current.checkers[0];
    checkers[1] = current.checkers[1];
    
    Move move = current.currentMove;
    int from = move.getFrom();
    int to = move.getTo();
    if (move.isPromotion()) {
      myPieces[from] = PAWN;
    } else {
      myPieces[from] = myPieces[to];
    }
    myPieces[to] = EMPTY;
    myPiecesMask = (myPiecesMask & ~(1ULL<<to)) | (1ULL<<from);
    if (myPieces[from] == PAWN){
      myPawnsMask = (myPawnsMask & ~(1ULL<<to)) | (1ULL<<from);
    } else if (myPieces[from] == KING) {
      myKingPos = from;
    }

    if (move.is00()) {
      updateCastling(to-1, from+3);
    } else if (move.is000()) {
      updateCastling(to+1, from-4);
    }

    pieceType captured = move.getCapturedPiece();
    int pop = move.isEnPassant() ? (to + (whiteMove ? -8 : 8)) : to;
    ennemyPieces[pop] = captured;
    bool capture = move.isCapture();
    if (capture) {
      ennemyPiecesMask |= 1ULL<<pop;
      if (captured == PAWN) {
        ennemyPawnsMask |= 1ULL<<pop;
      }
    }
    //delete &current;
  }

  void printPos() {
    std::cout << "White move : " << whiteMove << std::endl;
    std::cout << "Check : " << check << std::endl;
    std::cout << "000 : " << possible000[0] << " " << possible000[1] << std::endl;

    printBitboard(myPiecesMask);
    printBitboard(ennemyPiecesMask);
  }

  int getAllComb(int initialDepth, int depth, bool print) {
    if (depth==0) {
      return 1;
    } else {      
      int res = 0;
      int index = 0;
      Move move = legalMoves[index];
      while (move.value != 0) {
        bool found = false;

        movePiece(move);

        int add = getAllComb(initialDepth, depth - 1, false);
        res += add;
        undoMove();

        if (found) {
          initialDepth++;
        }
        
        if (depth==initialDepth) {
          std::cout << move.toString() << " -> " << add << std::endl;
        }
        index++;
        move = legalMoves[index];
      }
      return res;
    }
  }
}


