/* verilator lint_off DECLFILENAME */

% for intf in xheep.get_rh().get_intf_sv_helpers():
${intf.get_def()}

% endfor

/* verilator lint_on DECLFILENAME */
