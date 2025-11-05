/* Copyright 2025 EPFL
 * Copyright and related rights are licensed under the Solderpad Hardware
 * License, Version 0.51 (the “License”); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
 * or agreed to in writing, software, hardware and materials distributed under
 * this License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 *
 * Description: Contains common system definitions.
 *
 */

<%
  user_peripheral_domain = xheep.get_user_peripheral_domain()
  base_peripheral_domain = xheep.get_base_peripheral_domain()
  dma = base_peripheral_domain.get_dma()
  memory_ss = xheep.memory_ss()
%>

`ifndef CORE_V_MINI_MCU_CONFIG_H
`define CORE_V_MINI_MCU_CONFIG_H

`define AO_PERIPHERAL_SUBSYSTEM ao_peripheral_subsystem_${xheep._config_name}


`endif
