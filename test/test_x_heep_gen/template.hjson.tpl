## Writes down all kwargs from mcu_gen, but only peripheral related ones should be checked

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
    ao_peripheral_start_address: "${ao_peripheral_start_address}"
    ao_peripheral_size_address: "${ao_peripheral_size_address}"
    ao_peripherals: {
        % for name, info in ao_peripherals.items():
        ${name}: {
            % for key, value in info.items():
            ${key}: "${value}"
            % endfor
        }
        % endfor
    }
    ao_peripherals_count: ${ao_peripherals_count}

    // DMA Configuration
    dma_ch_count: "${dma_ch_count}"
    dma_ch_size: "${dma_ch_size}"
    num_dma_master_ports: "${num_dma_master_ports}"
    num_dma_xbar_channels_per_master_port: "${num_dma_xbar_channels_per_master_port}"
    dma_xbar_masters_array: "${dma_xbar_masters_array}"

    // Optional Peripherals
    peripheral_start_address: "${peripheral_start_address}"
    peripheral_size_address: "${peripheral_size_address}"
    peripherals: {
        % for name, info in peripherals.items():
        ${name}: {
            % for key, value in info.items():
            ${key}: "${value}"
            % endfor
        }
        % endfor
    }
    peripherals_count: ${peripherals_count}

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
