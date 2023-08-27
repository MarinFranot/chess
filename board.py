import tkinter as tk
from square import Square


class Board:

    def __init__(self):
        self.squares = [Square() for i in range(64)]

        for i in range(8):
            for j in range(8):
                if (i%2 == 0 and j %2 == 0) or (i%2 == 1 and j %2 == 1):
                    self.setColor(i*8+j, Square.black)
    

    def setColor(self, nb, color):
        self.squares[nb].color = color
        
    
    def show(self, printIdx=True):
        window = tk.Tk()
        window.title("Chess")

        size = 500
        dim = size/8
        window.geometry(str(size)+"x"+str(size))
        window.config(background="white")

        for i in range(64):
            square = tk.Button(window, bg=self.squares[i].color, 
                               highlightthickness=0)
            
            square.place(x=(i%8)*dim, y=(i//8)*dim, width=dim, height=dim)

        
        window.mainloop()



