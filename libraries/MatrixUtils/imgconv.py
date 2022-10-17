#! /bin/env python3
# python .\imgconv.py .\data\1.png -o ..\libraries\images\img.h -A -Z -l col
# python .\imgconv.py ..\..\matrix\data\marquee2.png -o ..\images\marquee.h -f string -A

# TODO: Unix path support

from PIL import Image
import sys
import os
import imghdr

args = sys.argv[1:]
valid_formats = [
    "hex", "h",
    "decimal", "dec", "d",
    "string", "str", "s",
]
valid_layouts = ["row", "r", "column", "col", "c"]

if (len(args) < 1):
    print("Error, must specify at least one argument for input file(s).")
    print("""
        ImgConv - Simple image to C converter
        
    Converts an image, or the images in a folder, to 
    C arrays for easy use in embedded programs.
    
    useage: python ./imgconv.py <image/folder> [-o <outfile>] 
    [-f <format>] [-l <layout>] [-A]
    
    Formats: hex/h, decimal/dec/d, string/str/s.
    Layouts: row-major (row/r), column-major (col/c)
    -A will add the arduino `PROGMEM` qualifier to data.
    -Z will format the data in a zigzag pattern, reversing
    the direction of every other row or column
    
    Output file contains:
        <name> (array, or array-of-arrays for multiple frames)
        <name>_width, <name>_height
        <name>_depth (specifying number of channels)
        <name>_numframes (only for multiple frames)
        <name>_names (filenames, only for multiple frames)
        
    Example useage:
        .\imgconv.py ..\..\matrix\data\marquee.png -o ..\images\marquee.h -f string -A
    """)
    exit(1)

# Base arguments
inp = args[0]
is_directory = os.path.isdir(inp)

format = "hex"
# Whether the data is stored in row or column-major order, important if directly writing to strips
layout = "col"

# Arduino mode, adds the PROGMEM qualifier to the data array(s).
arduino = ("-A" in args)

# Zigzag mode, causes every other major "slice" to be reversed, for matrices formed of a zigzag of LEDs:
# →→→→→→→→↓
# ↓←←←←←←←←
# →→→→→→→→↓
# ←←←←←←←←←
zigzag = ("-Z" in args)

# Output settings
suffix = "PROGMEM " if arduino else ""
prefix = "static const "

name = args[0].split("\\")[-1].split(".")[0]
outfile = name + ".h"

if ('-o' in args):
    oi = args.index('-o')
    if (len(args)-1 <= oi):
        print("Error, output file (-o) must be followed by the output filename")
        exit(1)
    #re.sub(r'\W+', '', (inp if is_directory else inp.split('.')[0]))
    outfile = args[oi + 1]
    name = args[oi + 1].split("\\")[-1].split(".")[0]

if ('-f' in args):
    fi = args.index('-f')
    if (len(args)-1 <= fi or args[fi + 1].lower() not in valid_formats):
        print("Error, format (-f) must be followed by a valid format specifier (hex|h, string|str|s, decimal|dec|d)")
        exit(1)
    format = args[fi + 1].lower()

if ('-l' in args):
    li = args.index('-l')
    if (len(args)-1 <= li or args[li + 1].lower() not in valid_layouts):
        print("Error, layout (-l) must be followed by a valid layout (column|col|c, row|r)")
        exit(1)
    layout = args[li + 1].lower()

out = []
# Find and sort images, if a directory is passed
imagenames = list([f"{inp}/{img}" for img in os.listdir(inp)
                  if imghdr.what(f"{inp}/{img}") != None]) if is_directory else [inp]
imagenames = sorted(
    imagenames, key=lambda name: not name[0].isdigit() or int(name.split('.')[0]))

width = -1
height = -1
depth = -1

# Put all images into output format, even if there is only one.
# will handle writing to file/C-specific formatting later
for imagename in imagenames:
    encoded = ""

    image = Image.open(imagename)
    image.convert("RGB")

    if height == -1:
        height = image.height
        width = image.width
    # All frames must be identically sized.
    assert height == image.height and width == image.width

    # Helper for DRYer code
    def outpixel(x, y):
        pix = image.getpixel((x, y))
        # Grayscale images won't be a tuple according to pillow docs, so convert it for consistency
        pix = list(pix) if isinstance(pix, tuple) else [pix]

        global depth
        if depth == -1:
            depth = len(pix)
        assert depth == len(pix)  # Ensure consistently channel-d images

        # Output the pixel depending on output format
        match format:
            case "hex" | "h":
                return "".join([f"{hex(c)}, " for c in pix])
            case "decimal" | "dec" | "d":
                return "".join([f"{str(c)}, " for c in pix])
            case "string" | "str" | "s":
                return "".join([f"\\{hex(c)[1:]}" for c in pix])

    # Row and column major options, x first or y first.
    if (layout in ["row", "r"]):
        for y in range(image.height):
            for x in range(image.width):
                if (zigzag and (y % 2) == 0):
                    x = (image.width-1) - x
                encoded += outpixel(x, y)
    else:
        for x in range(image.width):
            for y in range(image.height):
                if (zigzag and (x % 2) == 0):
                    y = (image.height-1) - y
                encoded += outpixel(x, y)

    out.append(encoded)

# ImgConv header
output_text = f"/*\n\tMade with ImgConv v1.1\n\tlayout: {layout}-major\n\tformat: {format}\n\tzigzag: {zigzag}\n*/\n\n"
# Include guard-ish and possibly arduino progmem include
output_text += f"#pragma once\n{'#include <avr/pgmspace.h>' if arduino else ''}\n\n"
# Dimensions
output_text += f"const unsigned int {name}_width = {width}, {name}_height = {height}, {name}_depth = {depth};\n"
# Parameter defines
output_text += f"#define {name.upper()}_ZIGZAG {1 if zigzag else 0}\n#define {name.upper()}_COLMAJOR {1 if layout in ['column', 'col', 'c'] else 0}\n\n"

# Single or multiple frames
if (len(out) == 1):
    # Output array
    # NOTE: String output requires 1 byte more for null terminator.
    output_text += f"{prefix}unsigned char {name}[{width*height*depth + (1 if format[0] == 's' else 0)}] {suffix}"
    # Strings are special
    if format[0] == 's':
        output_text += f"= \"{out[0]}\";"
    else:
        output_text += "= {\n\t" + out[0] + "\n};"
else:
    # Number of frames and frame filenames, for multiple frame collections
    output_text += f"unsigned int {name}_numframes = {len(imagenames)};\n\n"
    output_text += f"const char* {name}_names[] = " + "{\n"
    for imagename in imagenames:
        i = imagename.split("/")[-1]
        output_text += f"\t\"{i}\",\n"
    output_text += "};\n\n"

    # Output frames as an array-of-arrays, one 1D array per frame.
    output_text += f"{prefix}unsigned char {name}[][{width*height*depth + (1 if format[0] == 's' else 0)}] {suffix}" + "= {"
    for o in out:
        output_text += f"\n\t\"{o}\"," if format[0] == 's' else "\n\t{\n\t\t" + o + "\n\t},"
    output_text += "\n};"

# Finally, write data to output
with open(outfile, "w") as f:
    f.write(output_text)
