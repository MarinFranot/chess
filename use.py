import subprocess
from board import Board

def exec():
    commande = ["./exec"]

    result = subprocess.run(commande, capture_output=True, text=True)
    return int(result.stdout)


def colorBoard(board :Board, mask):
    i = 0
    while mask !=0 :
        val = (mask >> 0) & 1
        mask = mask >> 1
        if val:
            board.setColor(i, 'red')
        i += 1
    return board
            


if __name__ == "__main__":
    mask = exec()
    print(mask)
    print('\n\n')

    board = Board()
    colorBoard(board, mask)

    board.show()
    
