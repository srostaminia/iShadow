set term postscript enhanced eps color font "Helvetica,21" size 4in,2.5in
set output "solariv_am.eps"

set xlabel "Voltage (V)"
set ylabel "Current ({/Symbol m}A)"
set y2label "Power ({/Symbol m}W)"

set key below
set y2range [0:90]
#set yrange [0:2.5]
#set noytics
#set xtics 0.5
#set ytics 10
#set ytics mirror
#set format x "10^{%T}"

set ytics nomirror
set y2tics nomirror

set style line 1 lc rgb 'black'   lt 1 lw 2
set style line 2 lc rgb 'red'     lt 1 lw 2
set style line 3 lc rgb 'blue'    lt 1 lw 2
set style line 4 lc rgb 'black'   lt 2 lw 2
set style line 5 lc rgb 'red'     lt 2 lw 2
set style line 6 lc rgb 'blue'    lt 2 lw 2
#set style line 4 lc rgb 'magenta' lt 2 lw 2
#set style line 5 lc rgb 'orange' lt 2 lw 2

set datafile separator ','

plot 'am_low.log'  using ($1):($2) with lines ls 1 title 'Low', \
     'am_low.log'  using ($1):($1*$2) with lines axis x1y1 ls 4 title '', \
     'am_med.log'  using ($1):($2) with lines ls 2 title 'Med', \
     'am_med.log'  using ($1):($1*$2) with lines axis x1y2 ls 5 title '', \
     'am_high.log' using ($1):($2) with lines ls 3 title 'High', \
     'am_high.log' using ($1):($1*$2) with lines axis x1y2 ls 6 title ''
