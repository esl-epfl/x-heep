# Usage sv2v current state

## Clone OpenROAD-flow-scripts

```bash
cd flow
git clone https://github.com/The-OpenROAD-Project/OpenROAD-flow-scripts
mv OpenROAD-flow-scripts OpenROAD
```

I did not know if you will use this and if so submodule or vendor...

## Edalize

I saw that you are using `edalize` + `fusesoc`. I already have a sv2v into yosys pipeline setup for `edalize`. 

* [Branch](https://github.com/joennlae/edalize/blob/openroad-flow-script/edalize/openroad.py)

If you install that version you should have the same result. I saw you have your own extension for `design-compiler`. Very nice :-)

```bash
# probably uninstall first
pip uninstall edalize
pip install git+https://github.com/joennlae/edalize.git@openroad-flow-script
```

## Run command

```bash
fusesoc --verbose --cores-root . run --target=asic_yosys_synthesis --flag=use_sky130 openhwgroup.org:systems:core-v-mini-mcu
```

## Current status

* [hw.patch](./hw.patch) hw/ folder patch

```bash
git apply hw.patch
```

* I chose to use `SYNTHESIS` as this is the keyword used by `lowRISC` for the ifdefs.
* I tried to debug your approach but I couldn't find the error fast enought and I knew that I already have a working setup that uses `fusesoc` + `edalize`. I think that it was certainly missing the `synth` command but I couldn't figure out how to include it into the pipeline. Anyhow I think using `OpenROAD-flow-scripts` from the beginning is a safe an proven way.
* Designs that work with this flow: [Designs](https://github.com/The-OpenROAD-Project/OpenROAD-flow-scripts/tree/master/flow/designs/sky130hd)

### Current Problems and output

I get the following error.
```bash
314.2.1. Analyzing design hierarchy..
314.2.2. Executing AST frontend in derive mode using pre-parsed AST for module `\pad_ring'.
314.2.3. Executing AST frontend in derive mode using pre-parsed AST for module `\peripheral_subsystem'.
314.2.4. Executing AST frontend in derive mode using pre-parsed AST for module `\ao_peripheral_subsystem'.
314.2.5. Executing AST frontend in derive mode using pre-parsed AST for module `\memory_subsystem'.
314.2.6. Executing AST frontend in derive mode using pre-parsed AST for module `\system_bus'.
314.2.7. Executing AST frontend in derive mode using pre-parsed AST for module `\debug_subsystem'.
314.2.8. Executing AST frontend in derive mode using pre-parsed AST for module `\cpu_subsystem'.
314.2.9. Analyzing design hierarchy..
314.2.10. Executing AST frontend in derive mode using pre-parsed AST for module `\cv32e40p_register_file'.
314.2.11. Executing AST frontend in derive mode using pre-parsed AST for module `\ibex_core'.
314.2.12. Executing AST frontend in derive mode using pre-parsed AST for module `\dm_obi_top'.
314.2.13. Executing AST frontend in derive mode using pre-parsed AST for module `\dmi_jtag'.
314.2.14. Executing AST frontend in derive mode using pre-parsed AST for module `\system_xbar'.
ERROR: Module `\dma_690D0' referenced in module `\ao_peripheral_subsystem' in cell `\dma_i' is not part of the design.
Elapsed time: 0:01.35[h:]min:sec. CPU time: user 1.21 sys 0.13 (99%). Peak memory: 160220KB.
```

This is due to `dma` and other modules using types to pass as parameter. It generates a new module for each seen type combination during the `sv2v` pass but I am trying to figure out how we could solve that e.g. printing the infered module additionally.  

```sv
  dma #(
      .reg_req_t (reg_pkg::reg_req_t),
      .reg_rsp_t (reg_pkg::reg_rsp_t),
      .obi_req_t (obi_pkg::obi_req_t),
      .obi_resp_t(obi_pkg::obi_resp_t)
  ) dma_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::DMA_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::DMA_IDX]),
      .dma_master0_ch0_req_o,
      .dma_master0_ch0_resp_i,
      .dma_master1_ch0_req_o,
      .dma_master1_ch0_resp_i,
      .dma_intr_o
  );
```

after `sv2v`

```sv
	dma_690D0 dma_i(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.reg_req_i(peripheral_slv_req[420+:70]),
		.reg_rsp_o(peripheral_slv_rsp[204+:34]),
		.dma_master0_ch0_req_o(dma_master0_ch0_req_o),
		.dma_master0_ch0_resp_i(dma_master0_ch0_resp_i),
		.dma_master1_ch0_req_o(dma_master1_ch0_req_o),
		.dma_master1_ch0_resp_i(dma_master1_ch0_resp_i),
		.dma_intr_o(dma_intr_o)
	);
```

`dma_690D0` should also be printed but it is not printed at the moment.

Other examples:

```sv
	reg_demux_22C6F #(.NoPorts(core_v_mini_mcu_pkg_AO_PERIPHERALS)) reg_demux_i(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.in_select_i(peripheral_select),
		.in_req_i(peripheral_req),
		.in_rsp_o(peripheral_rsp),
		.out_req_o(peripheral_slv_req),
		.out_rsp_i(peripheral_slv_rsp)
	);
	// Trace: ../src/openhwgroup.org_systems_core-v-mini-mcu_0/hw/core-v-mini-mcu/ao_peripheral_subsystem.sv:119:3
	soc_ctrl_6DF27 soc_ctrl_i(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.reg_req_i(peripheral_slv_req[0+:70]),
		.reg_rsp_o(peripheral_slv_rsp[0+:34]),
		.boot_select_i(boot_select_i),
		.execute_from_flash_i(execute_from_flash_i),
		.use_spimemio_o(use_spimemio),
		.exit_valid_o(exit_valid_o),
		.exit_value_o(exit_value_o)
	);
```

There are like 20 occurences. I think we either have to ask him (sv2v-guy) if he can fix that for us in a nice way or we have to add the capabilty to `sv2v`.
