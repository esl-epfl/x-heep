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
% for bank in range(ram_numbanks):
create_power_domain PD_MEM_BANK_${bank} -elements {memory_subsystem_i/gen_sram[${bank}].ram_i}
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

% for bank in range(ram_numbanks):
add_power_state PD_MEM_BANK_${bank}.primary -state MEM_BANK_${bank}_ON <%text>\</%text>
    {-supply_expr {power == `{FULL_ON, 1.2} && ground == `{FULL_ON, 0.0}}}

add_power_state PD_MEM_BANK_${bank}.primary -state MEM_BANK_${bank}_OFF <%text>\</%text>
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

% for bank in range(ram_numbanks):
create_supply_net VDD_MEM_BANK_${bank}
create_supply_set PD_MEM_BANK_${bank}.primary -function {power VDD_MEM_BANK_${bank}} -function {ground VSS} -update

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
    -control_port       {sw_ctrl   cpu_subsystem_powergate_switch_o} <%text>\</%text>
    -ack_port           {sw_ack    cpu_subsystem_powergate_switch_ack_i} <%text>\</%text>
    -on_state           {on_state  sw_in {sw_ctrl}} <%text>\</%text>
    -off_state          {off_state {!sw_ctrl}}

create_power_switch switch_PD_PERIP_SUBS <%text>\</%text>
    -supply_set         PD_TOP.primary <%text>\</%text>
    -domain             PD_PERIP_SUBS <%text>\</%text>
    -input_supply_port  {sw_in     VDD} <%text>\</%text>
    -output_supply_port {sw_out    VDD_PERIP_SUBS} <%text>\</%text>
    -control_port       {sw_ctrl   peripheral_subsystem_powergate_switch_o} <%text>\</%text>
    -ack_port           {sw_ack    peripheral_subsystem_powergate_switch_ack_i} <%text>\</%text>
    -on_state           {on_state  sw_in {sw_ctrl}} <%text>\</%text>
    -off_state          {off_state {!sw_ctrl}}

% for bank in range(ram_numbanks):
create_power_switch switch_PD_MEM_BANK_${bank} <%text>\</%text>
    -supply_set         PD_TOP.primary <%text>\</%text>
    -domain             PD_MEM_BANK_${bank} <%text>\</%text>
    -input_supply_port  {sw_in     VDD} <%text>\</%text>
    -output_supply_port {sw_out    VDD_MEM_BANK_${bank}} <%text>\</%text>
    -control_port       {sw_ctrl   memory_subsystem_banks_powergate_switch_o[${bank}]} <%text>\</%text>
    -ack_port           {sw_ack    memory_subsystem_banks_powergate_switch_ack_i[${bank}]} <%text>\</%text>
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
    -isolation_signal cpu_subsystem_powergate_iso <%text>\</%text>
    -isolation_sense low <%text>\</%text>
    -clamp_value 0 <%text>\</%text>
    -applies_to outputs <%text>\</%text>
    -name_prefix cpu_iso_cell <%text>\</%text>
    -location parent

set_isolation perip_subs_iso <%text>\</%text>
    -domain PD_PERIP_SUBS <%text>\</%text>
    -isolation_power_net VDD <%text>\</%text>
    -isolation_ground_net VSS <%text>\</%text>
    -isolation_signal peripheral_subsystem_powergate_iso <%text>\</%text>
    -isolation_sense low <%text>\</%text>
    -clamp_value 0 <%text>\</%text>
    -applies_to outputs <%text>\</%text>
    -name_prefix cpu_iso_cell <%text>\</%text>
    -location parent

% for bank in range(ram_numbanks):
set_isolation mem_bank_${bank}_iso <%text>\</%text>
    -domain PD_MEM_BANK_${bank} <%text>\</%text>
    -isolation_power_net VDD <%text>\</%text>
    -isolation_ground_net VSS <%text>\</%text>
    -isolation_signal memory_subsystem_banks_powergate_iso[${bank}] <%text>\</%text>
    -isolation_sense low <%text>\</%text>
    -clamp_value 0 <%text>\</%text>
    -applies_to outputs <%text>\</%text>
    -name_prefix cpu_iso_cell <%text>\</%text>
    -location parent

% endfor
