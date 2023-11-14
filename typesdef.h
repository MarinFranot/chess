#ifndef TYPESDEF_H
#define TYPESDEF_H

#include <iostream>


enum pieceType {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, EMPTY};
extern std::string pieceStr[7];
enum pinType {RANK, DIAG, FREE};
extern std::string fenInit;




#endif