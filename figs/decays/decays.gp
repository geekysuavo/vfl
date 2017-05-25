
set terminal epscairo enhanced color font 'Times, 11pt' size 6in, 2in
set output 'decays.eps'

unset key
set xtics format ''
set ytics format ''

lx = 0.03
ly = 0.95

dkgrey = '#222222'
ltblue = '#aaaaff'
dkblue = '#0000cc'

set multiplot layout 1, 2

set label 1 '(a)' at graph lx, ly font 'Times Bold, 11pt'

set xrange [0 : 10]
set yrange [-10 : 12]
p 'ping.mdl' u 1:3:4 w filledcu lt rgb ltblue, \
  'ping.mdl' u 1:2 w l lw 2 lt rgb dkblue, \
  'ping.dat' u 1:2 w p pt 7 ps 0.1 lt rgb dkgrey

unset label 1
set label 2 '(b)' at graph lx, ly font 'Times Bold, 11pt'

set xrange [0 : 110]
unset yrange
p 'multexp.mdl' u 1:3:4 w filledcu lt rgb ltblue, \
  'multexp.mdl' u 1:2 w l lw 2 lt rgb dkblue, \
  'multexp.dat' u 1:2 w p pt 7 ps 0.1 lt rgb dkgrey

unset multiplot

