import hjson
import structs_gen
import x_heep_gen.load_config
import x_heep_gen.peripherals
import argparse
import os

# Path to the dma file
dma_file_path = "./sw/device/lib/drivers/dma/dma_structs.h"

# Path to the header_structs template
template_path = "./sw/device/lib/drivers/template.tpl"

# path in which the structs header files are generated, has to be formatted with
# the name of the peripheral
out_files_base_path = "./sw/device/lib/drivers/{}/{}_structs.h"


JSON_FILES = []  # list of the peripherals' json files
OUTPUT_FILES = []  # list of the output filenames
PERIPHERAL_NAMES = []  # list of the peripherals' names


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


def scan_peripherals_hjson(json_list):
    for p in json_list:
        for field in json_list[p]:
            if field == "path":
                add_peripheral(p, json_list[p]["path"])


def scan_peripherals_python(peripherals):
    """
    It loops over the peripherals and adds the ones with a configuration path

    :param peripherals: The peripherals to scan, list of Peripheral objects
    """
    for p in peripherals:
        if isinstance(p, x_heep_gen.peripherals.DataConfiguration):
            add_peripheral(p.get_name(), p.get_configpath())


def format_dma_channels(file_path, new_string):

    try:
        # Read the contents of the file
        with open(file_path, "r") as file:
            content = file.read()

        # Replace 'DMA_START_ADDRESS' with 'new_address'
        updated_content = content.replace(
            "#define dma_peri ((volatile dma *) DMA_START_ADDRESS)", new_string
        )

        # Write the updated content back to the file
        with open(file_path, "w") as file:
            file.write(updated_content)

        print("DMA channel has been successfully updated.")

    except FileNotFoundError:
        print(f"The file {file_path} does not exist.")
    except Exception as e:
        print(f"An error occurred: {str(e)}")

    # for i in range(len(JSON_FILES)):
    #     print("{}\n{}\n{}\n\n\n".format(PERIPHERAL_NAMES[i], JSON_FILES[i], OUTPUT_FILES[i]))


if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--cfg_peripherals",
        metavar="file",
        type=str,
        required=False,
        help="X-Heep general configuration, if not provided, the default value ./mcu_cfg.hjson will be used",
    )

    args = parser.parse_args()

    # hjson file from which the peripherals info are taken
    if args.cfg_peripherals != None:
        mcu_cfg_file = str(args.cfg_peripherals)
    else:
        print(
            "Warning: No configuration file provided, using default value ./mcu_cfg.hjson"
        )
        mcu_cfg_file = "./mcu_cfg.hjson"  # default value

    if not os.path.exists(mcu_cfg_file):
        exit(f"Error: Configuration file {mcu_cfg_file} not found")

    if mcu_cfg_file.endswith(".py"):
        x_heep = x_heep_gen.load_config.load_cfg_script_file(mcu_cfg_file)

        base_peripherals = x_heep.get_base_peripheral_domain().get_peripherals()
        user_peripherals = x_heep.get_user_peripheral_domain().get_peripherals()

        scan_peripherals_python(base_peripherals)
        scan_peripherals_python(user_peripherals)

    elif mcu_cfg_file.endswith(".hjson"):
        # Open the json file adn takes the data
        with open(str(mcu_cfg_file)) as f:
            data = hjson.load(f)

        # Get the info relative to the various peripherals
        base_peripherals = data["ao_peripherals"]
        user_peripherals = data["peripherals"]

        # loops through the peripherals and add the useful ones
        scan_peripherals_hjson(base_peripherals)
        scan_peripherals_hjson(user_peripherals)

    else:
        exit("Error: Invalid configuration file, only .hjson and .py are supported")

    # Call the generation script, once for every peripheral
    for i in range(len(JSON_FILES)):
        structs_gen.main(
            [
                "--template_filename",
                template_path,
                # "--peripheral_name", PERIPHERAL_NAMES[i],
                "--json_filename",
                JSON_FILES[i],
                "--output_filename",
                OUTPUT_FILES[i],
            ]
        )

    new_string = "#define dma_peri(channel) ((volatile dma *) (DMA_START_ADDRESS + DMA_CH_SIZE * channel))"
    format_dma_channels(dma_file_path, new_string)
