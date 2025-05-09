# from PIL import Image

# # Open the generated vertical bar image
# image = Image.open("cb_sprite_10.png")

# # Ensure it's in 1-bit mode (black and white)
# image = image.convert("1")

# # Get the pixel data as a list of bits (0 for white, 1 for black)
# pixels = list(image.getdata())

# # Initialize an empty list to store hex values
# hex_values = []

# # Process the pixels row by row (128 columns)
# for y in range(0, 32, 8):  # Each byte represents 8 pixels (from top to bottom)
#     for x in range(0, 128):  # 128 columns
#         byte = 0
#         # Check each of the 8 rows for this column (from top to bottom)
#         for bit in range(8):
#             if pixels[(y + bit) * 128 + x] != 0:  # 
#                 byte |= (1 << (7 - bit))  # Set the corresponding bit
#         hex_values.append(f"0x{byte:02x}")  # Append the hex value as C-style format

# # Print the hex array
# print(", ".join(hex_values))


# from PIL import Image

# # Load the image you just generated
# image = Image.open("./img/all2/sprite_0.png").convert("1")

# # Make sure the image is 128x32
# width, height = image.size
# if width != 128 or height != 32:
#     raise ValueError("Image must be exactly 128x32 pixels.")

# pixels = image.load()
# bytes_out = []

# # Convert image pixels into 128 columns × 4 pages (8 pixels each)
# for page in range(0, height // 8):  # 4 pages for 32px height
#     for x in range(width):  # 128 columns
#         byte = 0
#         for bit in range(8):
#             y = page * 8 + bit
#             if pixels[x, y] != 0:  # black pixel = 1 in OLED logic
#                 byte |= (1 << bit)  # bottom-up within the byte
#         bytes_out.append(byte)

# # Format as C-style hex
# hex_array = ', '.join(f'0x{b:02X}' for b in bytes_out)

# # Print out nicely
# print("const uint8_t checkerboard_bars[] PROGMEM = {")
# for i in range(0, len(bytes_out), 16):
#     print("    " + ', '.join(f"0x{b:02X}" for b in bytes_out[i:i+16]) + ",")
# print("};")


import os
import re
import argparse
from PIL import Image

# -------------------------------
# Argument Parsing
# -------------------------------
parser = argparse.ArgumentParser(description="Convert 128x32 .png images to C-style byte arrays.")
parser.add_argument("--input", "-i", required=True, help="Input folder containing sprite images.")
parser.add_argument("--prefix", default="sprite_", help="Filename prefix (default: sprite_)")
parser.add_argument("--ext", default=".png", help="File extension (default: .png)")
parser.add_argument("--output-name", default="sprite", help="Base name for output arrays")

args = parser.parse_args()

# -------------------------------
# File Discovery and Validation
# -------------------------------
files = os.listdir(args.input)

# Raise exception if there are non-PNG files
non_pngs = [f for f in files if not f.endswith(args.ext)]
if non_pngs:
    raise ValueError(f"Non-PNG files found in input folder: {non_pngs}")

# Extract and sort files by numeric suffix
pattern = re.compile(rf"{re.escape(args.prefix)}(\d+){re.escape(args.ext)}$")
matched_files = []

for f in files:
    match = pattern.match(f)
    if match:
        index = int(match.group(1))
        matched_files.append((index, f))
    else:
        raise ValueError(f"Filename does not match expected pattern: {f}")

# Sort files based on numeric suffix
matched_files.sort()

# -------------------------------
# Main Conversion Loop
# -------------------------------
for index, filename in matched_files:
    filepath = os.path.join(args.input, filename)

    # Load and convert image
    image = Image.open(filepath).convert("1")
    width, height = image.size

    if width != 128 or height != 32:
        raise ValueError(f"{filename} must be exactly 128x32 pixels.")

    pixels = image.load()
    bytes_out = []

    for page in range(0, height // 8):
        for x in range(width):
            byte = 0
            for bit in range(8):
                y = page * 8 + bit
                if pixels[x, y] != 0:
                    byte |= (1 << bit)
            bytes_out.append(byte)

    var_name = f"{args.output_name}_{index}"
    print(f"const char {var_name}[] PROGMEM = {{")
    for i in range(0, len(bytes_out), 16):
        line = ', '.join(f"0x{b:02X}" for b in bytes_out[i:i+16])
        print(f"    {line},")
    print("};\n")