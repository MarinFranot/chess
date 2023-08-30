#include <iostream>
#include <cstring>
#include <cmath>
#include <random>
#include <unordered_set>
#include <vector>

#include "position.h"
#include "bitboard.h"


namespace Bitboard{

    uint64_t getMoves(int pos, uint64_t pieces, bool restriction){
        uint64_t res = 0;
        int ways[] = {Position::north, Position::south, Position::east, Position::west};

        int mini = restriction ? 1 : 0;
        int maxi = restriction ? Position::boardSize-2 : Position::boardSize-1;
        
        for (int way : ways){
            
            auto getFlagStop = [way, pieces, mini, maxi](int x){
                int col = Position::getCol(x);
                int rank = Position::getRank(x);
                bool flagstop = (col<=mini && way==Position::west) || (col>=maxi && way==Position::east) || 
                                (rank<=mini && way==Position::south) || (rank>=maxi && way==Position::north);
                return flagstop;
            };

            bool flagstop = getFlagStop(pos);
            int x = pos + way;
            
            while(!flagstop){
                res |= 1ULL << x;
                
                flagstop = getFlagStop(x) || ((pieces >> x) & 1ULL);
                x += way;
                if (flagstop){
                    break;
                }
            }
        }
        return res;
    }


    uint64_t* getAllPiecesComb(int pos, int &arrsize){
        uint64_t full1 = 0xFFFFFFFFFFFFFFFF;

        uint64_t original = getMoves(pos, 0ULL, true);
        uint64_t mask = original;
        std::vector<int> positions;

        int length = 0;
        while (mask) {
            mask &= (mask - 1);
            length++;
        }
        length = std::pow(2, length)-1;
        uint64_t* res = new uint64_t[length];
        int nb = 0;
        mask = 0;
        while(mask != original){
            for (int i = 0; i < 64; i++){
                if (((original >> i) & 1ULL) && !((mask >> i) & 1ULL)){
                    res[nb] = mask;
                    nb++;
                    mask = mask & (full1 << i);
                    mask |= 1ULL << i;

                    break;
                }
            }
        }
        //std::cout << "nb: " << nb << std::endl;
        arrsize = length;
        return res;
    }


    bool isUnique(uint64_t* picecsComb, int arrSize){
        /*std::unordered_set<int> elements;
        bool unique = true;
        for (int j = 0; j < arrSize; j++) {
            if (elements.find(picecsComb[j]) != elements.end()) {
                unique = false;
            }
            elements.insert(picecsComb[j]);
        }
        return unique;*/
        for (int i=0; i<arrSize; i++){
            for (int j=i+1; j<arrSize; j++){
                if (picecsComb[i] == picecsComb[j]){
                    return false;
                }
            }
        }
        return true;
    }


    uint64_t getMagicNumber(uint64_t* picecsComb, int arrSize, int &maxTab){
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

        uint64_t miniSize = UINT64_MAX;
        int nbShift = 1;
        uint64_t res = 0ULL;
        while (64-nbShift >= 1+log2(arrSize+1)){
            uint64_t magicNb = dis(gen);
            uint64_t* newComb = new uint64_t[arrSize];
            std::memcpy(newComb, picecsComb, arrSize * sizeof(uint64_t));

            uint64_t maxi = 0;
            for (int j = 0; j < arrSize; j++){
                newComb[j] = (newComb[j]*magicNb) >> nbShift;
                if (newComb[j] > maxi){
                    maxi = newComb[j];
                }
            }

            bool unique = isUnique(newComb, arrSize);


            delete[] newComb;

            if (unique && maxi < miniSize) {
                miniSize = maxi;
                nbShift++;
                res = magicNb;
            }
        }
        std::cout << "miniSize: " << miniSize << std::endl;
        std::cout << "nbShift: " << nbShift << std::endl;
        return res;
    }

    uint64_t** generateRookTable(uint64_t* magicNumbers){
        uint64_t** tab = new uint64_t*[64];
        for (int i=0; i<64; i++) {
            int maxTab = 0;
            int arrsize = 0;
            uint64_t* piecesComb = Bitboard::getAllPiecesComb(i, arrsize);
            uint64_t magicNb = getMagicNumber(piecesComb, arrsize, maxTab);
            magicNumbers[i] = magicNb;

            uint64_t* moves = new uint64_t[maxTab];

            int shift = 1 + log2(arrsize+1);
            for (int j=0; j<arrsize; j++){
                int index = (piecesComb[j]*magicNb) >> shift;
                moves[index] = piecesComb[j];
            }
            tab[i] = moves;

            delete[] piecesComb;
        }
        return tab;
    }


}