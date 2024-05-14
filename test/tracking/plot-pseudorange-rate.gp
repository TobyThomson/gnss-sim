# Set output file format and name
set terminal pngcairo enhanced font 'Verdana,12'
set output 'graphs/pseudorange-rate-vs-time.png'

# Set plot title and axis labels
set title "Pseudorange Rate vs Simulation Time"
set xlabel "Simulation Time (s)"
set ylabel "Pseudorange Rate (m/s)"

# Set y axis limit
set yrange [-1000:1000]

# Enable grid and legend
set grid
set key autotitle columnheader

# Must define "nan" as missing to ignore PRN filtered lines
set datafile missing nan

# Define function to filter data based on PRN value
filter_prn(prn_col, prn_value) = (column(prn_col) == prn_value)

# Identify unique PRN numbers from the data file
PRN_list = system("awk 'NR==1 {next} {print $2}' channel-dump.txt | sort -u")

# Plot all data on the same figure, grouping by PRN
plot_cmd = "plot "
do for [PRN in PRN_list] {
    plot_cmd = plot_cmd . sprintf("'channel-dump.txt' using 1:($4*(filter_prn(2, \"%s\") ? 1 : NaN)):2 with linespoints title 'PRN %s', ", PRN, PRN)
}

# Remove trailing comma and space from plot command
plot_cmd = substr(plot_cmd, 1, strlen(plot_cmd) - 2)

# Plot data for each PRN separately
eval plot_cmd

# Uncomment for debugging
#set table "gnuplot-dump.txt"
#    plot 'channel-dump.txt' using 1:($4*(filter_prn(2, "18") ? 1 : NaN)):2 with table title 'PRN 18'
#unset table
