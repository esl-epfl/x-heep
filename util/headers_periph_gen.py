import structs_gen

# Generates the header file with the structs for every peripheral specified by a json file

template_path = "../sw/device/lib/drivers/template.tpl"     # template used for the output
json_files = []                                             # list of all the json files
output_files = []                                           # list of all the output file names

# gpio
json_files.append("../hw/vendor/lowrisc_opentitan/hw/ip/gpio/data/gpio.hjson")
output_files.append("../sw/device/lib/drivers/gpio/gpio_structs.h")

# i2c
json_files.append("../hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson")
output_files.append("../sw/device/lib/drivers/i2c/i2c_structs.h")

# rv_plic
json_files.append("../hw/vendor/lowrisc_opentitan/hw/ip/rv_plic/data/rv_plic.hjson")
output_files.append("../sw/device/lib/drivers/rv_plic/rv_plic_structs.h")

# rv_timer
json_files.append("../hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson")
output_files.append("../sw/device/lib/drivers/rv_timer/rv_timer_structs.h")

# uart
json_files.append("../hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson")
output_files.append("../sw/device/lib/drivers/uart/uart_structs.h")

# spi_host
json_files.append("../hw/vendor/lowrisc_opentitan_spi_host/data/spi_host.hjson")
output_files.append("../sw/device/lib/drivers/spi_host/spi_host_structs.h")

# spi_device
# json_files.append("../hw/vendor/lowrisc_opentitan/hw/ip/spi_device/data/spi_device.hjson")
# output_files.append("../sw/device/lib/drivers/spi_memio/spi_memio_structs.h")


for i in range(len(json_files)):

    structs_gen.main([  "--template_filename", template_path,
                        "--json_filename", json_files[i], 
                        "--output_filename", output_files[i]])