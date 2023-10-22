from board import Board


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
  #Board.printMoves("King")

  board = Board()
  board.show()
  
