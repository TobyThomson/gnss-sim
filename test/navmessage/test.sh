#!/bin/bash

# Run the simulator
echo "GENERATING SIMULATION DATA..."
../../build/gnss-sim -o file-output.bin

# Run GNSS-SDR
echo "RUNNING GNSS-SDR..."
gnss-sdr --config_file=navmessage.conf