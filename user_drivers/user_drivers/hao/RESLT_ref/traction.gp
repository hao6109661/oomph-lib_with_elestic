reset
set term png

set grid back

do for [i=0:100]{

set output sprintf("beam_traction_new%d.png",i)
# Set the plot range
    set yrange [-1:12]
    set xrange [-2:12]

# Set labels for the axes
set xlabel "x"
set ylabel "y"

# Set title for the plot
set title "Plot of Traction"

set label 1 sprintf("~{/symbol g}{1\\.} =%.2f, Scale=10^{-4}.", (1.0e-2)*i) at screen 0.97, 0.02 right front font ",15"

# Plot the first subplot
file = sprintf("beam%d.dat",i)

# Calculate the endpoint coordinates of the vectors
#set datafile separator ","  # Assuming data file is comma-separated
plot file every 20::1 using 1:2:(column(8)*1.0e5):(column(9)*1.0e5) with vectors head filled size screen 0.02,15,45 lw 2 lc "blue" title "traction f *10^{5}", \
     file using 1:2 with lines linewidth 2.7 lc rgb "red" title "Beam"

}

