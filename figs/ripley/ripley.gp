
set terminal epscairo enhanced color font 'Times, 11pt' size 6in, 2in
set output 'ripley.eps'

unset key
set xtics format ''
set ytics format ''
set xrange [-1.2 : 1]
set yrange [-0.2 : 1.2]

lx = 0.93
ly = 0.95

ltgrey = '#666666'
dkgrey = '#222222'
dkred = '#cc0000'
dkblue = '#0000cc'

set multiplot layout 1, 2

set label 1 '(a)' at graph lx, ly font 'Times Bold, 11pt'

p 'fix.lin' w l lw 1 lc rgb ltgrey, \
  'fix.cut' w l lw 2 lt 2 lc rgb dkgrey, \
  'fix.y1.dat' w p pt 2 ps 0.3 lw 2 lc rgb dkred, \
  'fix.y0.dat' w p pt 7 ps 0.3 lc rgb dkblue

unset label 1
set label 2 '(b)' at graph lx, ly font 'Times Bold, 11pt'

p 'var.lin' w l lc rgb ltgrey, \
  'var.cut' w l lw 2 lt 2 lc rgb dkgrey, \
  'var.y1.dat' w p pt 2 ps 0.2 lw 2 lc rgb dkred, \
  'var.y0.dat' w p pt 7 ps 0.2 lc rgb dkblue

unset multiplot

