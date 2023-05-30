import os
os.rename("Makefile", "Makefile.orig")

fileIn = open("Makefile.orig", "r")
fileOut = open("Makefile", "w")

# Loop through the file line by line
for line in fileIn:
    if "$(EDALIZE_LAUNCHER) vcs" in line:
        line = line[:-1] + " -ad=../../../hw/analog/control.init\n"
    fileOut.write(line)


# closing text file
fileIn.close()
fileOut.close()

