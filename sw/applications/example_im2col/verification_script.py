import numpy as np

# Parameters of the random image
image_height = 5
image_width = 5
channels = 3

# Parameters of the filter
filter_height = 3
filter_width = 3
padding = 0
stride = 1

# Calculate the number of patches, i.e. the number the filter can fit along one dimention during convolution
n_patches_h = (image_height + 2 * padding - filter_height) // stride + 1
n_patches_w = (image_width + 2 * padding - filter_width) // stride + 1

# Calculate the dimensions of the output matrix
OW = filter_width * filter_height * channels
OH = n_patches_h * n_patches_w

random_image=[]

for c in range(channels):
    random_image.append(np.random.randint(0, 2**16, size=(image_width, image_height), dtype=np.uint32))

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

print(random_image)

output_image = np.zeros((OW, OH))

column = 0
row = 0
i = 0
j = 0

# Perform im2col operation
for c in range(channels):
    for v in range(n_patches_h):
        for u in range(n_patches_w):
            for k in range(filter_height):
                for x in range(filter_width):
                    output_image[row][column] = random_image[c][i + k][j + x]
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

print(output_image)

# Dump input image and output col to header file
with open('im2colGolden.h', 'w') as f:
    f.write('#ifndef IMAGE_AND_COL_H\n')
    f.write('#define IMAGE_AND_COL_H\n\n')
    f.write('const uint32_t input_image[%d]' % channels)
    f.write('[%d]' % image_height)
    f.write('[%d] = {\n' % image_width)

    for i in range(channels):
        f.write('{')
        for j in range(image_height):
            f.write('{')
            for k in range(image_width):
                if k != image_width - 1:
                    f.write('%d, ' % random_image[i][j][k])
                else:
                    f.write('%d' % random_image[i][j][k])
            if j != image_height - 1:
                f.write('},\n')
            else:
                f.write('}\n')
        if i != channels - 1:
            f.write('},\n')
        else:
            f.write('}')
    f.write('};\n\n')

    f.write('const uint32_t golden_im2col[%d]' % OH)
    f.write('[%d] = {\n' % OW)
    
    count_row = 0
    count_column = 0

    for column in range(OW):
        f.write('{')
        for row in range(OH):
            if row != OH - 1:
                f.write('%d, ' % output_image[column][row])
            else:
                f.write('%d' % output_image[column][row])
        if column != OW - 1:
            f.write('},\n')
        else:
            f.write('}')
    f.write('};\n\n')

    f.write('#define IW %d\n' % image_width)
    f.write('#define IH %d\n' % image_height)
    f.write('#define CH %d\n' % channels)
    f.write('#define FW %d\n' % filter_width)
    f.write('#define FH %d\n' % filter_height)
    f.write('#define S %d\n' % stride)
    f.write('#define P %d\n' % padding)
    
    f.write('#endif\n')