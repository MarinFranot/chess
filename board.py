import tkinter as tk
import subprocess
import numpy as np




class Square:

  white = '#f0d9b5'
  black = '#b58863'

  def __init__(self, i, j):
    self.color = self.black if (i%2 == 0 and j %2 == 0) or (i%2 == 1 and j %2 == 1) else self.white 
    self.backColor = self.color
    self.selected = False
    self.piece = ""

  
  def click(self):
    self.selected = not self.selected

    if self.selected:
      self.color = 'red'
    else:
      self.color = self.backColor

  def desactivate(self):
    self.selected = False
    self.color = self.backColor



class Board:

  def __init__(self):
    self.squares = [Square(i//8, i%8) for i in range(64)]
    self.buttons = []
    self.showNumbers = True
    self.lastClicked = 0
    self.selected = 0

  def intToSquare(nb):
    return chr(ord('a') + nb%8) + str(nb//8+1)
  
  def SquareToInt(square):
    return (ord(square[0]) - ord('a')) + 8*(int(square[1])-1)

  def setColor(self, nb, color):
    self.squares[nb].color = color
    
  
  def click(self, nb):
    self.squares[nb].click()
    color = self.squares[nb].color
    self.buttons[nb].config(bg=color, activebackground=color)
    self.lastClicked = self.selected
    self.selected = nb


  def updateNumbers(self):
    self.showNumbers = not self.showNumbers
    for i in range(64):
      text = str(i) if self.showNumbers else self.squares[i].piece
      self.buttons[i].config(text=text)
  
  def exec(self):
    make = subprocess.run('make', capture_output=True, text=True)
    print(make.stdout)

    self.process = subprocess.Popen(['./bot.exe'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    self.process.stdin.write('ucinewgame\n')
    self.process.stdin.flush()

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
        
  def movePiece(self, fromNb, toNb):
    self.squares[toNb].piece = self.squares[fromNb].piece
    self.squares[fromNb].piece = ""
    self.showNumbers = True
    self.updateNumbers()

  def sendMove(self, fromNb, toNb):
    self.click(fromNb)
    self.click(toNb)
    self.movePiece(fromNb, toNb)
    moveChr = Board.intToSquare(fromNb) + Board.intToSquare(toNb)
    self.process.stdin.write('position startpos moves '+moveChr+'\n')
    self.process.stdin.flush()
    output = self.process.stdout.readline()
    #clear stdout
    self.process.stdout.readline()


    output.strip()

    #get last 4 characters
    result = output[-5:-1]
    print(result)
    self.movePiece(Board.SquareToInt(result[0:2]), Board.SquareToInt(result[2:4]))

  def quit(self, window):
    self.process.stdin.write('q\n')
    self.process.stdin.flush()
    self.process.wait()
    self.process.terminate()
    window.destroy()


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
    self.squares[0].piece = "R"
    self.squares[1].piece = "N"
    self.squares[2].piece = "B"
    self.squares[3].piece = "Q"
    self.squares[4].piece = "K"
    self.squares[5].piece = "B"
    self.squares[6].piece = "N"
    self.squares[7].piece = "R"
    for i in range(8):
      self.squares[8+i].piece = "P"
      self.squares[48+i].piece = "p"
    self.squares[56].piece = "r"
    self.squares[57].piece = "n"
    self.squares[58].piece = "b"
    self.squares[59].piece = "q"
    self.squares[60].piece = "k"
    self.squares[61].piece = "b"
    self.squares[62].piece = "n"
    self.squares[63].piece = "r"
    self.updateNumbers()




    nbB = 1
    buttonNumber = tk.Button(window, text="numbers", command=self.updateNumbers)
    buttonNumber.place(x=size+dim/4, y=dim/2, width=dim*3/2, height=dim/2)

    nbB += 2
    buttonExec = tk.Button(window, text="exec", command=self.exec)
    buttonExec.place(x=size+dim/4, y=nbB*dim/2, width=dim*3/2, height=dim/2)

    nbB += 2
    buttonSend = tk.Button(window, text="send", command=lambda: self.sendMove(self.lastClicked, self.selected))
    buttonSend.place(x=size+dim/4, y=nbB*dim/2, width=dim*3/2, height=dim/2)

    nbB += 2
    buttonQuit = tk.Button(window, text="quit", command=lambda: self.quit(window))
    buttonQuit.place(x=size+dim/4, y=nbB*dim/2, width=dim*3/2, height=dim/2)

    

    window.mainloop()

