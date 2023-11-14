#include "uci.h"


namespace Chess {
namespace UCI {

  std::string options = "id name Stockfish 14.1\n"
  "id author the Stockfish developers (see AUTHORS file)\n\n"
  "option name Threads type spin default 1 min 1 max 512\n"
  "option name Hash type spin default 16 min 1 max 33554432\n"
  "option name Move Overhead type spin default 10 min 0 max 5000\n"
  "option name UCI_ShowWDL type check default false\n"
  "option name SyzygyPath type string default <empty>\n"
  "uciok\n";

  Position::Move createMove(Position::Pos pos, const char* line) {
    Position::Move move;
    const char* sub = (line + strlen(line) - 6);
    bool prom = sub[0] == ' ' ? false : true;

    int from = Tools::toIndexPos(std::string(sub+!prom, 2));
    int to = Tools::toIndexPos(std::string(sub+!prom + 2, 2));
    bool capture = pos.ennemyPieces[to] != EMPTY;
    pieceType captured = pos.ennemyPieces[to];
    bool enPassant = false;
    if (pos.ennemyPieces[to] == EMPTY && pos.myPieces[to] == PAWN) {
      enPassant = true;
    }
    bool is00 = false;
    bool is000 = false;
    if (pos.myPieces[to] == KING) {
      is00 = to-from == 2;
      is000 = to-from == -2;
    }
    pieceType promotionPiece = EMPTY;
    if (prom) {
      promotionPiece = Tools::toPieceType(sub[4]);
    }
    move = Position::Move(from, to, capture, captured, enPassant, is00, is000, prom, promotionPiece);
    return move;
  }

  void search(Position::Pos &pos) {
    int depth = 6;
    int alpha = -100000;
    int beta = 100000;
    int nbEval = 0;

    Search::search(pos, depth, depth, alpha, beta, nbEval);
    Chess::Position::Move best = Chess::Search::getBest();
    Position::movePiece(pos, best);
    std::cout << "bestmove " << best.toString() << std::endl;
    std::cout << "nbEval " << nbEval << std::endl;
  }

  void runUCI() {
    Position::Pos pos = Position::Pos();
    Position::createTables();

    Search::init();
    
    std::ofstream outFile;
    std::string filename = "output.txt";
    outFile.open(filename, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
      throw std::runtime_error("Failed to open the file.");
    }
    outFile << "beginning" << std::endl;

    std::string line_cpp;
    while (true) {
      std::getline(std::cin, line_cpp);
      line_cpp.append(1, '\n');
      const char* line = line_cpp.c_str();
      outFile << line_cpp << std::endl;
      

      if (strcmp(line, "uci\n") == 0) {
        std::cout << options;
      }
      if (strcmp(line, "isready\n") == 0) {
        printf("readyok\n");
        
      }
      if (Tools::endsWith(line, "ucinewgame\n")) {
        pos.freeHistory();
        Position::init(pos, fenInit);
      }
      if (Tools::endsWith(line, "startpos\n")) {
        search(pos);
      }
      if (Tools::isMove(line)) {
        Position::Move move = createMove(pos, line);
        Position::movePiece(pos, move);

        search(pos);
      }
      if (strcmp(line, "quit\n") == 0 || strcmp(line, "q\n") == 0) {
        pos.freeHistory();
        Position::free();
        Search::free();
        outFile << "quitting..." << std::endl;
        break;
      }
    }

    outFile.close();
  }

}
}
