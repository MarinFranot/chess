from board import Board
import tkinter as tk
import subprocess



x = 30720
command = ['./exec', str(x)]
result = subprocess.run(command, capture_output=True, text=True)
print('result:', result.stdout)