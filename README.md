# PanPrinter-Arduino
The repo of the arduino code of The PanPrinter project

This project is about creating a Pancake "printer" with Arduino Mega.

The printer is able to make the pancake in shapes, and even custom-made shapes that can even give several areas different colors (the time spent on the pan changes the color of the area).

The printer understands shapes sent in OSGrid, a programming-language of some sort, I invented for this project.
OSGrid looks similar to Assembly - command [arg1[,arg2]].

Example of OSGrid (15x15 grid):
helloPanSys       # Connection establishment
checkCon 2579     # Connection check ('2579' is a random number, to sort-of check that the connection is reliable)
defMethod OSGrid  # I added an option to print with a different parser, of course, OSGrid is the only one currently available
gridDef 15C       # Choosing the grid size (15C = 15x15 cells)
levelsDef 3       # Choosing how many hit categories will be made
startPos 8,4      # Moving the printer head to the starting cell of the current hit level
doLevel 1         # Opening the pancake mix chamber
movTo 9,4         # Moving the printer head (and dropping mix in the move)
movTo 10,4
endLevel          # Closing the pancake mix chamber
startPos 10,10
doLevel 2
movTo 10,11
movTo 10,12
endLevel
startPos 0,0
doLevel 3
movTo 1,1
movTo 2,2
endLevel
endPrint          # Finishing up and preparing for the next print
