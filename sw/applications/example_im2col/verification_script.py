import numpy as np
import torch
import torch.nn.functional as F

# import tensorflow as tf

#######################################################################################################
def torch_im2col_ncwh(input_tensor, kernel_size, stride=1, padding=0, dilation=1):
    """
    Applies the im2col operation to an input tensor using PyTorch's unfold method, supporting both NCHW and NHWC formats.

    Parameters:
    - input_tensor: A 4D input tensor.
    - kernel_size: The size of the kernel. Can be a single integer or a tuple (kH, kW).
    - stride: The stride of the convolution. Can be a single integer or a tuple (sH, sW). Default is 1.
    - padding: Implicit paddings on both sides of the input. Can be a single integer or a tuple (padH, padW). Default is 0.
    - dilation: The spacing between kernel elements. Can be a single integer or a tuple (dH, dW). Default is 1.
    - format: The format of the input tensor ('NCHW' or 'NHWC').

    Returns:
    - A 2D tensor of shape (batch_size*output_height*output_width, channels*kernel_height*kernel_width)
      that represents the unfolded input.
    """

    # Ensure kernel_size, stride, padding, and dilation are tuples
    if isinstance(kernel_size, int):
        kernel_size = (kernel_size, kernel_size)
    if isinstance(stride, int):
        stride = (stride, stride)
    if isinstance(padding, int):
        padding = (padding, padding)
    if isinstance(dilation, int):
        dilation = (dilation, dilation)

    # Adjust padding format for F.pad (expects pad_left, pad_right, pad_top, pad_bottom)
    padding_format = (padding[1], padding[1], padding[0], padding[0]) # Right, Left, Bottom, Top

    # Apply zero padding
    padded_input = F.pad(input_tensor, padding_format, "constant", 0)

    
    # Unfold the padded input tensor
    unfolded = padded_input.unfold(2, kernel_size[0], stride[0]).unfold(3, kernel_size[1], stride[1])

    unfolded = unfolded.permute(0, 2, 3, 1, 4, 5)

    # Reshape to get the 2D tensor where each row is a flattened receptive field
    # For NHWC, the channel size is now at the last position
    channel_dim = padded_input.size(1)
    return unfolded.contiguous().view(-1, channel_dim * kernel_size[0] * kernel_size[1]).t()

def torch_im2col_nhwc(input_tensor, kernel_size, stride=1, padding=0, dilation=1):
    if padding != 0:
        paddings = [[0, 0], [padding[0], padding[1]], [padding[2], padding[3]], [0, 0]]
        input_tensor = tf.pad(input_tensor, paddings, "CONSTANT")

    patches = tf.image.extract_patches(
        images=input_tensor,
        sizes=[1, kernel_size[0], kernel_size[1], 1],
        strides=[1, stride[0], stride[1], 1],
        rates=[1, 1, 1, 1],
        padding='VALID'  # Use 'VALID' since we're manually adding padding
    )
    
    # Reshape extracted patches
    batch_size, height, width, channels = input_tensor.shape
    patch_dim = kernel_size[0] * kernel_size[1] * channels
    patches_reshaped = tf.reshape(patches, [-1, patch_dim])
    

# Function to save a tensor to a C file

def torch_save(tensor, variable_name, dim, row_len):
    """
    Saves a tensor to a C file as an array.

    Parameters:
    - tensor: The tensor to be saved.
    - variable_name: The name of the array variable in the generated C code.
    - filename: The name of the file to save the array to.
    """
    # Ensure tensor is in CPU and convert to 1D
    tensor_flat = tensor.cpu().flatten()

    # Start generating the C code
    c_code = f"const uint32_t {variable_name}[{dim}] ="+ " {\n    "

    # Group elements into chunks of size row_len and convert each element to string
    elements = [str(val) for val in tensor_flat.numpy()]
    rows = [", ".join(elements[i:i+row_len]) for i in range(0, len(elements), row_len)]

    # Join the rows with ',\n    ' to add a newline and indentation after every row_len elements
    c_code += ",\n    ".join(rows)

    # End the array definition
    c_code += "\n};\n"

    # Write to the file
    f.write(c_code)

def tensor_save(tensor, variable_name, dim, row_len):
    """
    Saves a tensor to a C file as a statically defined array.
    
    Parameters:
    - tensor: A NumPy array containing the tensor data.
    - variable_name: The name of the array variable in the generated C code.
    - filename: The name of the file to save the array to.
    """
    # Convert the tensor to a numpy array if it's not already one
    if not isinstance(tensor, np.ndarray):
        tensor = np.array(tensor)
    
    # Open the file for writing
    f.write(f"const uint32_t {variable_name}[{dim}] =" + " {\n")
        
    # Iterate over the tensor data and write each element to the file
    for i, value in enumerate(np.nditer(tensor)):
        f.write(f"    {value}")
        if i < tensor.size - 1:
            f.write(",\n")
        
    # Close the array definition
    f.write("\n};\n")


#######################################################################################################
# Parameters

# Define the type of operation to perform
# 0: nchw
# 1: nhwc
im2col_format = 0

# Define which tool to use to generate the golden reference
# 0: my own implementation
# 1: torch.nn.functional.unfold/tensorflow.image.extract_patches
im2col_tool = 1

# Parameters of the random image, padding excluded
image_height = 4
image_width = 4
channels = 1
batch = 1

# Parameters of the filter
filter_height = 2
filter_width = 2
padding = 1
stride = 1

# Calculate the number of patches, i.e. the number the filter can fit along one dimention during convolution
n_patches_h = (image_height + 2 * padding - filter_height) // stride + 1
n_patches_w = (image_width + 2 * padding - filter_width) // stride + 1

# Calculate the dimensions of the output matrix
OH = filter_width * filter_height * channels # Number of rows in a column -> size of a column
OW = n_patches_h * n_patches_w # Numver of columns in a row -> size of a row


if im2col_tool == 0:

    random_image=[]

    for c in range(channels):
        random_image.append(np.random.randint(0, 2**16, size=(image_width, image_height), dtype=np.uint32))

    # Save the random image to later dump it to a file

    tbs_random_image = np.array(random_image)
    # If necessary, perform the padding:

    if padding != 0:
        first = True
        padded_channels = []
        for c in range(channels):
            ch = np.empty((0, image_width+2*padding))
            for r in range(image_width + 2*padding):
                if r < padding or (r >= image_height + padding and r < image_height + 2*padding):
                    row = np.zeros(2*padding + image_width) # This will produce the initial and final rows of zeros
                else: 
                    row = np.concatenate((np.zeros(padding), random_image[c][r-padding], np.zeros(padding)))
                # Check to initialize ch for np concatenation: if it's the first time, just copy the current row
                ch = np.vstack((ch, row))
            padded_channels.append(ch)
        random_image = np.stack(padded_channels, axis=0)

    # print(random_image)

    output_matrix = np.zeros((OH, OW))

    column = 0
    row = 0
    i = 0
    j = 0

    # Perform im2col operation
    if im2col_format == 0:
        for c in range(channels):
            for v in range(n_patches_h):
                for u in range(n_patches_w):
                    for k in range(filter_height):
                        for x in range(filter_width):
                            output_matrix[row][column] = random_image[c][i + k][j + x]
                            row += 1
                    row -= filter_height * filter_width
                    column += 1
                    j += stride
                i += stride
                j = 0
            column = 0    
            i = 0
            j = 0    
            row += filter_height * filter_width
    else:
        print("Not implemented yet")
        exit(1)

    print(output_matrix)

elif im2col_tool == 1:
    # Perform the im2col operation using torch.nn.functional.unfold
    if im2col_format == 0:
        input_tensor = torch.randint(0, 65500, (batch, channels, image_height, image_width), dtype=torch.int32)
        output_matrix = torch_im2col_ncwh(input_tensor, (filter_height, filter_width), stride, padding)
    else:
        input_tensor = tf.random.uniform((batch, image_height, image_width, channels), dtype=tf.int32)

    # print(output_matrix)


# Dump input image and output col to header file
with open('im2colGolden.h', 'w') as f:
    f.write('#ifndef IMAGE_AND_COL_H\n')
    f.write('#define IMAGE_AND_COL_H\n\n')
    f.write('#define IW %d\n' % image_width)
    f.write('#define IH %d\n' % image_height)
    f.write('#define CH %d\n' % channels)
    f.write('#define FW %d\n' % filter_width)
    f.write('#define FH %d\n' % filter_height)
    f.write('#define S %d\n' % stride)
    f.write('#define P %d\n\n' % padding)

    f.write('#include <stdint.h>\n')

    f.write('extern const uint32_t input_image[%d];\n' % (channels * image_height * image_width))
    f.write('extern const uint32_t golden_im2col[%d];\n' % (OW*OH))

    f.write('#endif\n')

# Dump input image and output col to C file

with open('im2colGolden.c', 'w') as f:

    f.write('#include "im2colGolden.h"\n\n')

    if im2col_tool == 0:
    
        f.write('const uint32_t input_image[%d] = {\n' % (channels * image_height * image_width))

        for i in range(channels):
            for j in range(image_height):
                for k in range(image_width):
                    if k != image_width - 1 or j != image_height - 1 or i != channels - 1:
                        f.write('%d, ' % tbs_random_image[i][j][k])
                    else:
                        f.write('%d' % tbs_random_image[i][j][k])
                f.write('\n')
        f.write('};\n\n')

        f.write('const uint32_t golden_im2col[%d] = {\n' % (OW*OH))
        
        count_row = 0
        count_column = 0

        for column in range(OH):
            for row in range(OW):
                if row != OW - 1 or column != OH -1:
                    f.write('%d, ' % output_matrix[column][row])
                else:
                    f.write('%d' % output_matrix[column][row])
            f.write('\n')    
        f.write('};\n\n')

    elif im2col_tool == 1:
        if im2col_format == 0:
            torch_save(input_tensor, "input_image", channels * image_height * image_width, image_width)
            torch_save(output_matrix, "golden_im2col", OW*OH, OW)
        else:
            tensor_save(input_tensor, "input_image", channels * image_height * image_width, image_width)
            tensor_save(output_matrix, "golden_im2col", OW*OH, OW)
    