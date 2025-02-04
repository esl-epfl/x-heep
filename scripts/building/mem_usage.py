def parse_map_file(file_path):
    sections = {}
    with open(file_path, 'r') as file:
        collect = False
        for line in file:
            if line.startswith('.comment'): break
            if line.startswith('.text') or line.startswith('.data') or line.startswith('.bss'):
                collect = True
            if collect:
                if line.strip() == '':  # Stop collecting when there's a blank line
                    collect = False
                parts = line.split()
                if len(parts) >= 3 and parts[1].startswith('0x'):
                    section = parts[0]
                    address = int(parts[1], 16)
                    try:
                        size = int(parts[2], 16)
                    except ValueError:
                        continue  # Skip lines where the third part is not a size
                    if section in sections:
                        sections[section]['size'] += size
                    else:
                        sections[section] = {'address': address, 'size': size}
    return sections

def parse_memory_regions(file_path):
    regions = {}
    try:
        with open(file_path, 'r') as file:
            collect = False
            for line in file:
                if "Name" in line and "Origin" in line and "Length" in line:
                    collect = True
                    continue
                if collect:
                    if line.strip() == '':
                        collect = False  # Stop collecting when a blank line is encountered
                        continue
                    parts = line.split()
                    if len(parts) >= 4:
                        name = parts[0]
                        origin = int(parts[1], 16)
                        length = int(parts[2], 16)
                        attributes = parts[3]
                        regions[name] = {'origin': origin, 'length': length, 'attributes': attributes}
    except FileNotFoundError:
        print("File not found. Please check the path and try again.")
    return regions

def parse_sv_file(file_path):
    num_banks = 0
    num_il_banks = 0
    try:
        with open(file_path, 'r') as file:
            for line in file:
                if "NUM_BANKS =" in line:
                    num_banks = int(line.split('=')[1].strip().strip(';'))
                elif "NUM_BANKS_IL =" in line:
                    num_il_banks = int(line.split('=')[1].strip().strip(';'))
    except FileNotFoundError:
        print("File not found. Please check the path and try again.")
    return num_banks, num_il_banks

import re
def find_hex_numbers(file_path):
    # Pattern to match hex numbers of specific length (12 digits after "0x")
    pattern = re.compile(r'0x[0-9A-Fa-f]{16}\b')
    occurrences = []

    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith('.comment'): break
            matches = pattern.findall(line)
            if matches:
                occurrences.extend(matches)

    return occurrences

def extract_ildt_length(file_path):
    # Regex to match `.data_interleaved` followed by any amount of non-printable characters,
    # then a long hex number, more non-printable characters, and finally a short hex number.
    pattern = re.compile(r'\.data_interleaved\s+0x[0-9A-Fa-f]+\s+0x[0-9A-Fa-f]+')
    ildt_length     = None
    ildt_address    = None

    with open(file_path, 'r') as file:
        content = file.read()  # Read the whole file into a single string
        matches = pattern.findall(content)
        if matches:
            last_match = matches[-1]  # Get the last match
            parts = last_match.split()
            ildt_length = parts[-1]  # Extract the last hex number from the last match
            ildt_address = parts[-2]
    return ildt_address, ildt_length


num_banks, num_il_banks = parse_sv_file('hw/core-v-mini-mcu/include/core_v_mini_mcu_pkg.sv')
total_size_B = (num_banks)*(32*1024)
print(f"Total space: {total_size_B/1024:0.1f} kB = {num_banks-num_il_banks}x32 kB + Interleaved {num_il_banks} x32 kB")

# For the time being we assume all banks are 32 kB large
bank_sizes_B = [32*1024] * (num_banks)

banks = []
for i in range(num_banks):
    bank = {
        'type'  : "Cont" if i<(num_banks-num_il_banks) else "IntL",
        'size'  : bank_sizes_B[i],

    }
    banks.append(bank)


regions = parse_memory_regions('sw/build/main.map')
regions['code'] = regions.pop('ram0')
regions['data'] = regions.pop('ram1')
try:
    regions['ildt'] = regions.pop('ram2')
except KeyError:
    regions['ildt'] = {'origin': regions['data']['origin'] + regions['data']['length'], 'length':0}

regions['code']['origin_B'] = regions['code']['origin']
regions['code']['length_B'] = regions['code']['length']
regions['data']['origin_B'] = regions['data']['origin']
regions['data']['length_B'] = regions['data']['length']
regions['ildt']['origin_B'] = regions['ildt']['origin']
regions['ildt']['length_B'] = regions['ildt']['length']

addr_ildt_hex, size_ildt_hex   = extract_ildt_length('sw/build/main.map')

hexs = find_hex_numbers('sw/build/main.map')
try:
    last_valid_address_idx = hexs[::-1].index(addr_ildt_hex)
    hexs = hexs[:-last_valid_address_idx]   
    size_ildt_B = int(size_ildt_hex,16)
except:
    size_ildt_B = 0

hexs = [ int(h,16) if h != '0xffffffffffffffff' else 0 for h in hexs]

size_code_B = max( [ h if h < regions['data']['origin_B'] else 0 for h in hexs ]  ) - regions['code']['origin_B']
size_data_B = max( [ h if h < regions['ildt']['origin_B'] else 0 for h in hexs ]  ) - regions['data']['origin_B']


print( "Region \t Start \tEnd\tSize(kB)\tUsed(kB)\tUtilz(%) ")
print(f"Code:  \t{regions['code']['origin_B']/1024:5.1f}\t{(regions['code']['origin_B']+regions['code']['length_B'])/1024:5.1f}\t{regions['code']['length_B']/1024:5.1f}\t\t{size_code_B/1024:0.1f}\t\t{100*size_code_B/regions['code']['length_B']:0.1f}")
print(f"Data:  \t{regions['data']['origin_B']/1024:5.1f}\t{(regions['data']['origin_B']+regions['data']['length_B'])/1024:5.1f}\t{regions['data']['length_B']/1024:5.1f}\t\t{size_data_B/1024:0.1f}\t\t{100*size_data_B/regions['data']['length_B']:0.1f}")
if num_il_banks:
    print(f"ILdata:\t{regions['ildt']['origin_B']/1024:5.1f}\t{(regions['ildt']['origin_B']+regions['ildt']['length_B'])/1024:5.1f}\t{regions['ildt']['length_B']/1024:5.1f}\t\t{size_ildt_B/1024:0.1f}\t\t{100*size_ildt_B/regions['ildt']['length_B']:0.1f}")


char = '.'
address = 0
granularity_B = 1024

# Switch these two lines to choose if to display the correct start of the data or the
# "de facto" start, displaced by the code (in case the code size exceeds the designated length_B
data_start_B = max(regions['data']['origin_B'],regions['code']['origin_B'] + size_code_B)
data_start_B =  regions['data']['origin_B']

print("")

for i, bank in enumerate(banks):
    bank['use'] = ['-']*int((bank['size']/granularity_B))
    utilization = 0

    if bank['type'] == 'Cont':
        for piece in range(len(bank['use'])):
            address += granularity_B
            if address <= regions['code']['origin_B'] + size_code_B:
                if address <= regions['code']['origin_B'] + regions['code']['length_B']:
                    bank['use'][piece] = 'C'
                else:
                    bank['use'][piece] = '!'
                utilization += granularity_B

            elif address >= data_start_B  and address <= data_start_B + size_data_B:
                if address <= data_start_B + regions['data']['length_B']:
                    bank['use'][piece] = 'd'
                else:
                    bank['use'][piece] = '!'
                utilization += granularity_B
            else:
                bank['use'][piece] = '-'

    if bank['type'] == "IntL":
        size_per_il_bank = int((size_ildt_B/num_il_banks)/granularity_B)
        for piece in range(size_per_il_bank):
            bank['use'][piece] = 'd'
        for piece in range(size_per_il_bank, len(bank['use'])):
            bank['use'][piece] = '-'


    bank['use'] = ''.join([''.join(sublist) for sublist in bank['use']])
    print(bank['type'],i,bank['use'], f"\t{100*(utilization/bank['size']):0.1f}%")

