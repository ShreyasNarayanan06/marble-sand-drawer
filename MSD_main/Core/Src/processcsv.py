import csv
import sys
import os

# --- Configuration ---
if len(sys.argv) < 2:
    print("Usage: python processcsv.py <input.csv>")
    sys.exit(1)

csv_filename = sys.argv[1]   # The file to read (provided on the command line)
c_filename = "path_data.c"    # The C file it will create
h_filename = "path_data.h"    # The Header file it will create

coords = []

# 1. Read the CSV file
print(f"Reading {csv_filename}...")
with open(csv_filename, 'r') as f:
    reader = csv.reader(f)
    for row in reader:
        try:
            # Try to convert to floats (skips text headers automatically)
            x, y = float(row[0]), float(row[1])
            coords.append((x, y))
        except (ValueError, IndexError):
            continue 

num_points = len(coords)
print(f"Found {num_points} valid coordinate pairs.")

# 2. Generate the .h Header File
with open(h_filename, 'w') as f:
    f.write("#ifndef PATH_DATA_H\n")
    f.write("#define PATH_DATA_H\n\n")
    f.write(f"// Automatically generated from {csv_filename}\n")
    f.write(f"extern const int NUM_PATH_POINTS;\n")
    f.write(f"extern const double path_data[{num_points}][2];\n\n")
    f.write("#endif // PATH_DATA_H\n")

# 3. Generate the .c Source File
with open(c_filename, 'w') as f:
    f.write(f'#include "{h_filename}"\n\n')
    f.write(f"const int NUM_PATH_POINTS = {num_points};\n\n")
    f.write(f"// Array format: {{X, Y}}\n")
    f.write(f"const double path_data[{num_points}][2] = {{\n")
    
    # Write each coordinate pair
    for i, (x, y) in enumerate(coords):
        f.write(f"    {{{x}, {y}}}")
        if i < num_points - 1:
            f.write(",\n")
        else:
            f.write("\n")
            
    f.write("};\n")

print(f"Success! Drag {c_filename} and {h_filename} into your STM32 project.")

