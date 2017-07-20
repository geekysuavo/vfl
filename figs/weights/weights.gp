
set terminal epscairo enhanced dashed color font 'Times, 11pt' size 6in, 2in
set output 'weights.eps'

unset key
set y2tics
set yrange [-2 : 2]
set xlabel 'Feature index'
set ylabel 'Feature weight' off 1.7, 0
set ytics nomirror

lx = 0.03
ly = 0.95

ltgrey = '#bbbbbb'
dkgrey = '#222222'
mdgrey = '#444444'
dkred = '#cc0000'

f(x) = 1./(1.+exp(-x))

set multiplot layout 1, 2

set xrange [1 : 25]
set label 1 '(a)' at graph lx, ly font 'Times Bold, 11pt'

p \
 'fix.dat' u 1:($2-$3):($2+$3) w filledc lc rgb ltgrey, \
 'fix.dat' u 1:($2) w lp lt 1 pt 5 ps 0.2 lw 2 lc rgb dkgrey, \
 0 w l lt 2 lc rgb mdgrey, \
 'fix.dat' u 1:($4*$5) w l lt 1 lc rgb dkred ax x1y2

unset label 1
set y2tics 0.01
set xrange [1 : 10]
unset ylabel
set y2label 'Feature area' off -1, 0
set label 2 '(b)' at graph lx, ly font 'Times Bold, 11pt'

p \
 'var.dat' u 1:($2-$3):($2+$3) w filledc lc rgb ltgrey, \
 'var.dat' u 1:($2) w lp lt 1 pt 5 ps 0.2 lw 2 lc rgb dkgrey, \
 0 w l lt 2 lc rgb mdgrey, \
 'var.dat' u 1:($4*$5) w l lt 1 lc rgb dkred ax x1y2

unset multiplot

