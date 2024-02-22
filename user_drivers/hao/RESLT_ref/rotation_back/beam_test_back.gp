reset
set term png

# Set aspect ratio to 1:1
set size ratio -1

set grid back

theta_eq=-0.3
X0=0.8
Y0=0.5

#do for [i=1:10]{

#set output sprintf("beam_test%d.png",i)
set output sprintf("beam_compare_back1.png")

# Set the plot range
    set yrange [-2:12]
    set xrange [-2:12]

# Set labels for the axes
set xlabel "x"
set ylabel "y"

# Set title for the plot
set title "Plot of Beam"

set label 1 sprintf("{/symbol q}_{eq}=%.2f, X0=%.2f, Y0=%.2f.", theta_eq, X0, Y0) at screen 0.97, 0.02 right front font ",15"

# Plot the first subplot
#file = sprintf("beam%d.dat",i)
file1 = sprintf("beam1.dat")
file2 = sprintf("beam1_after.dat")

# Calculate the endpoint coordinates of the vectors
#set datafile separator ","  # Assuming data file is comma-separated
plot file1 using 1:2 with lines linewidth 2.7 lc rgb "red" title "Beam", \
     file1 using (column(1)*cos(theta_eq)-column(2)*sin(theta_eq)+X0):(column(1)*sin(theta_eq)+column(2)*cos(theta_eq)+Y0) with lines linewidth 2.7 lc rgb "yellow" title "Beam with rotation and translation (gnuplot)",\
     file2 using 1:2 with lines linewidth 2.7 lc rgb "red" title "Beam with rotation and translation",\
     file1 every 20::1 using ((column(1)*cos(theta_eq)-column(2)*sin(theta_eq))*cos(theta_eq)+(column(1)*sin(theta_eq)+column(2)*cos(theta_eq))*sin(theta_eq)):(-(column(1)*cos(theta_eq)-column(2)*sin(theta_eq))*sin(theta_eq)+(column(1)*sin(theta_eq)+column(2)*cos(theta_eq))*cos(theta_eq)) with points pointsize 2 pointtype 8 lc rgb "yellow" title "Beam moves back (gnuplot)"
     
     
     #file2 using (column(1)*cos(-theta_eq)-column(2)*sin(-theta_eq)-X0):(column(1)*sin(-theta_eq)+column(2)*cos(-theta_eq)-Y0) with lines linewidth 2.7 lc rgb "yellow" title ""
     #file2 using ((column(1)-X0)*cos(-theta_eq)-(column(2)-Y0)*sin(-theta_eq)):((column(1)-X0)*sin(-theta_eq)+(column(2)-Y0)*cos(-theta_eq)) with lines linewidth 2.7 lc rgb "yellow" title ""

#}

