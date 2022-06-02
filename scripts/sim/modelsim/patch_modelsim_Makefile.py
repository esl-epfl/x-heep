import os
os.rename("Makefile", "Makefile.orig")

string1 = '$(VSIM) -c -do \"do edalize_main.tcl; exit\"'
string2 = '$(VSIM) -c -do \"edalize_main.tcl\" -do \"exit\"'
  
# opening a text file
fileIn = open("Makefile.orig", "r")
fileOut = open("Makefile", "w")
  

# Loop through the file line by line
for line in fileIn:  

    # checking string is present in line or not
    if string1 in line:
      print("yes")
      line = line.replace(string1, string2)

    print(line)
    fileOut.write(line)
          
# closing text file    
fileIn.close()
fileOut.close() 
