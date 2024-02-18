#!/bin/bash

# Function to clean up and exit gracefully
cleanup() {
    kill "$background_pid"
    exit
}

# Register the cleanup function to be called on script termination
trap cleanup EXIT

# Run the simulator in file output mode
echo "GENERATING SIMULATION DATA..."
../../build/gnss-sim -o file-output.bin

# Run GNSS-SDR in file input mode
echo "RUNNING GNSS-SDR (FILE INPUT)..."
gnss-sdr --config_file=file-output.conf

# Add a break
echo ""

# Run GNSS-SDR in zmq input mode
echo "RUNNING GNSS-SDR (ZMQ INPUT)..."
gnss-sdr --config_file=zmq-output.conf &
background_pid=$!

# Run the simulator in zmq output mode
echo "GENERATING SIMULATION DATA..."
../../build/gnss-sim

# Add a break
echo ""

# Generate plaintext files for comparison
echo "GENERATING DIFF FILES..."
xxd file-output.bin > file-output.hex
xxd zmq-output.bin > zmq-output.hex

# Find differences between the files
echo "FINDING DIFFERENCES..."
diff --suppress-common-lines -y file-output.hex zmq-output.hex | less