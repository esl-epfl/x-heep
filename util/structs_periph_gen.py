import hjson
import structs_gen

# Path to the dma file
dma_file_path = "./sw/device/lib/drivers/dma/dma_structs.h" 

# Path to the header_structs template
template_path = "./sw/device/lib/drivers/template.tpl"

# hjson file from which the peripherals info are taken
mcu_cfg_file = "./mcu_cfg.hjson"

# path in which the structs header files are generated, has to be formatted with
# the name of the peripheral
out_files_base_path = "./sw/device/lib/drivers/{}/{}_structs.h" 


JSON_FILES = []         # list of the peripherals' json files
OUTPUT_FILES = []       # list of the output filenames
PERIPHERAL_NAMES = []   # list of the peripherals' names


"""
    Given the name of a peripheral and the path to its descriptive json file,
    it appends the info relative to the peripheral to the three lists used
    for generation
"""
def add_peripheral(name, path):
    JSON_FILES.append(path)
    OUTPUT_FILES.append(out_files_base_path.format(name, name))
    # PERIPHERAL_NAMES.append(name)


"""
    It loops over the json containing infos of several peripherals.
    When it encounters one with the "path" field, it adds it to the 
    lists
"""
def scan_peripherals(json_list):
    for p in json_list:
        for field in json_list[p]:
            if(field == 'path'):
                add_peripheral(p, json_list[p]["path"])


def format_dma_channels(file_path, new_string):
    
    try:
        # Read the contents of the file
        with open(file_path, 'r') as file:
            content = file.read()
        
        # Replace 'DMA_START_ADDRESS' with 'new_address'
        updated_content = content.replace('#define dma_peri ((volatile dma *) DMA_START_ADDRESS)', new_string)
        
        # Write the updated content back to the file
        with open(file_path, 'w') as file:
            file.write(updated_content)
        
        print("DMA channel has been successfully updated.")
        
    except FileNotFoundError:
        print(f"The file {file_path} does not exist.")
    except Exception as e:
        print(f"An error occurred: {str(e)}")

    # for i in range(len(JSON_FILES)):
    #     print("{}\n{}\n{}\n\n\n".format(PERIPHERAL_NAMES[i], JSON_FILES[i], OUTPUT_FILES[i]))

if __name__ == "__main__":

    # Open the json file adn takes the data
    with open(mcu_cfg_file) as f:
        data = hjson.load(f)

    # Get the info relative to the various peripherals
    ao_peripherals = data["ao_peripherals"]
    peripherals = data["peripherals"]

    # loops through the peripherals and add the useful ones
    scan_peripherals(ao_peripherals)
    scan_peripherals(peripherals)

    # Call the generation script, once for every peripheral
    for i in range(len(JSON_FILES)):
        structs_gen.main([ "--template_filename", template_path,
                                # "--peripheral_name", PERIPHERAL_NAMES[i],
                                "--json_filename", JSON_FILES[i], 
                                "--output_filename", OUTPUT_FILES[i]]
                            )
    
    new_string = "#define dma_peri(channel) ((volatile dma *) (DMA_START_ADDRESS + DMA_CH_SIZE * channel))"
    format_dma_channels(dma_file_path, new_string)