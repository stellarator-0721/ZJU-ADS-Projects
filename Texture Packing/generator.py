import random
import sys
import os

"""
Usage:
    python generator.py n W max_h output.txt
Example:
    python generator.py 5000 200 1000 input_5000.txt
The output file will be written to the 'data/' directory.
"""

if len(sys.argv) < 5:
    print("Usage: python generator.py n W max_h output.txt")
    sys.exit(1)

n = int(sys.argv[1])
W = int(sys.argv[2])
max_h = int(sys.argv[3])
filename = sys.argv[4]

# Ensure data directory exists
data_dir = "data"
os.makedirs(data_dir, exist_ok=True)

# Construct full output path
outfile = os.path.join(data_dir, filename)

with open(outfile, "w") as f:
    f.write(f"{n} {W}\n")
    for _ in range(n):
        w = random.randint(1, W)
        h = random.randint(1, max_h)
        f.write(f"{w} {h}\n")

print(f"Test data written to {outfile}")
