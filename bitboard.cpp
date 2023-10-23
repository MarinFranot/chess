#include <iostream>
#include <cstring>
#include <cmath>
#include <random>
#include <unordered_set>
#include <vector>
#include <fstream>

#include "position.h"
#include "bitboard.h"
#include "tools.h"


namespace Bitboard{

  uint64_t getMoves(int pos, uint64_t pieces, bool isRook, bool restriction){
    uint64_t res = 0;
    int ways[2][4] = {{Position::NORTH_EAST, Position::NORTH_WEST, Position::SOUTH_EAST, Position::SOUTH_WEST},
    {Position::NORTH, Position::SOUTH, Position::EAST, Position::WEST}};

    int mini = restriction ? 1 : 0;
    int maxi = restriction ? Position::boardSize-2 : Position::boardSize-1;

    for (int way : ways[isRook]){
      
      auto getFlagStop = [way, pieces, mini, maxi, isRook](int x){
        int col = Tools::getCol(x);
        int rank = Tools::getRank(x);
        bool flagstop = isRook &((col<=mini && way==Position::WEST) || (col>=maxi && way==Position::EAST) || 
                (rank<=mini && way==Position::SOUTH) || (rank>=maxi && way==Position::NORTH)) ||
                !isRook & ((col<=mini && (way==Position::NORTH_WEST || way==Position::SOUTH_WEST)) || (col>=maxi && (way==Position::NORTH_EAST || way==Position::SOUTH_EAST)) ||
                (rank<=mini && (way==Position::SOUTH_EAST || way==Position::SOUTH_WEST)) || (rank>=maxi && (way==Position::NORTH_EAST || way==Position::NORTH_WEST)));
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


  uint64_t* getAllPiecesComb(int pos, int &arrsize, bool isRook){
    uint64_t full1 = 0xFFFFFFFFFFFFFFFF;

    uint64_t original = getMoves(pos, 0ULL, isRook, true);
    uint64_t mask = original;

    int length = 0;
    while (mask) {
      mask &= (mask - 1);
      length++;
    }
    length = std::pow(2, length); //-1
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
    res[nb] = original;
    arrsize = length;
    return res;
  }


  bool isUnique(uint64_t* picecsComb, int arrSize){
    for (int i=0; i<arrSize; i++){
      for (int j=i+1; j<arrSize; j++){
        if (picecsComb[i] == picecsComb[j]){
          return false;
        }
      }
    }
    return true;
  }


  uint64_t getMagicNumber(uint64_t* picecsComb, int arrSize, int &maxTab, uint64_t &shift, uint64_t obj){
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

    uint64_t miniSize = UINT64_MAX;
    uint64_t nbShift = 1;
    uint64_t res = 0ULL;
    //while (64-nbShift >= 1+log2(arrSize+1)){
    while (miniSize > obj){
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
    maxTab = miniSize;
    shift = nbShift-1;
    return res;
  }

  void generateLongTable(uint64_t obj, bool isRook){
    std::ofstream outFile;
    outFile.open(isRook?"rookTable.txt":"bishopTable.txt", std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
      throw std::runtime_error("Failed to open the file.");
    }
    outFile << obj << " maxSizeAll" << std::endl; 

    for (int i=0; i<64; i++) {
      std::cout << "Calculating for " << i << std::endl;
      int maxTab = 0;
      int arrsize = 0;
      uint64_t shift = 0;
      uint64_t* piecesComb = Bitboard::getAllPiecesComb(i, arrsize, isRook);
      uint64_t magicNb = getMagicNumber(piecesComb, arrsize, maxTab, shift, obj);

      outFile << i << " pos" << std::endl; 
      outFile << arrsize << " nbComb" << std::endl; 
      outFile << maxTab << " maxi" << std::endl;
      outFile << magicNb << " magicNb" << std::endl; 
      outFile << shift << " shift" << std::endl; 


      //uint64_t shift = 1 + log2(arrsize+1);
      for (int j=0; j<arrsize; j++){
        int index = (piecesComb[j]*magicNb) >> shift;
        uint64_t moves = getMoves(i, piecesComb[j], isRook, false);
        outFile << index << " " << moves << std::endl; //square
      }

      delete[] piecesComb;
    }
    outFile.close();
  }

  



}