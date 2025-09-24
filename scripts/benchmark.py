import os
import sys
import subprocess
import time
import random
import string
import shutil
from pathlib import Path

# --- Configuration ---
NUM_FILES = 100000
NUM_DIRS = 1000
MAX_DEPTH = 5
MALICIOUS_PERCENTAGE = 0.01  # 1% of files will be malicious
FILE_MIN_SIZE_KB = 1
FILE_MAX_SIZE_KB = 128

MALICIOUS_CONTENT = {
    "EVIL": "179052c9c6165bf25917781fc5816993;Exploit",
    "MALWARE": "b867e23836356d568aadfe4a2fe9b0e1;Dropper",
}


def create_benchmark_data(root_dir):
    """Creates a large directory structure with test files."""
    scan_dir = root_dir / "scan_data"
    base_path = root_dir / "base.csv"
    log_path = root_dir / "report.log"

    if scan_dir.exists():
        print(f"'{scan_dir}' already exists. Removing it.")
        shutil.rmtree(scan_dir)

    print(f"Creating benchmark data in '{scan_dir}'...")
    scan_dir.mkdir(parents=True)

    # Create directory structure
    dirs = [scan_dir]
    for _ in range(NUM_DIRS):
        parent = random.choice(dirs)
        depth = len(parent.relative_to(scan_dir).parts)
        if depth < MAX_DEPTH:
            new_dir = parent / f"dir_{_}"
            new_dir.mkdir()
            dirs.append(new_dir)

    # Create files
    num_malicious = int(NUM_FILES * MALICIOUS_PERCENTAGE)
    malicious_keys = list(MALICIOUS_CONTENT.keys())

    print("Generating files...")
    print_progress_bar(0, NUM_FILES, prefix="Progress:", suffix="Complete", length=50)

    for i in range(NUM_FILES):
        target_dir = random.choice(dirs)
        file_path = target_dir / f"file_{i}.dat"

        content = b""
        if i < num_malicious:
            key = malicious_keys[i % len(malicious_keys)]
            content = key.encode("utf-8")
        else:
            size_kb = random.randint(FILE_MIN_SIZE_KB, FILE_MAX_SIZE_KB)
            content = generate_random_content(size_kb)

        with open(file_path, "wb") as f:
            f.write(content)

        if (i + 1) % 100 == 0 or (i + 1) == NUM_FILES:
            print_progress_bar(
                i + 1, NUM_FILES, prefix="Progress:", suffix="Complete", length=50
            )

    # Create base.csv
    with open(base_path, "w") as f:
        for line in MALICIOUS_CONTENT.values():
            f.write(f"{line}\n")

    print(f"Created {NUM_FILES} files and {len(dirs)} directories.")
    return scan_dir, base_path, log_path


def run_benchmark(scanner_exe, scan_dir, base_path, log_path):
    """Runs the scanner and measures its performance."""
    if not Path(scanner_exe).exists():
        print(f"Error: Scanner executable not found at '{scanner_exe}'")
        print("Please build the 'scanner' target first.")
        sys.exit(1)

    command = [
        scanner_exe,
        "--path",
        str(scan_dir),
        "--base",
        str(base_path),
        "--log",
        str(log_path),
    ]

    print(f"\nRunning command: {' '.join(command)}")
    print("Starting benchmark. Please monitor CPU usage...")

    start_time = time.perf_counter()
    process = subprocess.run(command, capture_output=True, text=True)
    end_time = time.perf_counter()

    if process.returncode != 0:
        print("\n--- SCANNER FAILED ---")
        print("Exit Code:", process.returncode)
        print("Output:\n", process.stdout)
        print("Error:\n", process.stderr)
        return

    duration = end_time - start_time
    files_per_sec = NUM_FILES / duration

    print("\n--- BENCHMARK COMPLETE ---")
    print(process.stdout)
    print(f"Total execution time: {duration:.2f} seconds")
    print(f"Performance: {files_per_sec:.2f} files/second")
    print("--------------------------")


def generate_random_content(size_kb):
    """Generates a block of random text data."""
    size_bytes = size_kb * 1024
    return "".join(
        random.choices(string.ascii_letters + string.digits, k=size_bytes)
    ).encode("utf-8")


def print_progress_bar(iteration, total, prefix="", suffix="", length=50, fill="█"):
    """Prints a dynamic progress bar."""
    percent = ("{0:.1f}").format(100 * (iteration / float(total)))
    filled_length = int(length * iteration // total)
    bar = fill * filled_length + "-" * (length - filled_length)
    sys.stdout.write(f"\r{prefix} |{bar}| {percent}% {suffix}")
    sys.stdout.flush()
    if iteration == total:
        sys.stdout.write("\n")


def main():
    if len(sys.argv) != 2:
        print(f"Usage: python {sys.argv[0]} <path_to_scanner_executable>")
        sys.exit(1)

    scanner_exe_path = sys.argv[1]
    benchmark_root = Path("./benchmark_data").resolve()

    scan_dir, base_path, log_path = create_benchmark_data(benchmark_root)
    run_benchmark(scanner_exe_path, scan_dir, base_path, log_path)

    print("\nCleaning up benchmark data...")
    shutil.rmtree(benchmark_root)


if __name__ == "__main__":
    main()
