import argparse
import pathlib
import pickle
from typing import Dict, List

from reggen.ip_block import IpBlock
import structs_gen
from x_heep_gen.system import XHeep
from x_heep_gen.peripherals.peripheral_helper import ip_block_paths

# Path to the header_structs template
template_path = "./sw/device/lib/drivers/template.tpl"


# path in which the structs header files are generated, has to be formatted with
# the name of the peripheral
out_files_base_path = "./sw/device/lib/drivers/{}/{}_structs.h" 



def mk_out(file: str) -> str:
    ip_block = IpBlock.from_path(file, [])
    name = ip_block.name.lower()
    return out_files_base_path.format(name, name)




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