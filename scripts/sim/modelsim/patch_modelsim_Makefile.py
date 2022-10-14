import os
os.rename("Makefile", "Makefile.orig")

string_toreplace = []

string_toreplace.append('$(VSIM) -c -do \"do edalize_main.tcl; exit\"')
string_toreplace.append('$(VSIM) -do \"run -all; quit -code [expr [coverage attribute -name TESTSTATUS -concise] >= 2 ? [coverage attribute -name TESTSTATUS -concise] : 0]; exit\"')
string_toreplace.append('VSIM ?= $(MODEL_TECH)/vsim')
string_toreplace.append('TOPLEVEL      := tb_top')
string_toreplace.append('EXTRA_OPTIONS ?= $(VSIM_OPTIONS) $(addprefix -g,$(PARAMETERS)) $(addprefix +,$(PLUSARGS))')

string_replaced = []

string_replaced.append('$(VSIM) -c -do \"edalize_main.tcl\" -do \"exit\"')
string_replaced.append('$(VSIM) -do run.tcl')
string_replaced.append('VSIM ?= $(MODEL_TECH)/vsim' + "\n" + \
                       'VOPT ?= $(MODEL_TECH)/vopt'+ "\n");
string_replaced.append('RUN_OPT = '+ "\n" + \
                       'ifdef RUN_OPT'+ "\n" + \
                       '    TOPLEVEL      := tb_top_vopt' + "\n" + \
                       'else'+ "\n" + \
                       '    TOPLEVEL      := tb_top' + "\n" + \
                       'endif'+ "\n\n" \
                       'RUN_UPF_OPTIONS ='+ "\n" + \
                       'RUN_UPF ?= '+ "\n" + \
                       'ifdef RUN_UPF'+ "\n" + \
                       '    RUN_UPF_OPTIONS := -pa' + "\n" + \
                       'endif'+ "\n\n");
string_replaced.append('EXTRA_OPTIONS ?= $(VSIM_OPTIONS) $(addprefix -g,$(PARAMETERS)) $(addprefix +,$(PLUSARGS)) $(RUN_UPF_OPTIONS)')


string_toappend = []

string_toappend.append("\t" + 'rm -rf work' + "\n\n");

string_toappend.append('opt:' + "\n\t" + \
                       '$(VOPT) -work work -debugdb -fsmdebug -pedanticerrors +acc=npr $(addprefix -G,$(PARAMETERS)) $(TOPLEVEL) -o $(TOPLEVEL)_vopt' + "\n\n");

string_toappend.append('opt-upf:' + "\n\t" + \
                       '$(VOPT) -work work -debugdb -fsmdebug ' + \
                         '-pa_genrpt=pa+de+cell+conn+pst+srcsink ' + \
                         '-pa_enable=vsim_msgs+highlight+debug ' + \
                         '-pa_checks=s+ul+i+p+cp+upc+ugc ' + \
                         '-pa_upf ../../../core-v-mini-mcu.upf -pa_top "/tb_top/testharness_i/x_heep_system_i/core_v_mini_mcu_i" -pa_lib work +acc=npr $(addprefix -G,$(PARAMETERS)) $(TOPLEVEL) -o $(TOPLEVEL)_vopt' + "\n");


# opening a text file
fileIn = open("Makefile.orig", "r")
fileOut = open("Makefile", "w")

# Loop through the file line by line
for line in fileIn:

    it = 0
    for string in string_toreplace:
        # checking string is present in line or not
        if string in line:
          print(string)
          line = line.replace(string, string_replaced[it])
        it = it + 1


    fileOut.write(line)

for string in string_toappend:
    fileOut.write(string)

# closing text file
fileIn.close()
fileOut.close()

fileRun = open("run.tcl", "w")
fileRun.write("run -all; quit -code [expr [coverage attribute -name TESTSTATUS -concise] >= 2 ? [coverage attribute -name TESTSTATUS -concise] : 0]; exit")
fileRun.close()
