import struct
import sys

if len(sys.argv) != 2:
    print("Usage: script.py hex_filename")
    sys.exit(-1)


filename = sys.argv[1]
filename_out = "output.hex"

with open (filename, "rb") as input_file:
    with open(filename_out, "wb") as output_file:
        while(word:=input_file.read(4)):
            if len(word) < 4: print("Final word not 4B")
            unpacked = struct.unpack(">L", word)
            output_file.write(struct.pack("<L", unpacked[0]))


