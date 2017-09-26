
set terminal epslatex color size 6in, 2in header \
 '\newcommand{\ft}[0]{\footnotesize}'

set output 'ripley.tex'

lx = 0.93
ly = 0.92

ltgrey = '#666666'
dkgrey = '#222222'
dkred = '#cc0000'
dkblue = '#0000cc'

unset key

set xtics format ''
set ytics format ''

set xrange [-1.2 : 1]
set yrange [-0.2 : 1.2]

set xlabel '$x_1$' off 0, 0.5
set ylabel '$x_2$' off 1

set rmargin 1
set multiplot layout 1, 2

set label 1 '{\ft\bf (a)}' at graph lx, ly

p 'fix.lin' w l lw 1 lc rgb ltgrey, \
  'fix.cut' w l lw 2 lt 2 lc rgb dkgrey, \
  'fix.y1.dat' w p pt 2 ps 0.5 lw 2 lc rgb dkred, \
  'fix.y0.dat' w p pt 7 ps 0.5 lc rgb dkblue

unset label 1
set label 2 '{\ft\bf (b)}' at graph lx, ly

p 'var.lin' w l lc rgb ltgrey, \
  'var.cut' w l lw 2 lt 2 lc rgb dkgrey, \
  'var.y1.dat' w p pt 2 ps 0.5 lw 2 lc rgb dkred, \
  'var.y0.dat' w p pt 7 ps 0.5 lc rgb dkblue

unset multiplot

