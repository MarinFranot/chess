import tkinter as tk
from square import Square
import subprocess


class Board:

    def __init__(self):
        self.squares = [Square(i//8, i%8) for i in range(64)]
        self.buttons = []
        self.showNumbers = False
    

    def setColor(self, nb, color):
        self.squares[nb].color = color
        
    
    def click(self, nb):
        self.squares[nb].click()
        color = self.squares[nb].color
        self.buttons[nb].config(bg=color, activebackground=color)


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
        

        window.mainloop()



