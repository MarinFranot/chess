#include "search.h"

namespace Search {

  Position::Move best;

  int pieceValue[7] = {100, 305, 333, 563, 950, 0, 0};

  int pawnTable[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10,-20,-20, 10, 10,  5,
    5, -5,-10,  0,  0,-10, -5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5,  5, 10, 25, 25, 10,  5,  5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0,  0,  0,  0,  0,  0,  0,  0,
  };
  int knightTable[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
  };
  int bishopTable[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
  };
  int rookTable[64] = {
    0,  0,  0,  5,  5,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    5, 10, 10, 10, 10, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0,
  };
  int queenTable[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -10,  5,  5,  5,  5,  5,  0,-10,
    0,  0,  5,  5,  5,  5,  0, -5,
    -5,  0,  5,  5,  5,  5,  0, -5,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20,
  };
  int kingTable[64] = {
    20, 30, 10,  0,  0, 10, 30, 20,
    20, 20,  0,  0,  0,  0, 20, 20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
  };
  int kingEnd[64] = {
    -50,-30,-30,-30,-30,-30,-30,-50,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -50,-40,-30,-20,-20,-30,-40,-50,
  };
  int emptyTable[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
  };
  int* pieceTables[7] = {pawnTable, knightTable, bishopTable, rookTable, queenTable, kingTable, emptyTable};

  int eval() {
    int res = 0;
    int coeff = Position::getWhiteMove() ? 1 : -1;
    int rev = Position::getWhiteMove() ? 0 : 1;
    pieceType* myPieces = Position::getMyPieces();
    pieceType* ennemyPieces = Position::getEnnemyPieces();
    for(int i=0; i<64; i++) {
      int indexTable = 63*rev +i * coeff;
      res += (pieceValue[myPieces[i]] + pieceTables[myPieces[i]][indexTable]) * coeff;
      res -= (pieceValue[ennemyPieces[i]] + pieceTables[ennemyPieces[i]][indexTable]) * coeff;
    }
    return res;
  }


  int search(int depth, int alpha, int beta) {
    if (depth == 0) {
      return eval();
    } else {
      int indexMove = 0;
      int score = 0;
      Position::Move move = Position::getLegalMoves()[indexMove];
      while(move.value != 0) {
        Position::movePiece(move);

        int score = search(depth-1, alpha, beta);
        if (Position::getWhiteMove()) {
          if (score > alpha) {
            alpha = score;
            best = move;
          }
        } else {
          if (score < beta) {
            beta = score;
            best = move;
          }
        }
        if (alpha >= beta) {
          break;
        }

        Position::undoMove();
        indexMove++;
        move = Position::getLegalMoves()[indexMove];
      }
      //check if there is no moves
      if (indexMove == 0) {
        if (Position::getCheck()) {
          score = Position::getWhiteMove() ? -10000 : 10000;
          return score;
        } else {
          return 0;
        }
      }
      return (Position::getWhiteMove() ? alpha : beta);
    }
  }

  Position::Move getBest() {
    return best;
  }

}