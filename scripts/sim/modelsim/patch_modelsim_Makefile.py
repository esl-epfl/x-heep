import os
os.rename("Makefile", "Makefile.orig")

string_workin = '$(VSIM) -c -do \"do edalize_main.tcl; exit\"'
string_workout = '$(VSIM) -c -do \"edalize_main.tcl\" -do \"exit\"'

string_runin = '$(VSIM) -do \"run -all; quit -code [expr [coverage attribute -name TESTSTATUS -concise] >= 2 ? [coverage attribute -name TESTSTATUS -concise] : 0]; exit\"'
string_runout = '$(VSIM) -do run.tcl'

string_tb_top = 'TOPLEVEL      := tb_top'
string_tb_top_vopt = 'TOPLEVEL      := tb_top_vopt'

string_opt = 'vopt -work work -debugdb -fsmdebug -pedanticerrors +acc=npr $(addprefix -G,$(PARAMETERS)) tb_top -o tb_top_vopt'
string_opt_upf = 'vopt -work work -suppress vopt-9653 -debugdb -fsmdebug -pedanticerrors +acc=npr tb_top -o tb_top_vopt -pa_upf ../../../core-v-mini-mcu.upf -pa_top "/tb_top/testharness_i/core_v_mini_mcu_i" -pa_lib work -pa_enable=highlight+debug $(addprefix -G,$(PARAMETERS))'

# opening a text file
fileIn  = open("Makefile.orig", "r")
fileOut = open("Makefile", "w")
fileRun = open("run.tcl", "w")

# Loop through the file line by line
for line in fileIn:

    # checking string is present in line or not
    if string_workin in line:
      line = line.replace(string_workin, string_workout)

    if string_runin in line:
      line = line.replace(string_runin, string_runout)

    if string_tb_top in line:
      line = line.replace(string_tb_top, string_tb_top_vopt)

    fileOut.write(line)

fileRun.write("run -all; quit -code [expr [coverage attribute -name TESTSTATUS -concise] >= 2 ? [coverage attribute -name TESTSTATUS -concise] : 0]; exit")

fileOut.write("\nvopt:\n\t")
fileOut.write(string_opt)
fileOut.write("\n\nvopt_upf:\n\t")
fileOut.write(string_opt_upf)

# closing text file
fileIn.close()
fileOut.close()
fileRun.close()
