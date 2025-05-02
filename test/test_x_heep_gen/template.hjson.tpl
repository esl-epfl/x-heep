## Writes down all kwargs from mcu_gen, but only peripheral related ones should be checked
<%! from x_heep_gen.peripherals.abstractions import DataConfiguration %>

{
    // CPU configuration
    cpu_type: "${cpu_type}"
    % if cve2_rv32e is not None:
    cve2_rv32e: "${cve2_rv32e}"
    % endif
    % if cve2_rv32m is not None:
    cve2_rv32m: "${cve2_rv32m}"
    % endif
    external_domains: ${external_domains}

    // Memory map
    debug: {
        address: "${debug_start_address}"
        length: "${debug_size_address}"
    }

    // AO Peripherals
    ao_peripheral_start_address: "${xheep.get_base_peripherals_base_address()}"
    ao_peripheral_size_address: "${xheep.get_base_peripherals_length()}"
    ao_peripherals: {
        % for peripheral in xheep.get_base_peripherals():
        ${peripheral.get_name()}: {
            offset: ${peripheral.get_address()}
            size: ${peripheral.get_length()}
            %if isinstance(peripheral, DataConfiguration):
            config_path: ${peripheral.get_config_path()}
            %endif
        }
        % endfor
    }
    ao_peripherals_count: ${len(xheep.get_base_peripherals())}

    // DMA Configuration
    dma_ch_count: "${xheep.get_dma()[0].get_num_channels()}"
    dma_ch_size: "${xheep.get_dma()[0].get_ch_length()}"
    num_dma_master_ports: "${xheep.get_dma()[0].get_num_master_ports()}"
    num_dma_xbar_channels_per_master_port: "${xheep.get_dma()[0].get_num_channels_per_master_port()}"
    dma_xbar_masters_array: "${xheep.get_dma()[0].get_xbar_array()}"

    // Optional Peripherals
    peripheral_start_address: "${xheep.get_user_peripherals_base_address()}"
    peripheral_size_address: "${xheep.get_base_peripherals_length()}"
    peripherals: {
        % for peripheral in xheep.get_user_peripherals():
        ${peripheral.get_name()}: {
            offset: ${peripheral.get_address()}
            size: ${peripheral.get_length()}
            %if isinstance(peripheral, DataConfiguration):
            config_path: ${peripheral.get_config_path()}
            %endif
        }
        % endfor
    }
    peripherals_count: ${len(xheep.get_user_peripherals())}

    // External Slaves and Flash Memory
    ext_slaves: {
        address: "${ext_slave_start_address}"
        length: "${ext_slave_size_address}"
    }
    flash_mem: {
        address: "${flash_mem_start_address}"
        length: "${flash_mem_size_address}"
    }

    // Memory Configuration
    linker_script: {
        stack_size: "${stack_size}"
        heap_size: "${heap_size}"
    }

    // Interrupt Configuration
    interrupts: {
        used: ${plic_used_n_interrupts}
        total: ${plit_n_interrupts}
        list: ${interrupts}
    }

    // Pad Configuration
    pad_config: {
        total_pads: ${total_pad}
        total_muxed_pads: ${total_pad_muxed}
        max_mux_bits: ${max_total_pad_mux_bitlengh}
        % if pads_attributes:
        attributes: ${pads_attributes}
        % endif
    }
}
