

class Square:

  white = '#f0d9b5'
  black = '#b58863'

  def __init__(self, i, j):
    self.color = self.black if (i%2 == 0 and j %2 == 0) or (i%2 == 1 and j %2 == 1) else self.white 
    self.backColor = self.color
    self.selected = False

  
  def click(self):
    self.selected = not self.selected

    if self.selected:
      self.color = 'red'
    else:
      self.color = self.backColor

  def desactivate(self):
    self.selected = False
    self.color = self.backColor


  
