#include "search.h"

namespace Chess {
namespace Search {

  Position::Move best;
  std::unordered_map<uint32_t, int> transpositionTable;
  int** whiteZobrist = new int*[64];
  int** blackZobrist = new int*[64];

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

  void init() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);

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

  Position::Move* orderMoves(bool white, pieceType* myPieces, pieceType* ennemyPieces) {
    int coeff = white ? 1 : -1;
    int rev = white ? 0 : 1;
    Position::Move* legalMoves = Position::legalMoves;
    int* score = new int[Tools::MAX_LEGAL_MOVES];

    for (int i=0; i<Tools::MAX_LEGAL_MOVES; i++) {
      Position::Move move = legalMoves[i];
      if (move.value == 0) {
        break;
      }
      int to = move.getTo();
      int indexTable = 63*rev + move.getTo() * coeff;
      

      
    }
    delete[] score;
    return legalMoves;
  }


  int eval() {
    int res = 0;
    int coeff = Position::whiteMove ? 1 : -1;
    int rev = Position::whiteMove ? 0 : 1;
    pieceType* myPieces = Position::myPieces;
    pieceType* ennemyPieces = Position::ennemyPieces;
    for(int i=0; i<64; i++) {
      int indexTable = 63*rev +i * coeff;
      res += (pieceValue[myPieces[i]] + pieceTables[myPieces[i]][indexTable]) * coeff;
      res -= (pieceValue[ennemyPieces[i]] + pieceTables[ennemyPieces[i]][indexTable]) * coeff;
    }
    return res;
  }


  int search(int initDepth, int depth, int alpha, int beta, int &nbEval) {
    if (depth == 0) {
      nbEval++;
      int res = eval();
      //std::cout << "Eval : " << res << std::endl;
      return res;
    } else {
      int indexMove = 0;
      int score = 0;
      Position::Move move = Position::legalMoves[indexMove];
      while(move.value != 0) {
        Position::movePiece(move);
        bool whiteMove = Position::whiteMove;
        uint32_t hash = getHash(Position::myPieces, Position::ennemyPieces, whiteMove);
        if (transpositionTable.find(hash) != transpositionTable.end()) {
          score = transpositionTable[hash];
        } else {
          score = search(initDepth, depth-1, alpha, beta, nbEval);
          transpositionTable[hash] = score;
        }
        Position::undoMove();

        if (!whiteMove) {
          if (score > alpha) {
            alpha = score;
            best = depth==initDepth ? move : best;
          }
        } else {
          if (score < beta) {
            beta = score;
            best = depth==initDepth ? move : best;
          }
        }
        if (alpha >= beta) {
          break;
        }

        indexMove++;
        move = Position::legalMoves[indexMove];
      }
      //check if there is no moves
      if (indexMove == 0) {
        if (Position::check) {
          score = Position::whiteMove ? -10000 : 10000;
          return score;
        } else {
          return 0;
        }
      }
      return (Position::whiteMove ? alpha : beta);
    }
  }

  Position::Move getBest() {
    return best;
  }

}
}