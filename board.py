import tkinter as tk
from square import Square
import subprocess
import numpy as np


class Board:

  def __init__(self):
    self.squares = [Square(i//8, i%8) for i in range(64)]
    self.buttons = []
    self.showNumbers = False
    self.lastClicked = 0
    self.table = Parser()
    self.selected = 0
  

  def setColor(self, nb, color):
    self.squares[nb].color = color
    
  
  def click(self, nb):
    self.squares[nb].click()
    color = self.squares[nb].color
    self.buttons[nb].config(bg=color, activebackground=color)
    self.lastClicked = nb
    #update bit nb in selected
    self.selected ^= 1 << nb


  def updateNumbers(self):
    self.showNumbers = not self.showNumbers
    for i in range(64):
      text = str(i) if self.showNumbers else ""
      self.buttons[i].config(text=text)
  
  def exec(self):
    piecesMask = 0
    for i in range(64):
      if self.squares[i].selected:
        piecesMask += 1 << i

    make = subprocess.run('make', capture_output=True, text=True)
    print(make.stdout)

    command = ['./exec', 'getMoves', str(piecesMask)]
    result = subprocess.run(command , capture_output=True, text=True)
    self.showControl(int(result.stdout))
    return int(result.stdout)

  def showControl(self, mask):
    for i in range(64):
      if mask & (1 << i):
        self.squares[i].color = "green"
        self.squares[i].desactivate()
        self.buttons[i].config(bg="green", activebackground="green")
      else:
        color = self.squares[i].backColor
        self.buttons[i].config(bg=color, activebackground=color)
        self.squares[i].color = color
        
       
  def genMoves(pos, type):
    moves = 0
    add = {"knight" : [-6, -10, -15, -17, 6, 10, 15, 17],
            "king" : [-9, -8, -7, -1, 1, 7, 8, 9],
            "pawn" : [-7, -9, 7, 9]}
    for move in add[type]:
      newPos = move + pos
      if newPos >= 0 and newPos < 64:
        cond = not((move%8 < 8/2)^ (newPos%8 >= pos%8))
        if cond:
          moves |= 2**newPos
    return moves
  
    

  def showLongMoves(self, pos, mask, isRook):
    print("mask = ", mask)
    magicNb = int(self.table.rookMagiNumbers[pos]) if isRook else int(self.table.bishopMagiNumbers[pos])
    shift = int(self.table.rookShifts[pos]) if isRook else int(self.table.bishopShifts[pos])
    idx = (mask * magicNb) >> shift
    idx &= (1<<(64-shift))-1
    moves = int(self.table.rookTable[pos][idx]) if isRook else int(self.table.bishopTable[pos][idx])
    for i in range(64):
      if moves & (1 << i):
        self.squares[i].color = "green"
        self.squares[i].desactivate()
        self.buttons[i].config(bg="green", activebackground="green")
      else:
        color = self.squares[i].backColor
        self.buttons[i].config(bg=color, activebackground=color)
        self.squares[i].color = color
    self.selected = 0


  def show(self):
    window = tk.Tk()
    window.title("Chess")

    size = 500
    dim = size/8
    window.geometry(str(size+2*int(dim))+"x"+str(size))
    window.config(background="white")

    for i in range(64):
      color = self.squares[i].color
      square = tk.Button(window, bg=color, activebackground=color,
                 highlightthickness=0, bd=0)
      square.config(command=lambda i=i: self.click(i))
      square.place(x=(i%8)*dim, y=size-(i//8+1)*dim, width=dim, height=dim)


      self.buttons.append(square)

    nbB = 1
    buttonNumber = tk.Button(window, text="numbers", command=self.updateNumbers)
    buttonNumber.place(x=size+dim/4, y=dim/2, width=dim*3/2, height=dim/2)

    nbB += 2
    buttonExec = tk.Button(window, text="exec", command=self.exec)
    buttonExec.place(x=size+dim/4, y=nbB*dim/2, width=dim*3/2, height=dim/2)

    nbB += 2
    buttonKnight = tk.Button(window, text="knight", command=lambda: self.showControl(Board.genMoves(self.lastClicked, "knight")))
    buttonKnight.place(x=size+dim/4, y=nbB*dim/2, width=dim*3/2, height=dim/2)

    nbB += 2
    buttonKing = tk.Button(window, text="king", command=lambda: self.showControl(Board.genMoves(self.lastClicked, "king")))
    buttonKing.place(x=size+dim/4, y=nbB*dim/2, width=dim*3/2, height=dim/2)

    nbB += 2
    fieldR = tk.Entry(window)
    fieldR.place(x=size+dim/4, y=nbB*dim/2, width=dim*3/2/2, height=dim/2)
    buttonRook = tk.Button(window, text="rook", command=lambda: self.showLongMoves(int(fieldR.get()), self.selected, True))
    buttonRook.place(x=size+dim/4+dim*3/2/2, y=nbB*dim/2, width=dim*3/2/2, height=dim/2)

    nbB += 2
    fieldB = tk.Entry(window)
    fieldB.place(x=size+dim/4, y=nbB*dim/2, width=dim*3/2/2, height=dim/2)
    buttonBishop = tk.Button(window, text="bishop", command=lambda: self.showLongMoves(int(fieldB.get()), self.selected, False))
    buttonBishop.place(x=size+dim/4+dim*3/2/2, y=nbB*dim/2, width=dim*3/2/2, height=dim/2)


    

    window.mainloop()



class Parser:

  def __init__(self):
    self.rookTable, self.rookMagiNumbers, self.rookShifts = Parser.getTable("rookTable.txt")
    self.bishopTable, self.bishopMagiNumbers, self.bishopShifts = Parser.getTable("bishopTable.txt")

  def getTable(filename):
    with open(filename, 'r') as f:
      lines = f.readlines()
    
    idx = 1
    sizeMaxAll = int(lines[0].split()[0])+1
    rookTable = np.zeros((64, sizeMaxAll), dtype=np.uint64)
    magicNumbers = np.zeros(64, dtype=np.uint64)
    shifts = np.zeros(64, dtype=np.int)

    pos = 0
    while idx < len(lines):
      line = lines[idx].strip()

      arrsize = int(lines[idx+1].split()[0])
      magicNumbers[pos] = int(lines[idx+3].split()[0])
      shifts[pos] = int(lines[idx+4].split()[0])

      idx += 5

      for j in range(arrsize):
        line = lines[idx].split()
        idxTable = int(line[0])
        mask = int(line[1])
        rookTable[pos][idxTable] = mask
        
        idx += 1
      
      pos += 1

    return rookTable, magicNumbers, shifts