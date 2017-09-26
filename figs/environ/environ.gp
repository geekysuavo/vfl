
set terminal epslatex color size 6in, 3in header \
 '\newcommand{\ft}[0]{\footnotesize}'

set output 'environ.tex'

lx = 0.03
ly = 0.90

dkgrey = '#222222'
ltblue = '#aaaaff'
dkblue = '#0000cc'
ltred  = '#ffaaaa'
dkred  = '#cc0000'

unset key
set lmargin 7
set rmargin 1
set multiplot layout 2, 1

set label 1 '{\ft\bf (a)}' at graph lx, ly

set ylabel 'CO$_2$ / ppm' off 3

set xtics 20 format '{\ft %g}'
set ytics 50 format '{\ft %g}' off -0.5

set xrange [1970 : 2060]

p 'co2-a.mdl' u 1:3:4 w filledcu lt rgb ltblue, \
  'co2-a.mdl' u 1:2 w l lw 2 lt rgb dkblue, \
  'co2-b.mdl' u 1:3:4 w filledcu lt rgb ltred, \
  'co2-b.mdl' u 1:2 w l lw 2 lt rgb dkred, \
  'co2.dat' u 1:2 w p pt 7 ps 0.3 lt rgb dkgrey

unset label 1
set label 2 '{\ft\bf (b)}' at graph lx, ly

set ylabel 'CH$_4$ / ppb' off 3

set xtics 10 format '{\ft %g}'
set ytics 200 format '{\ft %g}' off -1

set xrange [1980 : 2060]

p 'ch4-a.mdl' u 1:3:4 w filledcu lt rgb ltblue, \
  'ch4-a.mdl' u 1:2 w l lw 2 lt rgb dkblue, \
  'ch4-b.mdl' u 1:3:4 w filledcu lt rgb ltred, \
  'ch4-b.mdl' u 1:2 w l lw 2 lt rgb dkred, \
  'ch4.dat' u 1:2 w p pt 7 ps 0.3 lt rgb dkgrey

unset multiplot

