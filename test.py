import numpy as np


def getRankMask():
  res=[]
  for i in range(8):
    nb=0
    for j in range(8):
      nb+=1<<(i+8*j)
    res.append(nb)
  return res


def getDiagMask():
  res=[0]*15

  for i in range(64):
    #idx = i//8 + i%8
    idx = 14-(i//8  - i%8 + 7)
    print(idx)
    res[idx] += 1<<i
  return res




tab = getDiagMask()
print(tab)
for i in range(len(tab)):
  #print by rank of 8 bits
  for j in range(8):
    for k in range(8):
      if tab[i] & (1<<(8*j+k)):
        print("1", end="")
      else:
        print("0", end="")
    print()
  print()