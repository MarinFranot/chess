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
        
    
    def show(self):
        window = tk.Tk()
        window.title("Chess")
        window.geometry("800x800")
        window.config(background="white")

        for i in range(64):
            square = tk.Canvas(window, width=100, height=100, bg=self.squares[i].color, highlightthickness=0)
            square.grid(row=8-i//8, column=i%8)
        
        window.mainloop()



