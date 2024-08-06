import numpy as np
import torch
import torch.nn.functional as F

#######################################################################################################
def torch_im2col_ncwh(input_tensor, kernel_size, stride_d1=1, stride_d2=1, top_pad=1, bottom_pad=1, left_pad=1, right_pad=1, dilation=1):
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
    if isinstance(dilation, int):
        dilation = (dilation, dilation)

    # Adjust padding format for F.pad (expects pad_left, pad_right, pad_top, pad_bottom)
    padding_format = (left_pad, right_pad, top_pad, bottom_pad)

    # Apply zero padding
    padded_input = F.pad(input_tensor, padding_format, "constant", 0)

    
    # Unfold the padded input tensor
    unfolded = padded_input.unfold(2, kernel_size[0], stride_d2).unfold(3, kernel_size[1], stride_d1)

    unfolded = unfolded.permute(0, 2, 3, 1, 4, 5)

    # Reshape to get the 2D tensor where each row is a flattened receptive field
    # For NHWC, the channel size is now at the last position
    channel_dim = padded_input.size(1)
    return unfolded.contiguous().view(-1, channel_dim * kernel_size[0] * kernel_size[1]).t()

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
    c_code = ("const uint32_t %s[%d] = {\n    " % (variable_name, dim))

    # Group elements into chunks of size row_len and convert each element to string
    elements = [str(val) for val in tensor_flat.numpy()]
    rows = [", ".join(elements[i:i+row_len]) for i in range(0, len(elements), row_len)]

    # Join the rows with ',\n    ' to add a newline and indentation after every row_len elements
    c_code += ",\n    ".join(rows)

    # End the array definition
    c_code += "\n};\n"

    # Write to the file
    f.write(c_code)

#######################################################################################################
# Parameters of the random image, padding excluded
image_height = 10
image_width = 10
channels = 1
batch = 1

# Parameters of the filter
filter_height = 4
filter_width = 4
top_pad = 1
bottom_pad = 1
left_pad = 1
right_pad = 1
stride_d1 = 1
stride_d2 = 1

# Calculate the number of patches, i.e. the number the filter can fit along one dimension during convolution
n_patches_h = (image_height + top_pad + bottom_pad - filter_height) // stride_d2 + 1
n_patches_w = (image_width + right_pad + left_pad - filter_width) // stride_d1 + 1

# Calculate the dimensions of the output matrix
OH = filter_width * filter_height * channels * batch # Number of rows in a column -> size of a column
OW = n_patches_h * n_patches_w # Numver of columns in a row -> size of a row


input_tensor_nchw = torch.randint(0, 65500, (batch, channels, image_height, image_width), dtype=torch.int32)
output_matrix_nchw = torch_im2col_ncwh(input_tensor_nchw, (filter_height, filter_width), stride_d1, stride_d2, top_pad, bottom_pad, left_pad, right_pad)

# Dump input image and output col to header file
with open('im2colGolden.h', 'w') as f:
    f.write("/*\n   Copyright EPFL contributors.\n  Licensed under the Apache License, Version 2.0, see LICENSE for details.\n")
    f.write("  SPDX-License-Identifier: Apache-2.0\n\n")
    f.write("  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>\n\n")
    f.write("                          <tommaso.terzano@gmail.com>\n\n")
    f.write("  Info: Header file of im2colGolden, contains activations parameters and the prototypes of both input tensors and golden output.\n*/\n\n")
    f.write('#ifndef IMAGE_AND_COL_H\n')
    f.write('#define IMAGE_AND_COL_H\n\n')
    f.write('#include <stdint.h>\n\n')
    f.write('/* Parameters */\n')
    f.write('#define IW %d\n' % image_width)
    f.write('#define IH %d\n' % image_height)
    f.write('#define CH %d\n' % channels)
    f.write('#define FW %d\n' % filter_width)
    f.write('#define FH %d\n' % filter_height)
    f.write('#define BATCH %d\n' % batch)
    f.write('#define STRIDE_D1 %d\n' % stride_d1)
    f.write('#define STRIDE_D2 %d\n' % stride_d2)
    f.write('#define TOP_PAD %d\n' % top_pad)
    f.write('#define BOTTOM_PAD %d\n' % bottom_pad)
    f.write('#define LEFT_PAD %d\n' % left_pad)
    f.write('#define RIGHT_PAD %d\n' % right_pad)

    f.write('extern const uint32_t input_image_nchw[%d];\n' % (channels * image_height * image_width * batch))
    f.write('extern const uint32_t golden_im2col_nchw[%d];\n' % (OW*OH))
    f.write('\n#endif\n')

# Dump input image and output col to C file

with open('im2colGolden.c', 'w') as f:
    f.write("/*\n   Copyright EPFL contributors.\n  Licensed under the Apache License, Version 2.0, see LICENSE for details.\n")
    f.write("  SPDX-License-Identifier: Apache-2.0\n\n")
    f.write("  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>\n\n")
    f.write("                          <tommaso.terzano@gmail.com>\n\n")
    f.write("  Info: Contains randomly generated input activations and the golden result of the im2col algorithm, computed with either Pytorch or Tensorflow,\n  depending on the format.\n*/\n\n")

    f.write('#include "im2colGolden.h"\n\n')

    torch_save(input_tensor_nchw, "input_image_nchw", channels * image_height * image_width * batch, image_width)
    torch_save(output_matrix_nchw, "golden_im2col_nchw", OW*OH, OW)

    f.write('\n\n')