#include <iostream>
#include <string>


#include "position.h"

namespace Chess {
namespace Position {
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

  uint64_t* rankMasks;// = Tools::getLineMasks(true);
  uint64_t* colMasks;// = Tools::getLineMasks(false);
  uint64_t* diagNEMasks;// = Tools::getDiagMasks(true);
  uint64_t* diagSEMasks;// = Tools::getDiagMasks(false);

  uint64_t* bishopMagicNbs = new uint64_t[64];
  uint64_t* rookMagicNbs = new uint64_t[64];
  int* bishopShifts = new int[64];
  int* rookShifts = new int[64];

  uint64_t* pawnMoves[2];// = {Tools::getPiecesMovesMask(PAWN, false), Tools::getPiecesMovesMask(PAWN, true)};
  uint64_t* knightMoves;// = Tools::getPiecesMovesMask(KNIGHT);
  uint64_t** bishopMoves;// = Tools::getTable(false, bishopMagicNbs, bishopShifts);
  uint64_t** rookMoves;// = Tools::getTable(true, rookMagicNbs, rookShifts);
  uint64_t* kingMoves;// = Tools::getPiecesMovesMask(KING);

  void createTables() {
    rankMasks = Tools::getLineMasks(true);
    colMasks = Tools::getLineMasks(false);
    diagNEMasks = Tools::getDiagMasks(true);
    diagSEMasks = Tools::getDiagMasks(false);

    pawnMoves[0] = Tools::getPiecesMovesMask(PAWN, false);
    pawnMoves[1] = Tools::getPiecesMovesMask(PAWN, true);
    knightMoves = Tools::getPiecesMovesMask(KNIGHT);
    kingMoves = Tools::getPiecesMovesMask(KING);

    bishopMoves = Tools::getTable(false, bishopMagicNbs, bishopShifts);
    rookMoves = Tools::getTable(true, rookMagicNbs, rookShifts);
  }

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


  uint64_t lookupBishop(const Pos& pos, int from){
    uint64_t mask = (pos.myPiecesMask|(pos.ennemyPiecesMask &~(1ULL<<pos.ennemyKingPos))) & (getNEDiag(from)|getSEDiag(from)) & ~(1ULL<<from) & ~rim;
    uint64_t index = (mask*bishopMagicNbs[from])>>(bishopShifts[from]);
    return bishopMoves[from][index];
  }
  uint64_t lookupRook(const Pos& pos, int from){
    uint64_t restrict = rim & ~((getRankMask(63) & -static_cast<uint64_t>(from/8==7)) | (getRankMask(0) & -static_cast<uint64_t>(from/8==0)) |
     (getColMask(7) & -static_cast<uint64_t>(from%8==7)) | (getColMask(0) & -static_cast<uint64_t>(from%8==0))) ;
    uint64_t mask = (pos.myPiecesMask|(pos.ennemyPiecesMask &~(1ULL<<pos.ennemyKingPos))) & (getRankMask(from)|getColMask(from)) & ~(1ULL<<from) & ~(restrict | corners);	
    uint64_t res = rookMoves[from][(mask*rookMagicNbs[from])>>rookShifts[from]];
    return res;
  }
  uint64_t lookupQueen(const Pos &pos, int from){
    //uint64_t res = lookupBishop(pos, from) | lookupRook(pos, from);
    uint64_t bishop = lookupBishop(pos, from);
    uint64_t rook = lookupRook(pos, from);
    uint64_t res = bishop | rook;
    return res;
  }

  //update pins and discover checks
  void updatePins(Pos &pos){
    pos.currentPos->pins = 0;
    uint64_t queen = lookupQueen(pos, pos.ennemyKingPos);
    while(queen){
      int index = Tools::getLastBitIndex(queen);
      queen &= (queen - 1);
      pieceType pinned = pos.ennemyPieces[index];
      if (pinned != EMPTY){
        int dir = abs(Tools::getDir(pos.ennemyKingPos, index));
        uint64_t mask=0;
        bool isRank = false;
        if (dir == NORTH || dir == EAST){
          mask = dir==NORTH ? lookupRook(pos, index) & getColMask(index) : lookupRook(pos, index) & getRankMask(index);
          isRank = true;
        } else if (dir == NORTH_EAST || dir == NORTH_WEST){
          mask = dir==NORTH_EAST ? lookupBishop(pos, index) & getNEDiag(index) : lookupBishop(pos, index) & getSEDiag(index);
        }

        uint64_t originalMask = mask;
        while(mask!=0){
          int indexSearsh = Tools::getLastBitIndex(mask);
          mask &= (mask - 1);
          pieceType pinner = pos.myPieces[indexSearsh];
          if (pinner==QUEEN || (pinner==ROOK && isRank) || (pinner==BISHOP && !isRank)){
            pos.currentPos->pins |= 1ULL << index;
            pos.currentPos->pinsMasks[index] = originalMask;
            break;
          }
        }
      }
      //update checks
      if (pos.myPiecesMask & (1ULL<<index) && index!=pos.currentPos->checkers[0]){
        pieceType piece = pos.myPieces[index];
        int dir = abs(Tools::getDir(pos.ennemyKingPos, index));
        if (dir == NORTH || dir == EAST){
          if (piece == ROOK || piece == QUEEN){
            pos.currentPos->checkers[pos.currentPos->check] = index;
            pos.currentPos->check = true;
          }
        } else if (dir == NORTH_EAST || dir == NORTH_WEST){
          if (piece == BISHOP || piece == QUEEN){
            pos.currentPos->checkers[pos.currentPos->check] = index;
            pos.currentPos->check = true;
          }
        }
      }
    }
  }

  void updateChecks(Pos &pos) {
    pos.currentPos->blockCheck = 0;
    if (pos.currentPos->check) {
      if (pos.currentPos->checkers[1] == -1) {
        int target = pos.currentPos->checkers[0];
        
        if (pos.myPieces[target]==ROOK || pos.myPieces[target]==QUEEN || pos.myPieces[target]==BISHOP) {
          int dir = Tools::getDir(target, pos.ennemyKingPos);
          //std::cout << "dir : " << dir << " King : " << ennemyKingPos << " target " << target << std::endl;
          for (int i=target; i!=pos.ennemyKingPos; i+=dir) {
            pos.currentPos->blockCheck |= 1ULL<<i;
          }
        } else {
          pos.currentPos->blockCheck |= 1ULL<<target;
        }
      }
    } else {
      pos.currentPos->blockCheck = UINT64_MAX;
    }
  }

  void swap(Pos &pos){
    pos.whiteMove = !pos.whiteMove;
    std::swap(pos.myPieces, pos.ennemyPieces);
    std::swap(pos.myPiecesMask, pos.ennemyPiecesMask);
    std::swap(pos.myPawnsMask, pos.ennemyPawnsMask);
    std::swap(pos.currentPos->myControl, pos.currentPos->ennemyControl);
    std::swap(pos.myKingPos, pos.ennemyKingPos);
  }

  
  uint64_t getMoves(Pos &pos, pieceType piece, int from, bool isWhite, bool getControl){
    SavePos* currentPos = pos.currentPos;
    uint64_t res = 0;
    uint64_t pin = ((currentPos->pins>>from) & 1ULL) ? currentPos->pinsMasks[from] : UINT64_MAX;
    if (piece == PAWN){
      int dir = pos.whiteMove ? 1 : -1;
      if (getControl){
        res = pawnMoves[pos.whiteMove][from] & ~getColMask(from);
      } else {
        uint64_t col = getColMask(from);
        res = pawnMoves[pos.whiteMove][from] & ~((pos.myPiecesMask | pos.ennemyPiecesMask) & col);
        uint64_t up2 = 1ULL<<(from+16*dir);
        if ((from/8!=1) & pos.whiteMove || (from/8!=6) & !pos.whiteMove) {
          res &= ~up2;
        } else if (((pos.myPiecesMask | pos.ennemyPiecesMask)>>(from+8*dir))&1ULL){
          res &= ~up2;
        }
        bool passant = currentPos->enPassant >= 0;
        res = ((res & col) | (res & (pos.ennemyPiecesMask| ((1ULL<<currentPos->enPassant)*passant)))) & currentPos->blockCheck & pin;
      }
      
    } else if (piece == KNIGHT){
      res = getControl ? knightMoves[from] : knightMoves[from] & ~pos.myPiecesMask & currentPos->blockCheck &pin;
    } else if (piece == BISHOP){
      res = getControl ? lookupBishop(pos, from) : lookupBishop(pos, from) & ~pos.myPiecesMask & currentPos->blockCheck &pin;
    } else if (piece == ROOK){
      res = getControl ? lookupRook(pos, from) : lookupRook(pos, from) & ~pos.myPiecesMask & currentPos->blockCheck &pin;
    } else if (piece == QUEEN){
      res = getControl ? lookupQueen(pos, from) : lookupQueen(pos, from) & ~pos.myPiecesMask & currentPos->blockCheck &pin;
    } else if (piece == KING){
      if (getControl) {
        res = kingMoves[from];
      } else {
        res = kingMoves[from] & ~(pos.myPiecesMask|currentPos->ennemyControl);
        if (currentPos->possible00[pos.whiteMove]) {
          uint64_t mask00 = pos.whiteMove ? 0x60ULL : 0x6000000000000000ULL;
          if (!currentPos->check && !((pos.myPiecesMask | pos.ennemyPiecesMask | currentPos->ennemyControl) & mask00)) {
            res |= 1ULL<<(from+2);
          }
        }
        if (currentPos->possible000[pos.whiteMove]){
          uint64_t mask000 = pos.whiteMove ? 0xCULL : 0xC00000000000000ULL;
          uint64_t blockerMask = (pos.myPiecesMask | pos.ennemyPiecesMask) & (mask000 | (1ULL << (from-3)));
          if (!currentPos->check && !(currentPos->ennemyControl & mask000) && !blockerMask) {
            res |= 1ULL << (from-2);
          }
        }
      }
    }
    return res;
  }


  void updateAllMoves(Pos &pos){
    int index = 0;
    SavePos* currentPos = pos.currentPos;
    uint64_t mask = pos.myPiecesMask;
    while(mask){
      int from = Tools::getLastBitIndex(mask);
      mask &= (mask - 1);
      pieceType piece = pos.myPieces[from];
      uint64_t moves = getMoves(pos, piece, from, pos.whiteMove, false);
      //myControl |= getMoves(piece, from, whiteMove, true);
      while(moves){
        int to = Tools::getLastBitIndex(moves);
        moves &= (moves - 1);
        bool capture = (pos.ennemyPiecesMask >> to) & 1ULL;
        
        pieceType captured = pos.ennemyPieces[to];
        if (piece==PAWN) {
          if (to == currentPos->enPassant) {
            currentPos->legalMoves[index] = Move(from, to, true, PAWN, true);
          } else if (to/8==0 || to/8==7) {
            currentPos->legalMoves[index] = Move(from, to, capture, captured, false, false, false, true, QUEEN);
            currentPos->legalMoves[index+1] = Move(from, to, capture, captured, false, false, false, true, ROOK);
            currentPos->legalMoves[index+2] = Move(from, to, capture, captured, false, false, false, true, BISHOP);
            currentPos->legalMoves[index+3] = Move(from, to, capture, captured, false, false, false, true, KNIGHT);
            index += 3;
          } else {
            currentPos->legalMoves[index] = Move(from, to, capture, captured);
          }
        } else if (piece==KING && abs(from-to)==2) {
          bool is00 = to>from;
          currentPos->legalMoves[index] = Move(from, to, false, EMPTY, false, is00, !is00);
        } else {
          currentPos->legalMoves[index] = Move(from, to, capture, captured);
        }
        index++;
      }
    }
    currentPos->nbLegalMoves = index;
    currentPos->legalMoves[index] = Move();
  }

  void updateControl(Pos &pos) {
    pos.currentPos->myControl = 0;
    uint64_t mask = pos.myPiecesMask;
    while(mask){
      int from = Tools::getLastBitIndex(mask);
      mask &= (mask - 1);
      pieceType piece = pos.myPieces[from];
      pos.currentPos->myControl |= getMoves(pos, piece, from, pos.whiteMove, true);
    }
  }


  Pos init(Pos &pos, std::string fen){
    long unsigned int index = 0;
    int indexPos = 56;
    SavePos* currentPos = pos.currentPos;
    pos.indexHistory = 0;


    for (int i=0; i<64; i++){
      pos.myPieces[i] = EMPTY;
      pos.ennemyPieces[i] = EMPTY;
      currentPos->pinsMasks[i] = 0;
    }
    currentPos->checkers[0] = 0;
    currentPos->checkers[1] = 1;

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
          pos.myPieces[indexPos] = piece;
          pos.myPiecesMask |= 1ULL << indexPos;
          if (piece == KING) {
            pos.myKingPos = indexPos;
          } else if (piece == PAWN) {
            pos.myPawnsMask |= 1ULL << indexPos;
          }
        } else {
          pos.ennemyPieces[indexPos] = piece;
          pos.ennemyPiecesMask |= 1ULL << indexPos;
          if (piece == KING) {
            pos.ennemyKingPos = indexPos;
          } else if (piece == PAWN) {
            pos.ennemyPawnsMask |= 1ULL << indexPos;
          }
        }
        indexPos++;
      }
      index++;

    }

    pos.whiteMove = fen[index+1] == 'w';
    if (!pos.whiteMove) {
      swap(pos);
      pos.whiteMove = false;
    }
    index += 3;
    while(true){
      if (fen[index] == 'K') {
        currentPos->possible00[1] = true;
      } else if (fen[index] == 'Q') {
        currentPos->possible000[1] = true;
      } else if (fen[index] == 'k') {
        currentPos->possible00[0] = true;
      } else if (fen[index] == 'q') {
        currentPos->possible000[0] = true;
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
      currentPos->enPassant = -1;
    } else {
      currentPos->enPassant = Tools::toIndexPos(fen.substr(index, index+1));
      index++;
    }
    std::string movesCountString = fen.substr(index+2, fen.length());
    std::string halfCounterString = movesCountString.substr(0, movesCountString.find(" "));
    currentPos->halfMovesSinceReset = std::stoi(halfCounterString);

  
    std::string moveCounterString = movesCountString.substr(movesCountString.find(" ")+1, movesCountString.length());
    //currentPos.moveCounter = std::stoi(moveCounterString);

    updatePins(pos);
    updateAllMoves(pos);

    delete[] pieceMap;
    return pos;
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
    //delete[] pinsMasks;
    //delete[] legalMoves;
    for (int i=0; i<64; i++){
      delete[] bishopMoves[i];
      delete[] rookMoves[i];
    }
    delete[] bishopMoves;
    delete[] rookMoves;
    //delete[] myPieces;
    //delete[] ennemyPieces;
  }

  void updateCastling(Pos &pos, int fromRook, int toRook) {
    pos.myPieces[toRook] = pos.myPieces[fromRook];
    pos.myPieces[fromRook] = EMPTY;
    pos.myPiecesMask = (pos.myPiecesMask & ~(1ULL<<fromRook)) | (1ULL<<toRook);
  }

  void movePiece(Pos &pos, Move move){
    auto removeCastling = [](Pos pos, int posRook, bool white) {
      if (posRook%8 == 0) {
        pos.currentPos->possible000[white] = false;
      } else if (posRook%8 == 7) {
        pos.currentPos->possible00[white] = false;
      }
    };

    pos.currentPos->currentMove = move;
    pos.pushHistory();
    SavePos* currentPos = pos.currentPos;

    int from = move.getFrom();
    int to = move.getTo();
    
    
    pieceType piece = pos.myPieces[from];
    pos.myPieces[to] = piece;
    pos.myPieces[from] = EMPTY;
    pos.myPiecesMask = (pos.myPiecesMask & ~(1ULL<<from)) | (1ULL<<to);
    currentPos->enPassant = -1;
    if (piece == PAWN){
      if (move.isPromotion()) {
        pieceType promotion = move.getPromotion();
        pos.myPieces[to] = promotion;
        pos.myPawnsMask &= ~(1ULL<<from);
      } else {
        pos.myPawnsMask = (pos.myPawnsMask & ~(1ULL<<from)) | (1ULL<<to);
        int dir = pos.whiteMove ? 1 : -1;
        currentPos->enPassant = (abs(to-from) == 16) ? from+8*dir : -1;
      }
    } else if (piece == KING) {
      pos.myKingPos = to;
      currentPos->possible000[pos.whiteMove] = false;
      currentPos->possible00[pos.whiteMove] = false;

      if (move.is00()) {
        updateCastling(pos, from+3, to-1);
      } else if (move.is000()) {
        updateCastling(pos, from-4, to+1);
      }
    } else if (piece == ROOK) {
      removeCastling(pos, from, pos.whiteMove);
    }

    if (move.isCapture()){
      int pop = move.isEnPassant() ? (to + (pos.whiteMove ? -8 : 8)) : to;
      pos.ennemyPieces[pop] = EMPTY;
      pos.ennemyPiecesMask &= ~(1ULL<<pop);
      pieceType captured = move.getCapturedPiece();
      if (captured == PAWN){
        pos.ennemyPawnsMask &= ~(1ULL<<pop);
      } else if (captured == ROOK) {
        
        removeCastling(pos, to, !pos.whiteMove);
      }
    }

    uint64_t control = getMoves(pos, piece, to, pos.whiteMove, true);
    currentPos->check = control & (1ULL<<pos.ennemyKingPos);
    currentPos->checkers[1] = -1;
    if (currentPos->check){
      currentPos->checkers[0] = to;
    }

    updateControl(pos);
    updatePins(pos);
    updateChecks(pos);
    swap(pos);
    updateAllMoves(pos);
  }

  
  void undoMove(Pos &pos) {
    swap(pos);
    
    pos.popHistory();

    
    Move move = pos.currentPos->currentMove;
    int from = move.getFrom();
    int to = move.getTo();
    if (move.isPromotion()) {
      pos.myPieces[from] = PAWN;
    } else {
      pos.myPieces[from] = pos.myPieces[to];
    }
    pos.myPieces[to] = EMPTY;
    pos.myPiecesMask = (pos.myPiecesMask & ~(1ULL<<to)) | (1ULL<<from);
    if (pos.myPieces[from] == PAWN){
      pos.myPawnsMask = (pos.myPawnsMask & ~(1ULL<<to)) | (1ULL<<from);
    } else if (pos.myPieces[from] == KING) {
      pos.myKingPos = from;
    }

    if (move.is00()) {
      updateCastling(pos, to-1, from+3);
    } else if (move.is000()) {
      updateCastling(pos, to+1, from-4);
    }

    pieceType captured = move.getCapturedPiece();
    int pop = move.isEnPassant() ? (to + (pos.whiteMove ? -8 : 8)) : to;
    pos.ennemyPieces[pop] = captured;
    bool capture = move.isCapture();
    if (capture) {
      pos.ennemyPiecesMask |= 1ULL<<pop;
      if (captured == PAWN) {
        pos.ennemyPawnsMask |= 1ULL<<pop;
      }
    }
  }

  void printPos(Pos pos) {
    std::cout << "White move : " << pos.whiteMove << std::endl;

    printBitboard(pos.myPiecesMask);
    printBitboard(pos.ennemyPiecesMask);
  }

  int getAllComb(Pos &pos, int initialDepth, int depth, const int nbThreads, int mini, int maxi) {
    if (nbThreads == 1) {
      if (depth==0) {
        return 1;
      } else if (depth == 1) {
        int res = 0;
        Move move = pos.currentPos->legalMoves[res];
        while (move.value != 0) {
          res++;
          move = pos.currentPos->legalMoves[res];
        }
        return res;
      }
      else {      
        int res = 0;
        int nbLegalMoves = pos.currentPos->nbLegalMoves;
        for (int i=mini; i<std::min(maxi, nbLegalMoves); i++){
          Move move = pos.currentPos->legalMoves[i];
          movePiece(pos, move);

          int add = getAllComb(pos, initialDepth, depth - 1);
          res += add;
          undoMove(pos);
          
          if (depth==initialDepth) {
            std::cout << move.toString() << " -> " << add << std::endl;
          }
        }
        return res;
      }
    } else {
      std::vector<std::thread> threads(nbThreads);
      std::vector<int> res(nbThreads);

      int step = pos.currentPos->nbLegalMoves / nbThreads;
      for (int i=0; i<nbThreads; i++) {
        int mini = i*step;
        int maxi = (i+1)*step;
        //copy pos
        Pos pos_i = Pos(pos);
        threads[i] = std::thread([mini, maxi, &pos_i, &res, i, initialDepth, depth, nbThreads](){
          res[i] = getAllComb(pos_i, initialDepth, depth, 1, mini, maxi);
        });
      }
      for (int i=0; i<nbThreads; i++) {
        threads[i].join();
      }
      int result = 0;
      for (int i=0; i<nbThreads; i++) {
        result += res[i];
      }
      return result;


    }
  }
}
}