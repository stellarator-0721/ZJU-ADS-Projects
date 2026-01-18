============================================================
RECTANGLE PACKING ALGORITHMS - RUNNING GUIDE
============================================================

1. BUILD THE PROGRAMS
---------------------
Run the following command to compile both algorithms:
> make

This will create two executables in the 'build' folder:
- build/greedy
- build/sa

2. PREPARE INPUT FILES
-----------------------
Create input files in the 'data' folder with this format:

Example: data/input_10.txt
---------------------------------
10 100      <-- n=10 rectangles, strip width W=100
30 40       <-- rectangle 1: width=30, height=40
25 35       <-- rectangle 2: width=25, height=35
45 20       <-- rectangle 3: width=45, height=20
...        <-- 7 more rectangles
15 60       <-- rectangle 10: width=15, height=60
---------------------------------

Note: The program expects exactly 'n' rectangles after the first line.

3. RUN THE ALGORITHMS
---------------------
For Greedy algorithm:
> ./build/greedy data/input_10.txt

For Simulated Annealing:
> ./build/sa data/input_10.txt

Both programs require exactly one argument: the input file path.

4. OUTPUT FILES
---------------
After running, check the 'solution' folder for results:

For greedy: solution/greedy_10.txt
For SA:     solution/sa_10.txt

The output format is the same for both algorithms:
==================================================
Algorithm: Greedy NFDH            <-- Algorithm name
Input file: data/input_10.txt     <-- Input used
Rectangles: 10                    <-- Number of rectangles
Strip width: 100                  <-- Container width
Result height: 285                <-- Best height found
Runs: 1000                        <-- Executions for timing
Average CPU time (seconds): 0.000045120  <-- Performance
==================================================

5. IMPORTANT NOTES
------------------
- Input files MUST be in the 'data' folder
- The 'solution' folder will be created automatically
- For large instances (n > 1000), SA will be slower
- Greedy runs 1000 times for accurate timing
- SA runs 10 independent times and keeps the best result
- If you get "file not found" error, check that:
  1. The 'data' folder exists
  2. Input file is in correct format
  3. You're running from the project root directory

6. QUICK EXAMPLE
----------------
# Step 1: Build
make

# Step 2: Run greedy on a test file
./build/greedy data/input_50.txt

# Step 3: Check results
cat solution/greedy_50.txt

# Step 4: Run SA on same file
./build/sa data/input_50.txt

# Step 5: Compare results
cat solution/sa_50.txt

7. TROUBLESHOOTING
------------------
Error: "Usage: ./greedy data/input.txt"
Solution: Provide the input file path as argument

Error: "Error opening input file"
Solution: Check if file exists in 'data' folder

Error: "Error opening output file"
Solution: Ensure 'solution' folder exists or is writable

8. OUTPUT INTERPRETATION
------------------------
- "Result height": The minimum strip height found
- "Runs": How many times algorithm executed
- "Average CPU time": Time per execution in seconds
- Lower height = better packing
- Lower time = faster algorithm

============================================================
Summary: Build -> Run with input file -> Check solution folder
============================================================