#!/bin/bash

# Run the simulator
echo "GENERATING SIMULATION DATA..."
../../build/gnss-sim -e ABMF00GLP_R_20240490000_01D_MN.rnx -o file-output.bin

# Run GNSS-SDR
echo "RUNNING GNSS-SDR..."
gnss-sdr --config_file=navmessage.conf

# Update user
echo ""
echo "FORMATTING AND COMPARING OUTPUTS..."

# Extract the "<second>" region from the XML file
awk '/<second/,/<\/second>/' gps_ephemeris.xml | grep -vE '(satClkDrift|dtr|IODE_SF2|IODE_SF3|code_on_L2|L2_P_data_flag|AODO|fit_interval_flag|spare1|spare2|integrity_status_flag|alert_flag|antispoofing_flag|tow)' > extracted-region.xml

# Convert the extracted XML region to the text format
awk -F'[><]' '/<second/{p=1;next}/<\/second/{p=0}p' extracted-region.xml | awk -F'[><]' '{print $2 ": " $3}' > converted-extracted-region.txt

# Perform the diff
diff -y  gnss-sim-ephemeris.txt converted-extracted-region.txt

# Clean up temporary files
rm extracted-region.xml converted-extracted-region.txt
