
set terminal epslatex color size 6in, 2in header \
 '\newcommand{\ft}[0]{\footnotesize}'

set output 'weights.tex'

lx = 0.08
ly = 0.92

ltgrey = '#bbbbbb'
dkgrey = '#222222'
mdgrey = '#444444'
dkred = '#cc0000'

f(x) = 1./(1.+exp(-x))

unset key
set tmargin 0.2
set bmargin 1.5
set lmargin 4
set rmargin 5

set xtics 5 format '{\ft %g}'
set ytics 1 format '{\ft %g}'
set y2tics 5 format '{\ft %g}'
set ytics nomirror

set xrange [1 : 25]
set yrange [-2 : 2]

set xlabel '$j$' off 0, 0.5
set ylabel '$w_j$' off 3, 0

set multiplot layout 1, 2

set label 1 '{\ft\bf (a)}' at graph lx, ly

p \
 'fix.dat' u 1:($2-$3):($2+$3) w filledc lc rgb ltgrey, \
 'fix.dat' u 1:($2) w lp lt 1 pt 5 ps 0.5 lw 3 lc rgb dkgrey, \
 0 w l lt 2 lc rgb mdgrey, \
 'fix.dat' u 1:($4*$5) w l lt 1 lw 2 lc rgb dkred ax x1y2

unset label 1
set label 2 '{\ft\bf (b)}' at graph lx, ly

set xtics 2 format '{\ft %g}'
set y2tics 0.01 format '{\ft %g}' off 1

set xrange [1 : 10]
set yrange [-2 : 2]

set lmargin 2
set rmargin 7

unset ylabel
set y2label '$\tau_{j,1} \cdot \tau_{j,2}$' off -3.5, 0

p \
 'var.dat' u 1:($2-$3):($2+$3) w filledc lc rgb ltgrey, \
 'var.dat' u 1:($2) w lp lt 1 pt 5 ps 0.5 lw 3 lc rgb dkgrey, \
 0 w l lt 2 lc rgb mdgrey, \
 'var.dat' u 1:($4*$5) w l lt 1 lw 2 lc rgb dkred ax x1y2

unset multiplot

