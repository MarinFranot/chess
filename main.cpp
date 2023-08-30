#include <iostream>
#include "position.h"
#include "bitboard.h"



int main(int argc, char* argv[]) {
    std::string argument = argv[1];
    if (argument == "getMoves"){
        uint64_t pieces = std::stoull(argv[1]);
        int pos = 0;
        uint64_t control = Bitboard::getMoves(pos, pieces);
        std::cout << control;

    } else if (argument == "getTab"){
        int pos = 28;
        int arrsize = 0;
        uint64_t* tab = Bitboard::getAllPiecesComb(pos, arrsize);

        int maxTab = 0;
        uint64_t magicNb = Bitboard::getMagicNumber(tab, arrsize, maxTab);

        std::cout << "magicNb result : " << magicNb << std::endl;

        delete[] tab;


    } else {
        std::cout << "Error: invalid argument " << argv[1] << std::endl;
    }

    return 0;
}

