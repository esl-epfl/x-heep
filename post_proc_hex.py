import sys

def filter_verilog(input_filename, output_filename, max_address="0x50000000"):

    max_address_hex = int(max_address, 16)

    try:
        with open(input_filename, 'r') as input_file:
            verilog_lines = input_file.readlines()

        filtered_lines = []
        # Filter out lines with addresses above max_address
        for line in verilog_lines:
            if line.startswith('@'):
                address = int(line[1:].split()[0], 16)
                if address >= max_address_hex:
                    print(f"address {hex(address)} >= max_address {hex(max_address_hex)}")
                    break
            filtered_lines.append(line)

        with open(output_filename, 'w') as output_file:
            output_file.writelines(filtered_lines)

        print(f"Filtered Verilog saved to {output_filename}")
    except FileNotFoundError:
        print(f"Error: File '{input_filename}' not found.")

if len(sys.argv) != 3:
        print("Usage: python post_proc_hex.py <arg1> <arg2>")
        sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]

filter_verilog(input_file, output_file)
