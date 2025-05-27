# Test 1: Basic command execution
ls

# Test 2: Change directory and print working directory
cd ..
pwd

# Test 3: History command (will only show commands after shell starts)
history

# Test 4: Input redirection
echo "Hello, shell!" > input.txt
cat < input.txt

# Test 5: Output redirection (overwrite)
echo "New Output" > out.txt
cat out.txt

# Test 6: Output redirection (append)
echo "Line 1" > append.txt
echo "Line 2" >> append.txt
cat append.txt

# Test 7: Command piping
cat input.txt | grep Hello | wc -l

# Test 8: Multiple commands with semicolon
echo first; echo second; echo third

# Test 9: Conditional execution (true case)
echo hello && echo world

# Test 10: Conditional execution (false case)
badcommand && echo will not show

# Test 11: Combined features
cat < input.txt | grep Hello | tee output.txt && echo Done > done.txt
cat output.txt
cat done.txt

# Test 12: Final history
history

# Clean up (optional)
rm input.txt out.txt append.txt output.txt done.txt