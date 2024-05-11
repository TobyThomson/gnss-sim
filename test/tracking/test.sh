#!/bin/bash

# Remove the debug file from last run
rm channel-dump.txt

# Uncomment the following lines if binary generation required
# Run the simulator
echo "GENERATING SIMULATION DATA..."
../../build/gnss-sim -e brdc0010.22n -o file-output.bin

# Update user
echo ""
echo "PLOTING..."

# Plot graphs
gnuplot plot-pseudorange.gp
gnuplot plot-elevation.gp
gnuplot plot-pseudorange-rate.gp
gnuplot plot-carrier-phase.gp
gnuplot plot-navbit-pointer.gp
gnuplot plot-code-chip-pointer.gp

# Update user
echo ""
echo "RUNNING GNSS-SDR..."

# Run GNSS-SDR
gnss-sdr --config_file=tracking.conf > gnss-sdr.log

# Update user
echo ""
echo "FORMATTING GNSS-SDR LOG..."

# Reformat the GNSS-SDR log file (makes easier to diagnose)
sed -n '/Current receiver time/,/Stopping GNSS-SDR/ {/Stopping GNSS-SDR/!p}' gnss-sdr.log | \
awk '/^Current receiver time:/ {if (!prevLineStartsWithTime) print; prevLineStartsWithTime=1; next} {prevLineStartsWithTime=0; print}' | \
awk 'NR>1 && /^Current receiver time:/ {print ""} 1' > output.txt