#include <iostream>
#include <string>


#include "position.h"

namespace Chess {
namespace Position {
  //directions
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

  // masks
  uint64_t* rankMasks = Tools::getLineMasks(true);
  uint64_t* colMasks = Tools::getLineMasks(false);
  uint64_t* diagNEMasks = Tools::getDiagMasks(true);
  uint64_t* diagSEMasks = Tools::getDiagMasks(false);

  //generate bishop and rook moves
  uint64_t* bishopMagicNbs = new uint64_t[64];
  uint64_t* rookMagicNbs = new uint64_t[64];
  int* bishopShifts = new int[64];
  int* rookShifts = new int[64];

  uint64_t* pawnMoves[2] = {Tools::getPiecesMovesMask(PAWN, false), Tools::getPiecesMovesMask(PAWN, true)};
  uint64_t* knightMoves = Tools::getPiecesMovesMask(KNIGHT);
  uint64_t** bishopMoves = Tools::getTable(false, bishopMagicNbs, bishopShifts);
  uint64_t** rookMoves = Tools::getTable(true, rookMagicNbs, rookShifts);
  uint64_t* kingMoves = Tools::getPiecesMovesMask(KING);


  // get a rank or diagonal mask
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

  void printBitboard(uint64_t bitboard) {
    int i = 56;
    while (true) {
      std::cout << ((bitboard >> i) & 1ULL);
      if (i%8 == 7) {
        std::cout << std::endl;
        if (i == 7) {
          break;
        }
        i -= 16;
      }
      i++;
    }
    std::cout << std::endl;
  }

  //get the moves for a piece
  uint64_t lookupBishop(const Pos& pos, int from){
    uint64_t mask = (pos.myPiecesMask|(pos.ennemyPiecesMask &~(1ULL<<pos.ennemyKingPos))) & (getNEDiag(from)|getSEDiag(from)) & ~(1ULL<<from) & ~rim;
    uint64_t index = (mask*bishopMagicNbs[from])>>(bishopShifts[from]);
    return bishopMoves[from][index];
  }
  uint64_t lookupRook(const Pos& pos, int from){
    uint64_t restrict = rim & ~((getRankMask(63) & -static_cast<uint64_t>(from/8==7)) | (getRankMask(0) & -static_cast<uint64_t>(from/8==0)) |
     (getColMask(7) & -static_cast<uint64_t>(from%8==7)) | (getColMask(0) & -static_cast<uint64_t>(from%8==0))) ;
    uint64_t mask = (pos.myPiecesMask|(pos.ennemyPiecesMask &~(1ULL<<pos.ennemyKingPos))) & (getRankMask(from)|getColMask(from)) & ~(1ULL<<from) & ~(restrict | corners);	
    return rookMoves[from][(mask*rookMagicNbs[from])>>rookShifts[from]];
  }
  uint64_t lookupQueen(const Pos &pos, int from){
    return lookupBishop(pos, from) | lookupRook(pos, from);
  }

  //update pins and discovery checks
  void updatePins(Pos &pos){
    pos.currentPos->pins = 0;
    int enPassant = pos.currentPos->enPassant;
    enPassant = enPassant > 0 ? (pos.whiteMove ? enPassant+8 : enPassant-8) : enPassant;

    int remove = -1;
    uint64_t dirPawn = 0;
    uint64_t bufferEnnemyPiecesMask = pos.ennemyPiecesMask;
    uint64_t bufferMyPiecesMask = pos.myPiecesMask;
    if (enPassant >=0 && pos.ennemyKingPos/8 == enPassant/8) {
      
      bool condLeft = enPassant % 8 > 0 && ((pos.ennemyPawnsMask >> (enPassant-1)) & 1ULL);
      bool condRight = enPassant % 8 < 7 && ((pos.ennemyPawnsMask >> (enPassant+1)) & 1ULL);
      int newPawn = condLeft ? enPassant-1 : condRight ? enPassant+1 : -1;

      remove = pos.ennemyKingPos%8 < enPassant%8 ? std::min(newPawn, enPassant) : std::max(newPawn, enPassant);
      if (condLeft | condRight) {
        dirPawn = Tools::getDir(newPawn, enPassant);
        pos.myPiecesMask &= ~(1ULL<<remove);
        pos.ennemyPiecesMask &= ~(1ULL<<remove);
      }
    }

    uint64_t queen = lookupQueen(pos, pos.ennemyKingPos);
    pos.ennemyPiecesMask = bufferEnnemyPiecesMask;
    pos.myPiecesMask = bufferMyPiecesMask;


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
        
        bool condPassant = pinned == PAWN && (dir == 1) && (pos.myPieces[index + dirPawn] == PAWN);
        if (condPassant) {
          originalMask = ~(1ULL << pos.currentPos->enPassant);
          pos.myPiecesMask &= ~(1ULL<<(index+dirPawn));
          mask = lookupRook(pos, index) & getRankMask(index);
          pos.myPiecesMask |= 1ULL<<(index+dirPawn);
        }

        
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
      //update discovery checks
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

  // detect checks
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

  //swap the position
  void swap(Pos &pos){
    pos.whiteMove = !pos.whiteMove;
    std::swap(pos.myPieces, pos.ennemyPieces);
    std::swap(pos.myPiecesMask, pos.ennemyPiecesMask);
    std::swap(pos.myPawnsMask, pos.ennemyPawnsMask);
    std::swap(pos.currentPos->myControl, pos.currentPos->ennemyControl);
    std::swap(pos.myKingPos, pos.ennemyKingPos);
  }

  //get the moves for a piece
  uint64_t getMoves(Pos &pos, pieceType piece, int from, bool isWhite, bool getControl){
    SavePos* currentPos = pos.currentPos;
    uint64_t res = 0;
    uint64_t pin = ((currentPos->pins>>from) & 1ULL) ? currentPos->pinsMasks[from] : UINT64_MAX;
    if (piece == PAWN){
      int dir = pos.whiteMove ? 1 : -1;
      uint64_t control = pawnMoves[pos.whiteMove][from] & ~getColMask(from);
      if (getControl){
        return control;
      } else {
        uint64_t col = getColMask(from);
        res = pawnMoves[pos.whiteMove][from] & ~((pos.myPiecesMask | pos.ennemyPiecesMask) & col);
        uint64_t up2 = 1ULL<<(from+16*dir);
        if ((from/8!=1) & pos.whiteMove || (from/8!=6) & !pos.whiteMove) {
          res &= ~up2;
        } else if (((pos.myPiecesMask | pos.ennemyPiecesMask)>>(from+8*dir))&1ULL){
          res &= ~up2;
        }
        int enPassant = currentPos->enPassant;
        bool passant = enPassant >= 0;
        res = ((res & col) | (res & (pos.ennemyPiecesMask| ((1ULL<<enPassant)*passant)))) & currentPos->blockCheck & pin;
        if (passant && currentPos->blockCheck == 1ULL<<(enPassant - 8*dir) && ((control & 1ULL<<enPassant) != 0)) {
          res |= ((1ULL << enPassant) & pin);
        }
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


  //generate all the moves for a position
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

  //update the squares controlled bt my pieces
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


  //initialize the position from a fen string
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
    currentPos->checkers[0] = -1;
    currentPos->checkers[1] = -1;

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
    swap(pos);

    updateControl(pos);
    currentPos->check = currentPos->ennemyControl & (1ULL<<pos.myKingPos);
    updatePins(pos);
    updateChecks(pos);
    swap(pos);
    updateAllMoves(pos);


    delete[] pieceMap;
    return pos;
  }

  //free the memory
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
    for (int i=0; i<64; i++){
      delete[] bishopMoves[i];
      delete[] rookMoves[i];
    }
    delete[] bishopMoves;
    delete[] rookMoves;
  }

  // move the king and the rook for castling
  void updateCastling(Pos &pos, int fromRook, int toRook) {
    pos.myPieces[toRook] = pos.myPieces[fromRook];
    pos.myPieces[fromRook] = EMPTY;
    pos.myPiecesMask = (pos.myPiecesMask & ~(1ULL<<fromRook)) | (1ULL<<toRook);
  }

  //move a piece
  void movePiece(Pos &pos, Move move){
    auto removeCastling = [](Pos &pos, int posRook, bool white) {
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
        piece = promotion;
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
      } else if (captured == ROOK && to/8 == pos.ennemyKingPos/8) {
        removeCastling(pos, to, !pos.whiteMove);
      }
    }

    uint64_t control = getMoves(pos, piece, to, pos.whiteMove, true);
    currentPos->check = control & (1ULL<<pos.ennemyKingPos);
    currentPos->checkers[0] = -1;
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

  
  //undo a move
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


  // get all the combinations of moves
  int getAllComb(Pos &pos, int initialDepth, int depth, const int nbThreads, bool multi, int mini, int maxi) {
    if (!multi) {
      if (depth==0) {
        return 1;
      } else if (depth == 1) {
        return pos.currentPos->nbLegalMoves;
      } else {      
        int res = 0;
        int nbLegalMoves = pos.currentPos->nbLegalMoves;

        if (maxi - mini == 1 && nbThreads > 1) {
          Move move = pos.currentPos->legalMoves[mini];
          movePiece(pos, move);
          res += getAllComb(pos, initialDepth, depth - 1, nbThreads, true);
          undoMove(pos);
          if (depth==initialDepth) {
            std::cout << move.toString() << " -> " << res << " (" << nbThreads << " Threads)" << 
            std::endl;
          }
          return res;
        }
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
      int borneThreads = std::min(nbThreads, pos.currentPos->nbLegalMoves);
      std::vector<std::thread> threads(borneThreads);
      std::vector<int> res(borneThreads);
      
      int nbLegalMoves = pos.currentPos->nbLegalMoves;
      int step = nbLegalMoves / borneThreads;
      int mod = nbThreads % borneThreads;

      for (int i = 0; i < borneThreads; i++) {
        int newThreads = nbThreads / borneThreads + (i < mod ? 1 : 0);
        int mini = i * step;
        int maxi = (i+1) * step;
        threads[i] = std::thread([&, i, initialDepth, depth, mini, maxi](){
          Pos position(pos);
          res[i] = getAllComb(position, initialDepth, depth, newThreads, false, mini, maxi);
        });
      }

      for (int i = 0; i < borneThreads; i++) {
          threads[i].join();
      }

      int result = 0;
      for (int i = 0; i < borneThreads; i++) {
          result += res[i];
      }
      return result;
    }
  }
}
}