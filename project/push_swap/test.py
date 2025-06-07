import random
import subprocess
import sys
from tqdm import tqdm

CHECKER = "./checker_linux"
PUSH_SWAP = "./push_swap"

if len(sys.argv) < 2:
    print(f"Usage: {sys.argv[0]} <length> [<counts>]")
    sys.exit(1)

length = int(sys.argv[1])
counts = 100
if len(sys.argv) == 3:
    counts = int(sys.argv[2])

print("==== Push_swap Test Runner (Python, with negatives and stats) ====")

op_counts = []
erros_lst = []

for i in tqdm(range(counts), desc="Running tests", unit="test"):
    test_case = random.sample(range(-length, length + 1), length)
    test_case_str = list(map(str, test_case))

    try:
        # Run push_swap
        push_swap_proc = subprocess.run(
            [PUSH_SWAP, *test_case_str],
            capture_output=True,
            text=True
        )
        ops = push_swap_proc.stdout.strip().splitlines()
        op_count = len(ops)
        op_counts.append(op_count)

        # Run checker
        checker_proc = subprocess.run(
            [CHECKER, *test_case_str],
            input=push_swap_proc.stdout,
            capture_output=True,
            text=True
        )
        result = checker_proc.stdout.strip()

        # if result == "OK":
        #     print(f"Test {i+1}: OK, operations = {op_count}")
        # else:
        #     print(f"Test {i+1}: KO, operations = {op_count}")
        #     print(f"Test case: {' '.join(test_case_str)}")
        if result == "OK":
            # do nothing, just pass
            pass
        else:
            erros_lst.append((test_case, ops))

    except Exception as e:
        print(f"Error running test {i+1}: {e}")
        op_counts.append(None)


if erros_lst:
    print("Errors found in the following test cases:")
    for test_case, ops in erros_lst:
        print(f"Test case: {' '.join(map(str, test_case))}")
        print(f"Operations: {ops}")
else:
    print("All tests passed successfully.")

# Compute stats
valid_ops = [c for c in op_counts if c is not None]
if valid_ops:
    avg_ops = sum(valid_ops) / len(valid_ops)
    print(f"Average operations over {len(valid_ops)} tests: {avg_ops:.2f}")
else:
    print("No valid operation counts to compute average.")
