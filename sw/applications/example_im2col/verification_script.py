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
OH = filter_width * filter_height * channels # Number of rows in a column -> size of a column
OW = n_patches_h * n_patches_w # Numver of columns in a row -> size of a row

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

output_image = np.zeros((OH, OW))
print(output_image)

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
    f.write('#define IW %d\n' % image_width)
    f.write('#define IH %d\n' % image_height)
    f.write('#define CH %d\n' % channels)
    f.write('#define FW %d\n' % filter_width)
    f.write('#define FH %d\n' % filter_height)
    f.write('#define S %d\n' % stride)
    f.write('#define P %d\n\n' % padding)

    f.write('const uint32_t input_image[%d] = {\n' % (channels * image_height * image_width))

    for i in range(channels):
        for j in range(image_height):
            for k in range(image_width):
                if k != image_width - 1 or j != image_height - 1 or i != channels - 1:
                    f.write('%d, ' % random_image[i][j][k])
                else:
                    f.write('%d' % random_image[i][j][k])
            f.write('\n')
    f.write('};\n\n')

    f.write('const uint32_t golden_im2col[%d] = {\n' % (OW*OH))
    
    count_row = 0
    count_column = 0

    for column in range(OH):
        for row in range(OW):
            if row != OW - 1 or column != OH -1:
                f.write('%d, ' % output_image[column][row])
            else:
                f.write('%d' % output_image[column][row])
        f.write('\n')    
    f.write('};\n\n')

    
    
    f.write('#endif\n')