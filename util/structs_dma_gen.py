import hjson

# path in which the structs header files are generated, has to be formatted with
# the name of the peripheral
out_file_path = "./sw/device/lib/drivers/dma/dma_structs.h" 

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
    new_string = "#define dma_peri(channel) ((volatile dma *) (DMA_START_ADDRESS + DMA_SIZE * channel))"
    format_dma_channels(out_file_path, new_string)
