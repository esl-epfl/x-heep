import pickle
import os
import x_heep_gen
from x_heep_gen.pads import Pad
import hjson
from jsonref import JsonRef
import sys


data = pickle.load(open(os.path.join(os.path.dirname(__file__), "./golden_cache/xheep_cache.pickle"), "rb"))
new_cache = pickle.load(open(os.path.join(os.path.dirname(__file__), "../build/xheep_cache.pickle"), "rb"))
keys = list(data.keys())
keys.remove("xheep")
for key in keys:
    print(key)
    
    #verify equality with ../build/xheep_cache.pickle
    pad_cache = data[key]
    new_cache_data = new_cache[key]
    
    if pad_cache != new_cache_data:
        print(f"Mismatch in key {key}")
        print("Old cache:", pad_cache,type(pad_cache))
        print("New cache:", new_cache_data,type(new_cache_data))
        sys.exit(1)
print("All keys match!")


        
#print(pad_cfg)
