reset
# Set up the multiplot layout
set term png

    do for [i=1:10]{
    set output sprintf("beam_traction%d.png",i)
    
    set xlabel "x"
    set ylabel "y"
    set yrange [-3:12]
    set xrange [-5:10]
    #set title 'Comparison-Straight Case-Fixed Points (q=0.25...0.49)'

    # Plot the first subplot
    file = sprintf("beam%d.dat",i)
   
    set grid back
    #unset colorbox
    #set xtics("0" 0, "0.25π" 0.25*pi, "0.5π" 0.5*pi, "0.75π" 0.75*pi, "π" pi)
    #set ytics("-0.08" -0.08,"-0.06" -0.06,"-0.04" -0.04,"-0.02" -0.02,"0" 0, "0.02" 0.02, "0.04" 0.04, "0.06" 0.06, "0.08" 0.08)
    # Plot the scatter plot with points colored based on stability
    plot file using 1:2 with lines linewidth 2.7 lc rgb "red" title "Beam"
    #plot file using 1:2 with points pointsize 1 pointtype 7 lc rgb "red" title sprintf('Pex')
         #file using 2:3 with points pointsize 2.2 pointtype 8 lc rgb "green" title sprintf('Pnu')
    }


