import os
os.rename("Makefile", "Makefile.orig")

string_workin = '$(VSIM) -c -do \"do edalize_main.tcl; exit\"'
string_workout = '$(VSIM) -c -do \"edalize_main.tcl\" -do \"exit\"'

string_runin = '$(VSIM) -do \"run -all; quit -code [expr [coverage attribute -name TESTSTATUS -concise] >= 2 ? [coverage attribute -name TESTSTATUS -concise] : 0]; exit\"'

string_runout = '$(VSIM) -do run.tcl'

  
# opening a text file
fileIn = open("Makefile.orig", "r")
fileOut = open("Makefile", "w")

fileRun = open("run.tcl", "w")

# Loop through the file line by line
for line in fileIn:  

    # checking string is present in line or not
    if string_workin in line:
      line = line.replace(string_workin, string_workout)

    if string_runin in line:
      line = line.replace(string_runin, string_runout)

    
    fileOut.write(line)
          
fileRun.write("run -all; quit -code [expr [coverage attribute -name TESTSTATUS -concise] >= 2 ? [coverage attribute -name TESTSTATUS -concise] : 0]; exit")


# closing text file    
fileIn.close()
fileOut.close()
fileRun.close()
