## Writes down all kwargs from mcu_gen, but only peripheral related ones should be checked
<%! from x_heep_gen.peripherals.abstractions import DataConfiguration %>

<%
    user_peripheral_domain = xheep.get_user_peripheral_domain()
    base_peripheral_domain = xheep.get_base_peripheral_domain()
    dma = base_peripheral_domain.get_dma()
    pdm2pcm = user_peripheral_domain.get_pdm2pcm()
%>

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
    ao_peripheral_start_address: "${base_peripheral_domain.get_start_address()}"
    ao_peripheral_size_address: "${base_peripheral_domain.get_length()}"
    ao_peripherals: {
        % for peripheral in base_peripheral_domain.get_peripherals():
        ${peripheral.get_name()}: {
            offset: ${peripheral.get_address()}
            size: ${peripheral.get_length()}
            %if isinstance(peripheral, DataConfiguration):
            config_path: ${peripheral.get_config_path()}
            %endif
        }
        % endfor
    }
    ao_peripherals_count: ${len(base_peripheral_domain.get_peripherals())}

    // DMA Configuration
    dma_ch_count: "${dma.get_num_channels()}"
    dma_ch_size: "${dma.get_ch_length()}"
    num_dma_master_ports: "${dma.get_num_master_ports()}"
    num_dma_xbar_channels_per_master_port: "${dma.get_num_channels_per_master_port()}"
    fifo_depth: "${dma.get_fifo_depth()}"
    addr_mode_en: "${dma.get_addr_mode()}"
    subaddr_mode_en: "${dma.get_subaddr_mode()}"
    hw_fifo_mode_en: "${dma.get_hw_fifo_mode()}"
    zero_padding_en: "${dma.get_zero_padding()}"
    dma_xbar_masters_array: "${dma.get_xbar_array()}"

    // Optional Peripherals
    peripheral_start_address: "${user_peripheral_domain.get_start_address()}"
    peripheral_size_address: "${user_peripheral_domain.get_length()}"
    peripherals: {
        % for peripheral in user_peripheral_domain.get_peripherals():
        ${peripheral.get_name()}: {
            offset: ${peripheral.get_address()}
            size: ${peripheral.get_length()}
            %if isinstance(peripheral, DataConfiguration):
            config_path: ${peripheral.get_config_path()}
            %endif
        }
        % endfor
    }
    peripherals_count: ${len(user_peripheral_domain.get_peripherals())}

    %if pdm2pcm != None:
    // PDM2PCM configuration
        %if pdm2pcm.get_cic_mode():
    pdm2pcm_cic_only: 0x1
        %else:
    pdm2pcm_cic_only: 0x0
        %endif
    %endif

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
