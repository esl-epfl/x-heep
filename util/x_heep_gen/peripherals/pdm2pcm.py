from typing import Dict, List
from .peripheral_helper import peripheral_from_file


@peripheral_from_file("./hw/ip/pdm2pcm/data/pdm2pcm.hjson")
class Pdm2PcmPeripheral():
    """
    A class representing a `pdm2pcm` peripheral.
    """
    def get_io_prefix(self) -> str:
        return ""
    
    def get_io_connections(self) -> List[Dict]:
        d = [
            {"name": "pdm", "type": "input", "width": 1},
            {"name": "pdm_clk" , "type": "output_no_en", "width": 1},
        ]
        return d