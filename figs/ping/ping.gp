
set terminal epscairo enhanced color font 'Times, 11pt' size 6in, 2in
set output 'ping.eps'

unset key
set xtics format ''
set ytics format ''
set xrange [0 : 10]
set yrange [-10 : 12]

dkgrey = '#222222'
ltblue = '#aaaaff'
dkblue = '#0000cc'

p 'ping.p0.dat' u 1:3:4 w filledcu lt rgb ltblue, \
  'ping.p0.dat' u 1:2 w l lw 2 lt rgb dkblue, \
  'ping.dat' u 1:2 w p pt 7 ps 0.1 lt rgb dkgrey

