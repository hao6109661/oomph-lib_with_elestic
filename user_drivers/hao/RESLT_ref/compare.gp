reset
# Set up the multiplot layout
set term png

    set output sprintf("compare.png")
    
    set xlabel "d"
    set ylabel "Pex, Pnu"
    #set xrange [0:3.15]
    #set yrange [-0.08:0.08]
    #set title 'Comparison-Straight Case-Fixed Points (q=0.25...0.49)'

    # Plot the first subplot
    file = sprintf("trace_beam.dat")
   
    set grid back
    #unset colorbox
    #set xtics("0" 0, "0.25π" 0.25*pi, "0.5π" 0.5*pi, "0.75π" 0.75*pi, "π" pi)
    #set ytics("-0.08" -0.08,"-0.06" -0.06,"-0.04" -0.04,"-0.02" -0.02,"0" 0, "0.02" 0.02, "0.04" 0.04, "0.06" 0.06, "0.08" 0.08)
    # Plot the scatter plot with points colored based on stability
    plot file using 2:1 with points pointsize 1 pointtype 7 lc rgb "red" title sprintf('Pex'), \
         file using 2:3 with points pointsize 2.2 pointtype 8 lc rgb "green" title sprintf('Pnu')


