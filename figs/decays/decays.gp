
set terminal epslatex color size 6in, 2in header \
 '\newcommand{\ft}[0]{\footnotesize}'

set output 'decays.tex'

lx = 0.10
ly = 0.90

dkgrey = '#222222'
ltblue = '#aaaaff'
dkblue = '#0000cc'

unset key
set tmargin 0.2
set bmargin 1.5
set lmargin 3
set rmargin 1
set multiplot layout 1, 2

set label 1 '{\ft\bf (a)}' at graph lx, ly

set xlabel '$\m{x}$' off 0, 0.5
set ylabel '$y$' off 5, -0.5

set xtics 5 format '{\ft %g}'
set ytics 10 format '{\ft %g}'

set xrange [0 : 10]
set yrange [-10 : 12]

p 'ping.mdl' u 1:3:4 w filledcu lt rgb ltblue, \
  'ping.mdl' u 1:2 w l lw 2 lt rgb dkblue, \
  'ping.dat' u 1:2 w p pt 7 ps 0.3 lt rgb dkgrey

unset yrange
unset ylabel
unset label 1
set label 2 '{\ft\bf (b)}' at graph lx, ly

set xtics 25 format '{\ft %g}'
set ytics 100 format '{\ft %g}' off -0.5

set xrange [0 : 110]

p 'multexp.mdl' u 1:3:4 w filledcu lt rgb ltblue, \
  'multexp.mdl' u 1:2 w l lw 2 lt rgb dkblue, \
  'multexp.dat' u 1:2 w p pt 7 ps 0.3 lt rgb dkgrey

unset multiplot

