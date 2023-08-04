#created by Luca Varisco

import subprocess
import time
import argparse
import signal
import psutil

DEFAULT_N = 111
DEFAULT_TIMEOUT = 30

# Type of return: -1 -> seg fault
#                 -2 -> wrong diff
#                 -3 -> time exceed
#                  1 -> right

def kill(proc_pid):
    try:
        process = psutil.Process(proc_pid)
        for proc in process.children(recursive=True):
            proc.kill()
        process.kill()
    except psutil.NoSuchProcess:
        pass

def launch_bash_command(n, timeout, processes):
    bash_command = "./main < open_{}.txt > out.txt".format(n)
    process = None  # Initialize process variable

    try:
        # Start the process
        start_time = time.time()
        process = subprocess.Popen(bash_command, shell=True)
        output, error = process.communicate(timeout=timeout)  # Wait for the process to finish or timeout
        end_time = time.time()

        # Check if the process is still running
        if process.poll() is None:
            # If the process is still running, terminate it and return -3
            kill(process.pid)
            process.wait()  # Wait for the process to terminate
            return -3, None

        # Process has finished, check for errors
        if process.returncode != 0:
            return -1, None

        # Check the output using the 'diff' command
        diff_command = "diff open_{}.output.txt out.txt".format(n)
        diff_output = subprocess.run(diff_command, shell=True, capture_output=True)

        if diff_output.returncode == 0:
            return 1, end_time - start_time
        else:
            return -2, None
    except subprocess.TimeoutExpired:
        # Timeout occurred while launching the process
        if process is not None:
            kill(process.pid)
            process.wait()  # Wait for the process to terminate
        return -3, None
    finally:
        if process is not None:
            kill(process.pid)
            process.wait()  # Wait for the process to terminate

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Launch multiple bash commands and check their output.")
parser.add_argument("n", type=int, nargs="?", default=DEFAULT_N, help="Number of the range (default: 111)")
parser.add_argument("-t", "--time", type=int, default=DEFAULT_TIMEOUT, help="Timeout value in seconds (default: 30)")
args = parser.parse_args()

# MAIN

results = {}
r = 0
t = 0
e = 0
d = 0

for n in range(1, args.n + 1):
    print("Starting test", n)
    result, elapsed_time = launch_bash_command(n, args.time, [])
    results[n] = (result, elapsed_time)

# Constants for ANSI escape codes
COLOR_GREEN = "\033[32;1m"
COLOR_RED = "\033[31;1m"
COLOR_ORANGE = "\033[35;1m"
COLOR_YELLOW = "\033[33;1m"
COLOR_RESET = "\033[30;0m"

# Display the results with colors and time
for n, (result, elapsed_time) in results.items():
    if result == 1:
        color_code = COLOR_GREEN
        result_text = "RIGHT"
        r += 1
        if elapsed_time is not None:
            time_info = " (Time: {:.2f}s)".format(elapsed_time)
        else:
            time_info = ""
    elif result == -1:
        color_code = COLOR_RED
        result_text = "SEG F."
        e += 1
        time_info = ""
    elif result == -2:
        color_code = COLOR_ORANGE
        result_text = "DIFF"
        d += 1
        time_info = ""
    elif result == -3:
        color_code = COLOR_YELLOW
        result_text = "TIME"
        t += 1
        time_info = ""
    else:
        color_code = ""
        result_text = "UNKNOWN"
        time_info = ""

    print("{} -> {}{}{}".format(n, color_code, result_text, time_info) + COLOR_RESET)

print("")
print("Final res:")
print("R ->", r)
print("S ->", e)
print("D ->", d)
print("T ->", t)
