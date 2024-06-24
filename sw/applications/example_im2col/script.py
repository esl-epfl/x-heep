with open('im2col_data.txt', 'r') as file:
    file.write("im2col_cpu:\n")
    for value in im2col_cpu_array:
        file.write(f"{value}\n")
    file.write("\n")
    
    file.write("im2col_dma_2d_C:\n")
    for value in im2col_dma_2d_C_array:
        file.write(f"{value}\n")
    file.write("\n")
    
    file.write("im2col_spc:\n")
    for value in im2col_spc_array:
        file.write(f"{value}\n")
    file.write("\n")