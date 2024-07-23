upf_version 2.1

set_design_top core_v_mini_mcu
set_scope .


<%text>
#####################
##  POWER DOMAINS  ##
#####################
</%text>\

create_power_domain PD_TOP -include_scope
create_power_domain PD_CPU -elements {cpu_subsystem_i}
create_power_domain PD_PERIP_SUBS -elements {peripheral_subsystem_i}
% for bank in xheep.iter_ram_banks():
create_power_domain PD_MEM_BANK_${bank.name()} -elements {memory_subsystem_i/ram${bank.name()}_i}
% endfor


<%text>
####################
##  POWER STATES  ##
####################
</%text>\

add_power_state PD_TOP.primary -state TOP_ON <%text>\</%text>
    {-supply_expr {power == `{FULL_ON, 1.2} && ground == `{FULL_ON, 0.0}}}

add_power_state PD_CPU.primary -state CPU_ON <%text>\</%text>
    {-supply_expr {power == `{FULL_ON, 1.2} && ground == `{FULL_ON, 0.0}}}

add_power_state PD_CPU.primary -state CPU_OFF <%text>\</%text>
    {-supply_expr {power == `{OFF} && ground == `{FULL_ON, 0.0}}} -simstate CORRUPT

add_power_state PD_PERIP_SUBS.primary -state PERIP_SUBS_ON <%text>\</%text>
    {-supply_expr {power == `{FULL_ON, 1.2} && ground == `{FULL_ON, 0.0}}}

add_power_state PD_PERIP_SUBS.primary -state PERIP_SUBS_OFF <%text>\</%text>
    {-supply_expr {power == `{OFF} && ground == `{FULL_ON, 0.0}}} -simstate CORRUPT

% for bank in xheep.iter_ram_banks():
add_power_state PD_MEM_BANK_${bank.name()}.primary -state MEM_BANK_${bank.name()}_ON <%text>\</%text>
    {-supply_expr {power == `{FULL_ON, 1.2} && ground == `{FULL_ON, 0.0}}}

add_power_state PD_MEM_BANK_${bank.name()}.primary -state MEM_BANK_${bank.name()}_OFF <%text>\</%text>
    {-supply_expr {power == `{OFF} && ground == `{FULL_ON, 0.0}}} -simstate CORRUPT

% endfor

<%text>
###################
##  SUPPLY NETS  ##
###################
</%text>\

create_supply_port VDD -direction in
create_supply_port VSS -direction in

create_supply_net VDD
create_supply_net VSS

connect_supply_net VDD -ports VDD
connect_supply_net VSS -ports VSS

create_supply_set PD_TOP.primary -function {power VDD} -function {ground VSS} -update

create_supply_net VDD_CPU
create_supply_set PD_CPU.primary -function {power VDD_CPU} -function {ground VSS} -update

create_supply_net VDD_PERIP_SUBS
create_supply_set PD_PERIP_SUBS.primary -function {power VDD_PERIP_SUBS} -function {ground VSS} -update

% for bank in xheep.iter_ram_banks():
create_supply_net VDD_MEM_BANK_${bank.name()}
create_supply_set PD_MEM_BANK_${bank.name()}.primary -function {power VDD_MEM_BANK_${bank.name()}} -function {ground VSS} -update

% endfor

<%text>
################
##  SWITCHES  ##
################
</%text>\

create_power_switch switch_PD_CPU <%text>\</%text>
    -supply_set         PD_TOP.primary <%text>\</%text>
    -domain             PD_CPU <%text>\</%text>
    -input_supply_port  {sw_in     VDD} <%text>\</%text>
    -output_supply_port {sw_out    VDD_CPU} <%text>\</%text>
    -control_port       {sw_ctrl   ao_peripheral_subsystem_i/cpu_subsystem_pwr_ctrl_o<%text>\</%text>[pwrgate_en_n<%text>\</%text>]} <%text>\</%text>
    -ack_port           {sw_ack    ao_peripheral_subsystem_i/cpu_subsystem_pwr_ctrl_i<%text>\</%text>[pwrgate_ack_n<%text>\</%text>]}  <%text>\</%text>
    -on_state           {on_state  sw_in {sw_ctrl}} <%text>\</%text>
    -off_state          {off_state {!sw_ctrl}}

create_power_switch switch_PD_PERIP_SUBS <%text>\</%text>
    -supply_set         PD_TOP.primary <%text>\</%text>
    -domain             PD_PERIP_SUBS <%text>\</%text>
    -input_supply_port  {sw_in     VDD} <%text>\</%text>
    -output_supply_port {sw_out    VDD_PERIP_SUBS} <%text>\</%text>
    -control_port       {sw_ctrl   ao_peripheral_subsystem_i/peripheral_subsystem_pwr_ctrl_o<%text>\</%text>[pwrgate_en_n<%text>\</%text>]} <%text>\</%text>
    -ack_port           {sw_ack    ao_peripheral_subsystem_i/peripheral_subsystem_pwr_ctrl_i<%text>\</%text>[pwrgate_ack_n<%text>\</%text>]}  <%text>\</%text>
    -on_state           {on_state  sw_in {sw_ctrl}} <%text>\</%text>
    -off_state          {off_state {!sw_ctrl}}

% for bank in xheep.iter_ram_banks():
create_power_switch switch_PD_MEM_BANK_${bank.name()} <%text>\</%text>
    -supply_set         PD_TOP.primary <%text>\</%text>
    -domain             PD_MEM_BANK_${bank.name()} <%text>\</%text>
    -input_supply_port  {sw_in     VDD} <%text>\</%text>
    -output_supply_port {sw_out    VDD_MEM_BANK_${bank.name()}} <%text>\</%text>
    -control_port       {sw_ctrl   ao_peripheral_subsystem_i/memory_subsystem_pwr_ctrl_o[${bank.name()}]<%text>\</%text>[pwrgate_en_n<%text>\</%text>]} <%text>\</%text>
    -ack_port           {sw_ack    ao_peripheral_subsystem_i/memory_subsystem_pwr_ctrl_i[${bank.name()}]<%text>\</%text>[pwrgate_ack_n<%text>\</%text>]}  <%text>\</%text>
    -on_state           {on_state  sw_in {sw_ctrl}} <%text>\</%text>
    -off_state          {off_state {!sw_ctrl}}

% endfor

<%text>
#################
##  ISOLATION  ##
#################
</%text>\

set_isolation cpu_iso <%text>\</%text>
    -domain PD_CPU <%text>\</%text>
    -isolation_power_net VDD <%text>\</%text>
    -isolation_ground_net VSS <%text>\</%text>
    -isolation_signal ao_peripheral_subsystem_i/cpu_subsystem_pwr_ctrl_o<%text>\</%text>[isogate_en_n<%text>\</%text>]  <%text>\</%text>
    -isolation_sense low <%text>\</%text>
    -clamp_value 0 <%text>\</%text>
    -applies_to outputs <%text>\</%text>
    -name_prefix cpu_iso_cell <%text>\</%text>
    -location parent

set_isolation perip_subs_iso <%text>\</%text>
    -domain PD_PERIP_SUBS <%text>\</%text>
    -isolation_power_net VDD <%text>\</%text>
    -isolation_ground_net VSS <%text>\</%text>
    -isolation_signal ao_peripheral_subsystem_i/peripheral_subsystem_pwr_ctrl_o<%text>\</%text>[isogate_en_n<%text>\</%text>]  <%text>\</%text>
    -isolation_sense low <%text>\</%text>
    -clamp_value 0 <%text>\</%text>
    -applies_to outputs <%text>\</%text>
    -name_prefix cpu_iso_cell <%text>\</%text>
    -location parent

% for bank in xheep.iter_ram_banks():
set_isolation mem_bank_${bank.name()}_iso <%text>\</%text>
    -domain PD_MEM_BANK_${bank.name()} <%text>\</%text>
    -isolation_power_net VDD <%text>\</%text>
    -isolation_ground_net VSS <%text>\</%text>
    -isolation_signal ao_peripheral_subsystem_i/memory_subsystem_pwr_ctrl_o[${bank.name()}]<%text>\</%text>[isogate_en_n<%text>\</%text>]  <%text>\</%text>
    -isolation_sense low <%text>\</%text>
    -clamp_value 0 <%text>\</%text>
    -elements {memory_subsystem_i/ram${bank.name()}_i/rdata_o} <%text>\</%text>
    -name_prefix cpu_iso_cell <%text>\</%text>
    -location parent

% endfor
