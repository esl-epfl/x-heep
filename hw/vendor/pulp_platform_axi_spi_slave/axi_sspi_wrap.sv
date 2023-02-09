/* 
 * Copyright (C) 2018-2020 ETH Zurich, University of Bologna
 * Copyright and related rights are licensed under the Solderpad Hardware
 * License, Version 0.51 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 *
 *                http://solderpad.org/licenses/SHL-0.51. 
 *
 * Unless required by applicable law
 * or agreed to in writing, software, hardware and materials distributed under
 * this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * Alfio Di Mauro <adimauro@iis.ee.ethz.ch>
 *
 */

module axi_sspi_wrap #(
	parameter AXI_ADDR_WIDTH = 32,
	parameter AXI_DATA_WIDTH = 64,
	parameter AXI_USER_WIDTH = 1,
	parameter AXI_ID_WIDTH = 3,
	parameter DUMMY_CYCLES = 1
)(
	input logic clk_i,
	input logic rstn_i,
	input logic test_mode_i,

	AXI_BUS.Master axi_spis_bus,

	output qspis_pkg::qspis_to_pad_t qspis_to_pad_o,
	input qspis_pkg::pad_to_qspis_t pad_to_qspis_i
	
);

  logic [3:0] spi_oen;

  assign qspis_to_pad_o.sd0_oe = ~spi_oen[0];
  assign qspis_to_pad_o.sd1_oe = ~spi_oen[1];
  assign qspis_to_pad_o.sd2_oe = ~spi_oen[2];
  assign qspis_to_pad_o.sd3_oe = ~spi_oen[3];


axi_spi_slave #(
	.AXI_ADDR_WIDTH(AXI_ADDR_WIDTH),
	.AXI_DATA_WIDTH(AXI_DATA_WIDTH),
	.AXI_USER_WIDTH(AXI_USER_WIDTH),
	.AXI_ID_WIDTH  (AXI_ID_WIDTH),
	.DUMMY_CYCLES  (DUMMY_CYCLES)
) i_axi_spi_slave (
	.test_mode           (test_mode_i           ),

	.spi_sclk            (pad_to_qspis_i.sck_i  ),
	.spi_cs              (pad_to_qspis_i.csn_i ),

	.spi_oen0_o          (spi_oen[0] ),
	.spi_oen1_o          (spi_oen[1] ),
	.spi_oen2_o          (spi_oen[2] ),
	.spi_oen3_o          (spi_oen[3] ),


	.spi_sdi0            (pad_to_qspis_i.sd0_i  ),
	.spi_sdi1            (pad_to_qspis_i.sd1_i  ),
	.spi_sdi2            (pad_to_qspis_i.sd2_i  ),
	.spi_sdi3            (pad_to_qspis_i.sd3_i  ),
	.spi_sdo0            (qspis_to_pad_o.sd0_o  ),
	.spi_sdo1            (qspis_to_pad_o.sd1_o  ),
	.spi_sdo2            (qspis_to_pad_o.sd2_o  ),
	.spi_sdo3            (qspis_to_pad_o.sd3_o  ),

	.axi_aclk            (clk_i                 ),
	.axi_aresetn         (rstn_i                ),

	.axi_master_aw_valid (axi_spis_bus.aw_valid ),
	.axi_master_aw_addr  (axi_spis_bus.aw_addr  ),
	.axi_master_aw_prot  (axi_spis_bus.aw_prot  ),
	.axi_master_aw_region(axi_spis_bus.aw_region),
	.axi_master_aw_len   (axi_spis_bus.aw_len   ),
	.axi_master_aw_size  (axi_spis_bus.aw_size  ),
	.axi_master_aw_burst (axi_spis_bus.aw_burst ),
	.axi_master_aw_lock  (axi_spis_bus.aw_lock  ),
	.axi_master_aw_cache (axi_spis_bus.aw_cache ),
	.axi_master_aw_qos   (axi_spis_bus.aw_qos   ),
	.axi_master_aw_id    (axi_spis_bus.aw_id    ),
	.axi_master_aw_user  (axi_spis_bus.aw_user  ),
	.axi_master_aw_ready (axi_spis_bus.aw_ready ),
	.axi_master_ar_valid (axi_spis_bus.ar_valid ),
	.axi_master_ar_addr  (axi_spis_bus.ar_addr  ),
	.axi_master_ar_prot  (axi_spis_bus.ar_prot  ),
	.axi_master_ar_region(axi_spis_bus.ar_region),
	.axi_master_ar_len   (axi_spis_bus.ar_len   ),
	.axi_master_ar_size  (axi_spis_bus.ar_size  ),
	.axi_master_ar_burst (axi_spis_bus.ar_burst ),
	.axi_master_ar_lock  (axi_spis_bus.ar_lock  ),
	.axi_master_ar_cache (axi_spis_bus.ar_cache ),
	.axi_master_ar_qos   (axi_spis_bus.ar_qos   ),
	.axi_master_ar_id    (axi_spis_bus.ar_id    ),
	.axi_master_ar_user  (axi_spis_bus.ar_user  ),
	.axi_master_ar_ready (axi_spis_bus.ar_ready ),
	.axi_master_w_valid  (axi_spis_bus.w_valid  ),
	.axi_master_w_data   (axi_spis_bus.w_data   ),
	.axi_master_w_strb   (axi_spis_bus.w_strb   ),
	.axi_master_w_user   (axi_spis_bus.w_user   ),
	.axi_master_w_last   (axi_spis_bus.w_last   ),
	.axi_master_w_ready  (axi_spis_bus.w_ready  ),
	.axi_master_r_valid  (axi_spis_bus.r_valid  ),
	.axi_master_r_data   (axi_spis_bus.r_data   ),
	.axi_master_r_resp   (axi_spis_bus.r_resp   ),
	.axi_master_r_last   (axi_spis_bus.r_last   ),
	.axi_master_r_id     (axi_spis_bus.r_id     ),
	.axi_master_r_user   (axi_spis_bus.r_user   ),
	.axi_master_r_ready  (axi_spis_bus.r_ready  ),
	.axi_master_b_valid  (axi_spis_bus.b_valid  ),
	.axi_master_b_resp   (axi_spis_bus.b_resp   ),
	.axi_master_b_id     (axi_spis_bus.b_id     ),
	.axi_master_b_user   (axi_spis_bus.b_user   ),
	.axi_master_b_ready  (axi_spis_bus.b_ready  )
);


endmodule
