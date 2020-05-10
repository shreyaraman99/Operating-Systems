# general plot parameters
set terminal png
set datafile separator ","


set title "List-1: Throughput vs Number of Threads for Mutex and Spin-lock"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Number of operations/second"
set logscale y
set output 'lab2b_1.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7))\
     title 'Number of operation/sec w/mutex' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7))\
     title 'Number of operation/sec w/spin-lock' with linespoints lc rgb 'green'



set title "List-2: Average Time per Mutex Wait and per Operation"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Time per operation (ns)"
set logscale y
set output 'lab2b_2.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8)\
     title 'Average time (ns) per lock operation' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(($7))\
        title 'Time (ns) per operation w/mutex' with linespoints lc rgb 'blue', \




set title "List-3: Successful Iterations vs Threads for each Synchronization Method"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
    "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3)\
    with points lc rgb "green" title "Unprotected", \
    "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb "red" title "Mutex", \
    "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb "blue" title "Spin-Lock"



set title "List-4: Throughput vs Partitioned List with Mutex"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Number of operations/second"
set logscale y
set output 'lab2b_4.png'
set key right top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7))\
        title 'List=1' with linespoints lc rgb 'purple', \
     "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7))\
        title 'List=4' with linespoints lc rgb 'green', \
     "< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7))\
        title 'List=8' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7))\
        title 'List=16' with linespoints lc rgb 'red'



set title "List-5: Throughput vs Partitioned List with Spin-Lock"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Number of operations/second"
set logscale y 10
set output 'lab2b_5.png'
set key right top
plot \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7))\
     title 'List=1' with linespoints lc rgb 'purple', \
     "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7))\
     title 'List=4' with linespoints lc rgb 'greem', \
     "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7))\
     title 'List=8' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7))\
     title 'List=16' with linespoints lc rgb 'red'
