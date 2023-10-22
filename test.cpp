#include <iostream>
#include <fstream>
#include <string>


namespace Test {
    int x = 0;
    int y = 0;
    enum pieceType {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, EMPTY};

    void print(){
        std::cout << "x: " << x << std::endl;
        std::cout << "y: " << y << std::endl;
    }

    void setX(int newX){
        x = newX;
    }

    void setY(int newY){
        y = newY;
    }

    void incrementX(){
        x++;
    }

    void incrementY(){
        y++;
    }
}


int main(){
    Test::print();
    Test::setX(5);
    Test::incrementY();
    Test::print();
    
    Test::pieceType piece = Test::KING;
    std::cout << static_cast<uint64_t>(piece) << std::endl;
    return 0;
}