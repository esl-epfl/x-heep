# Generate 1KB of different bytes in hexadecimal notation
hex_data = [f'0x{i:08X},' if (i + 1) % 8 != 0 else f'0x{i:08X},\n' for i in range(0xFFFFFFFF, 0xFFFFFBFE, -1)]  # Repeat 0x00 to 0xFF four times
hex_data = ' '.join(hex_data)  # Convert the list to a space-separated string

# Write the data to a file
with open('hex_data.txt', 'w') as file:
    file.write(hex_data)

print("1KB of different bytes in hexadecimal notation has been written to 'hex_data.txt'.")
