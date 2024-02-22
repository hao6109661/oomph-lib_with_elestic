reset
set term png

set grid back

do for [i=0:100]{

set output sprintf("beam_test_flow%d.png",i)
# Set the plot range
    set yrange [-1:12]
    set xrange [-2:12]

# Set labels for the axes
set xlabel "x"
set ylabel "y"

# Set title for the plot
set title "Plot of Test Flow"

set label 1 sprintf("~{/symbol g}{1\\.} =%.2f", (1.0e-2)*i) at screen 0.97, 0.02 right front font ",15"

# Plot the first subplot
file = sprintf("test_flow%d.dat",i)

# Calculate the endpoint coordinates of the vectors
#set datafile separator ","  # Assuming data file is comma-separated
plot file using 1:2:3:4 with vectors head filled size screen 0.02,15,45 lw 2 lc "black" title "test flow" , \
     file using 3:2 with lines linewidth 2.5 lc rgb "pink" title ""
}

