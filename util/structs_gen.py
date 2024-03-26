import hjson
from  math import ceil
import string
import argparse
import sys
from datetime import date

############################################################
#  This module generates the structures for the registers  #  
#  of a peripheral and writes them into a file formatted   #
#  using a template.                                       #
############################################################


# Bit length of each register
reg_length = 32

# Entry name for the reserved bits
reserved_name = "_reserved"

# Tab definition as 4 blank spaces #
tab_spaces = "  "

# ENUM definitions #
enum_start = "typedef enum {}_enum {{\n"
enum_end = "}} {}_t;\n\n"

# UNION definitions #
union_start = tab_spaces + "union\n" + tab_spaces + "{\n"
union_end = tab_spaces + "}} {};\n\n"

# STRUCT definitions #
struct_typedef_start = (2 * tab_spaces) + "struct \n" + (
            2 * tab_spaces) + "{{\n"  # used to define a new struct and format the name
struct_entry = (3 * tab_spaces) + "{}" + "{}" + ":{}"  # type, name and amount of bits
struct_typedef_end = (2 * tab_spaces) + "} b ;"  # define the end of the new struct definition and the format for the new type-name

# Documentation comments definitions #
comment_align_space = 50
line_comment_start = "/*!< "
line_comment_end = "*/"
struct_comment = "Structure used for bit access"
word_comment = "Type used for word access"


def read_json(json_file):
    """
    Opens the json file taken as input and returns its content
    """
    # Open the hjson file #
    f = open(json_file)
    j_data = hjson.load(f)
    f.close()
    return j_data


def write_template(tpl, structs, enums, struct_name):
    """
    Opens a given template and substitutes the structs and enums fields.
    Returns a string with the content of the updated template
    """

    lower_case_name = struct_name.lower()
    upper_case_name = struct_name.upper()
    start_addr_def = "{}_peri ((volatile {} *) {}_START_ADDRESS)".format(lower_case_name, struct_name, upper_case_name)
    
    today = date.today()
    today = today.strftime("%d/%m/%Y")

    # To print the final result into the template
    with open(tpl) as t:
        template = string.Template(t.read())

    return template.substitute( structures_definitions=structs, 
                                enums_definitions=enums, 
                                peripheral_name=struct_name, 
                                peripheral_name_upper=upper_case_name, 
                                date=today,
                                start_address_define=start_addr_def)


def write_output(out_file, out_string):
    """
    Writes the final out_string into the specified out_file
    """

    with open(out_file, "w") as f:
        f.write(out_string)


def generate_enum(enum_field, name):
    """
    Generates an enum with the values specified.
    The enum is generated basing on the 'enum_field' parameter that contains all the values and names
    :param enum_field: list containing, for each entry, the value, the name and the description for each enum field
    :param name: name of the register field associated to the enum
    :return: the string containing the formatted enum
    """
    enum = enum_start.format(name)

    # first_entry = True
    last_entry = False

    for key in enum_field:
        enum += tab_spaces + format(key["name"], "<15") + "=" + tab_spaces + str(key["value"])
        if not last_entry:
            enum += ","
        # if first_entry:
        #     enum += tab_spaces + format(key["name"], "<15") + "=" + tab_spaces + str(key["value"])
        #     first_entry = False
        # else:
        #     enum += ",\n" + tab_spaces + format(key["name"], "<15") + "=" + tab_spaces + str(key["value"])

        if "desc" in key:
            enum += format("", "<25") + line_comment_start + format(key["desc"].replace("\n", " "), "<100") + line_comment_end

        enum += "\n"

    enum += enum_end.format(name)

    return enum


def count_bits(bits_range):
    """
    Used to determine if the "bits_range" input string contains only a single bit index or a range of bits.
    In the latter case the range is supposed to be identified with the following format "end_bit:start_bit".
    Ex: "7:0" will correspond to 8 bits from 0 to 7.
    It returns the amount of bits specified in the range

    :param bits_range: string containing the nuber of bit (or range or bits) of a specific field
    :return: the amount of bits
    """
    if bits_range.find(":") != -1:
        start_bit = bits_range.split(":")[1]
        end_bit = bits_range.split(":")[0]
        return int(end_bit) - int(start_bit) + 1
    else:
        return 1


def select_type(amount_of_bits):
    """
    Used to select the C type to give to a specific bit field. The type depends on the amount of bits
    that the filed has.

    :param amount_of_bits: amount of bits of the field to which to assign a type
    :return: the string containing the type selected
    """
    if 1 <= amount_of_bits < 9:
        return "uint8_t"
    elif 9 <= amount_of_bits < 17:
        return "uint16_t"
    elif 17 <= amount_of_bits < 33:
        return "uint32_t"
    elif 33 <= amount_of_bits < 65:
        return "uint64_t"


def intr_regs_auto_gen():
    """
    Generate hardcoded registers for interrupts

    :return: string with the register varibles to be added to the struct
    """
    res  = ""
    line = tab_spaces + "uint32_t {};".format("INTR_STATE")
    reg_comment = line_comment_start + "Interrupt State Register" + line_comment_end + "\n\n"
    res += line.ljust(comment_align_space) + reg_comment

    line = tab_spaces + "uint32_t {};".format("INTR_ENABLE")
    reg_comment = line_comment_start + "Interrupt Enable Register" + line_comment_end + "\n\n"
    res += line.ljust(comment_align_space) + reg_comment

    line = tab_spaces + "uint32_t {};".format("INTR_TEST")
    reg_comment = line_comment_start + "Interrupt Test Register" + line_comment_end + "\n\n"
    res += line.ljust(comment_align_space) + reg_comment

    return res


def alert_regs_auto_gen():
    """
    Generate hardcoded registers for alerts

    :return: string with the register varibles to be added to the struct
    """
    res = ""
    
    line = tab_spaces + "uint32_t {};".format("ALERT_TEST")
    reg_comment = line_comment_start + "Alert Test Register" + line_comment_end + "\n\n"
    res += line.ljust(comment_align_space) + reg_comment

    return res


def add_fields(register_json):
    """
    Loops through the fields of the json of a register, passed as parameter.
    Returns the structs and enums entries relative to the register, already
    indented.

    :param register_json: the json-like description of a register
    :return: the strings of the the struct fields, the enum (if present)
    """

    struct_fields = ""
    enum = ""
    bits_counter = 0  # to count the bits used for all the fields of the register

    # loops through the fields of the register
    for field in register_json["fields"]:
        field_bits = count_bits(field["bits"])
        field_type = select_type(field_bits)

        # Check if there is an ENUM, if yes it generates it and set the type of the associated field
        if "enum" in field:
            field_type = "{}_t".format(field["name"])
            enum += generate_enum(field["enum"], field["name"])

        bits_counter += int(field_bits)

        # Handles the case in which the field has no name (it's given the same name as the register)
        if "name" in field:
            field_name = field["name"]
        else:
            field_name = register_json["name"]

        # insert a new struct in the structs string
        struct_fields += struct_entry.format(format(field_type, "<15"), format(field_name, "<20"),
                                                    format(str(field_bits) + ";", "<5"))

        # if there is a description, it adds a comment
        if "desc" in field:
            struct_fields += line_comment_start + format("bit: {}".format(field["bits"]), "<10") + format(
                field["desc"].replace("\n", " "), "<100") + line_comment_end
        struct_fields += "\n"
                    
    # add an entry for the reserved bits (if present)
    if bits_counter < reg_length:
        reserved_bits = reg_length - bits_counter
        reserved_type = select_type(reserved_bits)
        struct_fields += struct_entry.format(format(reserved_type, "<15"), format(reserved_name, "<20"),
                                                   format(str(reserved_bits) + ";", "<5"))
        struct_fields += "\n"

    return struct_fields, enum


def add_registers(peripheral_json):
    """
    Reads the json description of a peripheral and generates structures for every
    register.

    :param peripheral_json: the json-like description of the registers of a peripheral
    :return: the strings containing the indented structs and enums relative to the registers
    """

    reg_struct = "\n"
    reg_enum = ""

    # number of "reserved" fields. Used to name them with a progressive ID
    num_of_reserved = 0 

    # Keeps track of the offset in Bytes from the base address of the peripheral.
    # It is usefult to compute how many Bytes to reserve in case a "skipto"
    # keywork is encountered
    bytes_offset = 0 

    
    # To handle INTR specific registers #
    if "interrupt_list" in peripheral_json:
        if 'no_auto_intr_regs' not in peripheral_json:
            reg_struct += intr_regs_auto_gen()

    # To handle the ALERT registers #
    if "alert_list" in peripheral_json:
        if "no_auto_alert_regs" not in peripheral_json:
            reg_struct += alert_regs_auto_gen()

    # loops through the registers of the hjson
    for elem in peripheral_json['registers']:

        # check and handle the multireg case
        if "multireg" in elem:
            multireg = elem["multireg"]
            count_var = multireg["count"]

            # search the multireg count default value
            # This is the number of bitfields needed
            for p in peripheral_json["param_list"]:
                if count_var == p["name"]:
                    count = int(p["default"])
            
            # counts the bits needed by the multireg register
            n_bits = 0
            for f in multireg["fields"]:
                n_bits += count_bits(f["bits"])

            # computes the number of registers needed to pack all the bit fields needed
            n_multireg = ceil((count * n_bits) /  int(peripheral_json["regwidth"]))
            
            # generate the multiregisters
            for r in range(n_multireg):
                reg_name = multireg["name"] + str(r)
                line = tab_spaces + "uint32_t {};".format(reg_name)
                reg_comment = line_comment_start + multireg["desc"].replace("\n", " ") + line_comment_end + "\n\n"
                reg_struct += line.ljust(comment_align_space) + reg_comment
                bytes_offset += 4   # one register is 4 bytes

        # check and handle the "window" case
        elif "window" in elem:
            
            window = elem["window"]
            
            validbits = int(window["validbits"])

            line = tab_spaces + "{} {};".format(select_type(validbits), window["name"])
            reg_comment = line_comment_start + window["desc"].replace("\n", " ") + line_comment_end + "\n\n"
            reg_struct += line.ljust(comment_align_space) + reg_comment
            

        # if no multireg or window, just generate the reg
        elif "name" in elem:   
            
            line = tab_spaces + "uint32_t {};".format(elem["name"])
            reg_comment = line_comment_start + elem["desc"].replace("\n", " ") + line_comment_end + "\n\n"
            reg_struct += line.ljust(comment_align_space) + reg_comment
            bytes_offset += 4       # in order to properly generate subsequent "multireg cases"

        if "skipto" in elem:
            new_address = elem["skipto"]

            # check if the new address is in hexadecimal or decimal
            # and convert it to decimal
            if(new_address[:2] == "0x"):
                new_address = int(new_address, base=16)
            else:
                new_address = int(new_address)

            offset_value = int((new_address - bytes_offset) / 4)
            
            line = tab_spaces + "uint32_t _reserved_{}[{}];".format(num_of_reserved, int(offset_value))
            reg_comment = line_comment_start + "reserved addresses" + line_comment_end + "\n\n"
            reg_struct += line.ljust(comment_align_space) + reg_comment
            bytes_offset += offset_value * 4
            num_of_reserved += 1


            ## OLD VERSION WITH UNION AND BIT FIELDS ##
            # reg_struct += union_start + struct_typedef_start.format(elem["name"])

            # # generate the struct entries relative to the fields of the register
            # new_field, new_enum = add_fields(elem)
            # reg_struct += new_field
            # reg_enum += new_enum

            # reg_struct += struct_typedef_end
            # reg_struct += format(line_comment_start, ">43") + format(struct_comment, "<100") + "*/\n"

            # reg_struct += (2 * tab_spaces) + "uint32_t" + " w;"
            # reg_struct += format(line_comment_start, ">37") + format(word_comment, "<110") + "*/\n"

            # reg_struct += union_end.format(elem["name"])
    
    return reg_struct, reg_enum


# def gen(input_template, input_hjson_file):
# if __name__ == '__main__':
def main(arg_vect):

    parser = argparse.ArgumentParser(prog="Structure generator",
                                     description="Given a template and a json file as input, it generates "
                                                 "suitable structs and enums and prints them into a file, following the "
                                                 "structure provided by the template.")
    parser.add_argument("--template_filename",
                        help="filename of the template for the final file generation")
    # parser.add_argument("--peripheral_name",
    #                     help="name of the peripheral for which the structs are generated")
    parser.add_argument("--json_filename",
                        help="filename of the input json basing on which the structs and enums will begenerated")
    parser.add_argument("--output_filename",
                        help="name of the file in which to write the final formatted template with the structs "
                             "and enums generated")

    args = parser.parse_args(arg_vect)

    input_template = args.template_filename
    input_hjson_file = args.json_filename
    output_filename = args.output_filename
    # peripheral_name = args.peripheral_name

    data = read_json(input_hjson_file)

    # Two strings used to store all the structs and enums #
    structs_definitions = "typedef struct {\n"      # used to store all the struct definitions to write in the template in the end
    enums_definitions = ""                          # used to store all the enums definitions, if present

    # START OF THE GENERATION #

    reg_structs, reg_enums = add_registers(data)
    structs_definitions += reg_structs
    enums_definitions += reg_enums

    structs_definitions += "}} {};".format(data["name"])

    final_output = write_template(input_template, structs_definitions, enums_definitions, data["name"])
    write_output(output_filename, final_output)


if __name__ == "__main__":
    main(sys.argv[1:])