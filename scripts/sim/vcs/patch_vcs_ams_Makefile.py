# Copyright EPFL contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0


import os
os.rename("Makefile", "Makefile.orig")

fileIn = open("Makefile.orig", "r")
fileOut = open("Makefile", "w")

# Loop through the file line by line
for line in fileIn:
    if "$(EDALIZE_LAUNCHER) vcs" in line:
        line = line[:-1] + " -ad=../../../hw/ip_examples/ams/analog/control.init\n"
    fileOut.write(line)


# closing text file
fileIn.close()
fileOut.close()

