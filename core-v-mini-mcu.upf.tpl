upf_version 2.1

set_design_top core_v_mini_mcu
set_scope .

#####################
##  POWER DOMAINS  ##
#####################

create_power_domain PD_TOP -include_scope
create_power_domain PD_CPU -elements {cpu_subsystem_i}
create_power_domain PD_PERIP_SUBS -elements {peripheral_subsystem_i}
% for bank in range(ram_numbanks):
create_power_domain PD_MEM_BANK_${bank} -elements {memory_subsystem_i/gen_sram[${bank}].ram_i}
% endfor


####################
##  POWER STATES  ##
####################

add_power_state PD_CPU.primary -state CPU_ON \
    {-supply_expr {power == `{FULL_ON, 1.2} && ground == `{FULL_ON, 0.0}}}

add_power_state PD_CPU.primary -state CPU_OFF \
    {-supply_expr {power == `{OFF} && ground == `{FULL_ON, 0.0}}} -simstate CORRUPT

add_power_state PD_PERIP_SUBS.primary -state PERIP_SUBS_ON \
    {-supply_expr {power == `{FULL_ON, 1.2} && ground == `{FULL_ON, 0.0}}}

add_power_state PD_PERIP_SUBS.primary -state PERIP_SUBS_OFF \
    {-supply_expr {power == `{OFF} && ground == `{FULL_ON, 0.0}}} -simstate CORRUPT

% for bank in range(ram_numbanks):
add_power_state PD_MEM_BANK_${bank}.primary -state MEM_BANK_${bank}_ON \
    {-supply_expr {power == `{FULL_ON, 1.2} && ground == `{FULL_ON, 0.0}}}

add_power_state PD_MEM_BANK_${bank}.primary -state MEM_BANK_${bank}_OFF \
    {-supply_expr {power == `{OFF} && ground == `{FULL_ON, 0.0}}} -simstate CORRUPT

% endfor

add_power_state PD_TOP.primary -state TOP_ON \
    {-supply_expr {power == `{FULL_ON, 1.2} && ground == `{FULL_ON, 0.0}}}


###################
##  SUPPLY NETS  ##
###################

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


################
##  SWITCHES  ##
################

create_power_switch switch_PD_CPU \
    -supply_set         PD_TOP.primary \
    -domain             PD_CPU \
    -input_supply_port  {sw_in     VDD} \
    -output_supply_port {sw_out    VDD_CPU} \
    -control_port       {sw_ctrl   cpu_subsystem_powergate_switch} \
    -on_state           {on_state  sw_in {sw_ctrl}} \
    -off_state          {off_state {!sw_ctrl}}

create_power_switch switch_PD_PERIP_SUBS \
    -supply_set         PD_TOP.primary \
    -domain             PD_PERIP_SUBS \
    -input_supply_port  {sw_in     VDD} \
    -output_supply_port {sw_out    VDD_PERIP_SUBS} \
    -control_port       {sw_ctrl   peripheral_subsystem_powergate_switch} \
    -on_state           {on_state  sw_in {sw_ctrl}} \
    -off_state          {off_state {!sw_ctrl}}

% for bank in range(ram_numbanks):
create_power_switch switch_PD_MEM_BANK_${bank} \
    -supply_set         PD_TOP.primary \
    -domain             PD_MEM_BANK_${bank} \
    -input_supply_port  {sw_in     VDD} \
    -output_supply_port {sw_out    VDD_MEM_BANK_${bank}} \
    -control_port       {sw_ctrl   memory_subsystem_banks_powergate_switches[${bank}]} \
    -on_state           {on_state  sw_in {sw_ctrl}} \
    -off_state          {off_state {!sw_ctrl}}

% endfor


#################
##  ISOLATION  ##
#################

set_isolation cpu_iso \
    -domain PD_CPU \
    -isolation_power_net VDD \
    -isolation_ground_net VSS \
    -isolation_signal cpu_subsystem_powergate_switch \
    -isolation_sense low \
    -clamp_value 0 \
    -applies_to both \
    -name_prefix cpu_iso_cell \
    -location parent

set_isolation perip_subs_iso \
    -domain PD_PERIP_SUBS \
    -isolation_power_net VDD \
    -isolation_ground_net VSS \
    -isolation_signal peripheral_subsystem_powergate_switch \
    -isolation_sense low \
    -clamp_value 0 \
    -applies_to both \
    -name_prefix cpu_iso_cell \
    -location parent

% for bank in range(ram_numbanks):
set_isolation mem_bank_${bank}_iso \
    -domain PD_MEM_BANK_${bank} \
    -isolation_power_net VDD \
    -isolation_ground_net VSS \
    -isolation_signal memory_subsystem_banks_powergate_switches[${bank}] \
    -isolation_sense low \
    -clamp_value 0 \
    -applies_to both \
    -name_prefix cpu_iso_cell \
    -location parent

% endfor
