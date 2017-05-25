
set terminal epscairo enhanced color font 'Times, 11pt' size 6in, 3in
set output 'environ.eps'

unset key
set ytics format ''

lx = 0.03
ly = 0.90

dkgrey = '#222222'
ltblue = '#aaaaff'
dkblue = '#0000cc'
ltred  = '#ffaaaa'
dkred  = '#cc0000'

set multiplot layout 2, 1

set label 1 '(a)' at graph lx, ly font 'Times Bold, 11pt'

set xrange [1970 : 2060]
p 'co2-a.mdl' u 1:3:4 w filledcu lt rgb ltblue, \
  'co2-a.mdl' u 1:2 w l lw 2 lt rgb dkblue, \
  'co2-b.mdl' u 1:3:4 w filledcu lt rgb ltred, \
  'co2-b.mdl' u 1:2 w l lw 2 lt rgb dkred, \
  'co2.dat' u 1:2 w p pt 7 ps 0.1 lt rgb dkgrey

unset label 1
set label 2 '(b)' at graph lx, ly font 'Times Bold, 11pt'

set xrange [1980 : 2060]
p 'ch4-a.mdl' u 1:3:4 w filledcu lt rgb ltblue, \
  'ch4-a.mdl' u 1:2 w l lw 2 lt rgb dkblue, \
  'ch4-b.mdl' u 1:3:4 w filledcu lt rgb ltred, \
  'ch4-b.mdl' u 1:2 w l lw 2 lt rgb dkred, \
  'ch4.dat' u 1:2 w p pt 7 ps 0.1 lt rgb dkgrey

unset multiplot

