#include "search.h"


namespace Chess {
namespace Search {

  Position::Move best;
  std::unordered_map<uint32_t, int> transpositionTable;
  int** whiteZobrist = new int*[64];
  int** blackZobrist = new int*[64];

  int pieceValue[7] = {100, 305, 333, 563, 950, 0, 0};

  //tables for piece activity
  int pawnTable[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10,-30,-30, 10, 10,  5,
    5, -5,-10,  0,  0,-10, -5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5,  5, 10, 25, 25, 10,  5,  5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0,  0,  0,  0,  0,  0,  0,  0,
  };
  int knightTable[64] = {
    -50,-30,-30,-30,-30,-30,-30,-50,
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

  void init() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);

    // generate zobrist keys
    gen.seed(10);
    for (int i=0; i<64; i++) {
      whiteZobrist[i] = new int[7];
      blackZobrist[i] = new int[7];
      for (int j=0; j<7; j++) {
        whiteZobrist[i][j] = dis(gen);
        blackZobrist[i][j] = dis(gen);
      }
    }
  }

  void free() {
    for (int i=0; i<64; i++) {
      delete[] whiteZobrist[i];
      delete[] blackZobrist[i];
    }
    delete[] whiteZobrist;
    delete[] blackZobrist;
  }

  void clearTranspositionTable() {
    transpositionTable.clear();
  }

  //get zobrist hash of the position
  uint32_t getHash(pieceType* myPieces, pieceType* ennemyPieces, bool whiteMove) {
    auto getHash2 = [](pieceType* myPieces, pieceType* ennemyPieces) {
      uint32_t hash = 0;
      for (int i=0; i<64; i++) {
        hash ^= whiteZobrist[i][myPieces[i]];
        hash ^= blackZobrist[i][ennemyPieces[i]];
      }
      return hash;
    };
    return whiteMove ? getHash2(myPieces, ennemyPieces) : getHash2(ennemyPieces, myPieces);
  }
  int getIndex(int i, int rev, int coeff) {
    return 63*rev + i*coeff;
  }

  //order the different moves
  void orderMoves(Position::Pos pos) {
    auto compare = [](Position::Move a, Position::Move b) {
      return a.score > b.score;
    };
    int coeff = pos.whiteMove ? 1 : -1;
    int rev = pos.whiteMove ? 0 : 1;
    Position::Move* legalMoves = pos.currentPos->legalMoves;

    for (int i=0; i<Tools::MAX_LEGAL_MOVES; i++) {
      if (legalMoves[i].value == 0) {
        break;
      }
      Position::Move move = legalMoves[i];
      int to = move.getTo();
      int from = move.getFrom();
      move.score = pieceTables[pos.myPieces[from]][getIndex(to, rev, coeff)];
      move.score -=pieceTables[pos.myPieces[from]][getIndex(from, rev, coeff)];
      move.score += pieceValue[pos.ennemyPieces[to]];
    }
    std::sort(legalMoves, legalMoves + Tools::MAX_LEGAL_MOVES, compare);
  }

  //evaluation function
  int eval(Position::Pos &pos) {
    int res = 0;
    int coeff = pos.whiteMove ? 1 : -1;
    int rev = pos.whiteMove ? 0 : 1;
    pieceType* myPieces = pos.myPieces;
    pieceType* ennemyPieces = pos.ennemyPieces;
    for(int i=0; i<64; i++) {
      int indexTable = getIndex(i, rev, coeff);
      res += (pieceValue[myPieces[i]] + pieceTables[myPieces[i]][indexTable]) * coeff;
      res -= (pieceValue[ennemyPieces[i]] + pieceTables[ennemyPieces[i]][indexTable]) * coeff;
    }
    return res;
  }

  //alpha beta search
  int search(Position::Pos &pos, int initDepth, int depth, int alpha, int beta, int &nbEval) {
    if (depth == 0) {
      nbEval++;
      int res = eval(pos);
      return res;
    } else {
      int indexMove = 0;
      int score = 0;
      Position::Move move = pos.currentPos->legalMoves[indexMove];
      bool whiteMove = pos.whiteMove;
      orderMoves(pos);
      
      while(move.value != 0) {
        Position::movePiece(pos, move);

        uint32_t hash = getHash(pos.myPieces, pos.ennemyPieces, whiteMove);
        if (transpositionTable.find(hash) != transpositionTable.end()) {
          score = transpositionTable[hash];
        } else {
          score = search(pos, initDepth, depth-1, alpha, beta, nbEval);
          transpositionTable[hash] = score;
        }

        if (whiteMove) {
          if (score > alpha) {
            alpha = score;
            if (depth == initDepth) {
              best = move;
            }
          }
        } else {
          if (score < beta) {
            beta = score;
            if (depth == initDepth) {
              best = move;
            }
          }
        }
        Position::undoMove(pos);

        indexMove++;
        move = pos.currentPos->legalMoves[indexMove];
        if (alpha >= beta) {
          break;
        }
      }
      //check if there is no moves
      if (indexMove == 0) {
        if (pos.currentPos->check) {
          score = whiteMove ? -10000 : 10000;
          return score;
        } else {
          return 0;
        }
      }
      return (whiteMove ? alpha : beta);
    }
  }

  Position::Move getBest() {
    return best;
  }

}
}