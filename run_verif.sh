
# Init Conda
conda activate core-v-mini-mcu
make mcu-gen MEMORY_BANKS=8 EXTERNAL_DOMAINS=4

# Run Fusesoc
if [[ $1 = "rtl" ]]
then
    type="rtl"
    fusesoc="questasim-sim-opt-upf"
    fusesoc_flags="--flag=use_upf --flag=use_external_device_example"
    sim_folder="sim-modelsim"
    upf="RUN_UPF=1"
else
    echo -e "ERROR: Wrong parameter!\n"
    return
fi

# Run Fusesoc
make $fusesoc FUSESOC_FLAGS="${fusesoc_flags}"

# Open output file
if test -f "run_verif_${type}_log.txt";
then
    rm run_verif_${type}_log.txt
fi
touch run_verif_${type}_log.txt
echo -e "APPLICATIONS OUTPUT:\n\n" >> ./run_verif_${type}_log.txt

# Run hello_world application
echo -e "Run hello_world application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/hello_world/hello_world.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/hello_world/hello_world.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run hello_world application (exec from flash)
echo -e "Run hello_world application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/hello_world/hello_world.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/hello_world/hello_world.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run hello_world application (load from flash)
echo -e "Run hello_world application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/hello_world/hello_world.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/hello_world/hello_world.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run matadd application
echo -e "Run matadd application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/matadd/matadd.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/matadd/matadd.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run matadd application (exec from flash)
echo -e "Run matadd application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/matadd/matadd.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/matadd/matadd.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run matadd application (load from flash)
echo -e "Run matadd application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/matadd/matadd.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/matadd/matadd.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_external_peripheral application
echo -e "Run example_external_peripheral application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_external_peripheral/example_external_peripheral.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_external_peripheral/example_external_peripheral.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_external_peripheral application (exec from flash)
echo -e "Run example_external_peripheral application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_external_peripheral/example_external_peripheral.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_external_peripheral/example_external_peripheral.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_external_peripheral application (load from flash)
echo -e "Run example_external_peripheral application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_external_peripheral/example_external_peripheral.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_external_peripheral/example_external_peripheral.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_gpio_cnt application
echo -e "Run example_gpio_cnt application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_gpio_cnt/example_gpio_cnt.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_gpio_cnt/example_gpio_cnt.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_gpio_cnt application (exec from flash)
echo -e "Run example_gpio_cnt application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_gpio_cnt/example_gpio_cnt.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_gpio_cnt/example_gpio_cnt.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_gpio_cnt application (load from flash)
echo -e "Run example_gpio_cnt application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_gpio_cnt/example_gpio_cnt.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_gpio_cnt/example_gpio_cnt.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run gpio_pmw application
echo -e "Run gpio_pmw application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/gpio_pmw/gpio_pmw.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/gpio_pmw/gpio_pmw.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run gpio_pmw application (exec from flash)
echo -e "Run gpio_pmw application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/gpio_pmw/gpio_pmw.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/gpio_pmw/gpio_pmw.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run gpio_pmw application (load from flash)
echo -e "Run gpio_pmw application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/gpio_pmw/gpio_pmw.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/gpio_pmw/gpio_pmw.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run spi_host_example application (load from flash)
echo -e "Run spi_host_example application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/spi_host_example/spi_host_example.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/spi_host_example/spi_host_example.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run dma_example application
echo -e "Run dma_example application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/dma_example/dma_example.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/dma_example/dma_example.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run dma_example application (exec from flash)
echo -e "Run dma_example application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/dma_example/dma_example.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/dma_example/dma_example.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run dma_example application (load from flash)
echo -e "Run dma_example application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/dma_example/dma_example.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/dma_example/dma_example.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run spi_host_dma_example application (load from flash)
echo -e "Run spi_host_dma_example application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/spi_host_dma_example/spi_host_dma_example.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/spi_host_dma_example/spi_host_dma_example.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_core application
echo -e "Run example_power_gating_core application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_core/example_power_gating_core.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_core/example_power_gating_core.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_core application (exec from flash)
echo -e "Run example_power_gating_core application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_core/example_power_gating_core.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_core/example_power_gating_core.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_core application (load from flash)
echo -e "Run example_power_gating_core application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_core/example_power_gating_core.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_core/example_power_gating_core.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run spi_host_dma_power_gate_example application (load from flash)
echo -e "Run spi_host_dma_power_gate_example application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/spi_host_dma_power_gate_example/spi_host_dma_power_gate_example.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/spi_host_dma_power_gate_example/spi_host_dma_power_gate_example.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_periph application
echo -e "Run example_power_gating_periph application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_periph/example_power_gating_periph.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_periph/example_power_gating_periph.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_periph application (exec from flash)
echo -e "Run example_power_gating_periph application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_periph/example_power_gating_periph.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_periph/example_power_gating_periph.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_periph application (load from flash)
echo -e "Run example_power_gating_periph application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_periph/example_power_gating_periph.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_periph/example_power_gating_periph.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_ram_blocks application
echo -e "Run example_power_gating_ram_blocks application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_ram_blocks/example_power_gating_ram_blocks.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_ram_blocks/example_power_gating_ram_blocks.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_ram_blocks application (exec from flash)
echo -e "Run example_power_gating_ram_blocks application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_ram_blocks/example_power_gating_ram_blocks.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_ram_blocks/example_power_gating_ram_blocks.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_ram_blocks application (load from flash)
echo -e "Run example_power_gating_ram_blocks application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_ram_blocks/example_power_gating_ram_blocks.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_ram_blocks/example_power_gating_ram_blocks.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_set_retentive_ram_blocks application
echo -e "Run example_set_retentive_ram_blocks application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_set_retentive_ram_blocks/example_set_retentive_ram_blocks.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_set_retentive_ram_blocks/example_set_retentive_ram_blocks.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_set_retentive_ram_blocks application (exec from flash)
echo -e "Run example_set_retentive_ram_blocks application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_set_retentive_ram_blocks/example_set_retentive_ram_blocks.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_set_retentive_ram_blocks/example_set_retentive_ram_blocks.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_set_retentive_ram_blocks application (load from flash)
echo -e "Run example_set_retentive_ram_blocks application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_set_retentive_ram_blocks/example_set_retentive_ram_blocks.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_set_retentive_ram_blocks/example_set_retentive_ram_blocks.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_external application
echo -e "Run example_power_gating_external application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_external/example_power_gating_external.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_external/example_power_gating_external.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_external application (exec from flash)
echo -e "Run example_power_gating_external application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_external/example_power_gating_external.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_external/example_power_gating_external.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_power_gating_external application (load from flash)
echo -e "Run example_power_gating_external application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_power_gating_external/example_power_gating_external.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_power_gating_external/example_power_gating_external.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_set_retentive_external_ram_blocks application
echo -e "Run example_set_retentive_external_ram_blocks application...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_set_retentive_external_ram_blocks/example_set_retentive_external_ram_blocks.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_set_retentive_external_ram_blocks/example_set_retentive_external_ram_blocks.hex" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_set_retentive_external_ram_blocks application (exec from flash)
echo -e "Run example_set_retentive_external_ram_blocks application (exec from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_set_retentive_external_ram_blocks/example_set_retentive_external_ram_blocks.flash_exec.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_set_retentive_external_ram_blocks/example_set_retentive_external_ram_blocks.flash_exec.hex boot_sel=1 execute_from_flash=1" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Run example_set_retentive_external_ram_blocks application (load from flash)
echo -e "Run example_set_retentive_external_ram_blocks application (load from flash)...\n" >> ./run_verif_${type}_log.txt
cd sw/
make clean applications/example_set_retentive_external_ram_blocks/example_set_retentive_external_ram_blocks.flash_load.hex
cd ../build/openhwgroup.org_systems_core-v-mini-mcu_0/${sim_folder}/
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/applications/example_set_retentive_external_ram_blocks/example_set_retentive_external_ram_blocks.flash_load.hex boot_sel=1 execute_from_flash=0" ${upf}
cat uart0.log >> ../../../run_verif_${type}_log.txt
cd ../../../
echo -e "\n" >> ./run_verif_${type}_log.txt

# Print output file
echo -e "\n"
cat ./run_verif_${type}_log.txt
