import argparse
import pathlib
import pickle
from typing import Dict, List

from reggen.ip_block import IpBlock
import structs_gen
from x_heep_gen.system import XHeep
from x_heep_gen.peripherals.peripheral_helper import ip_block_paths

# Path to the dma file
dma_file_path = "./sw/device/lib/drivers/dma/dma_structs.h" 

# Path to the header_structs template
template_path = "./sw/device/lib/drivers/template.tpl"


# path in which the structs header files are generated, has to be formatted with
# the name of the peripheral
out_files_base_path = "./sw/device/lib/drivers/{}/{}_structs.h" 



def mk_out(file: str) -> str:
    ip_block = IpBlock.from_path(file, [])
    name = ip_block.name.lower()
    return out_files_base_path.format(name, name)


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


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="structs_periph_gen")
    
    parser.add_argument("--infile",
                        "-i",
                        type=pathlib.Path,
                        required=True,
                        help="config data object")

    args = parser.parse_args()

    with open(args.infile, "rb") as f:
        kwargs = pickle.load(f)

    system: XHeep = kwargs["xheep"]

    ip_files: Dict[str, List[str]] = dict()
    for path in ip_block_paths:
        ip_files[path] = []
    
    for domain in system.iter_peripheral_domains():
        for periph in domain.iter_peripherals():
            path = periph.get_ip_path()
            if path is not None:
                ip_files.setdefault(path, []).append(periph.full_name)

    # Call the generation script, once for every peripheral
    for file, periphs in ip_files.items():
        print(f"Generate file {mk_out(file)} from {file}")
        structs_gen.main([ "--template_filename", template_path,
                                "--peripheral_name", *periphs,
                                "--json_filename", file, 
                                "--output_filename", mk_out(file)]
                            )
    
    new_string = "#define dma_peri(channel) ((volatile dma *) (DMA_START_ADDRESS + DMA_CH_SIZE * channel))"
    format_dma_channels(dma_file_path, new_string)
