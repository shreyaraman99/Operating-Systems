#NAME: Shreya Raman
#EMAIL: shreyaraman99@gmail.com
#ID: 004923456

default: 
	gcc -Wextra -Wall -lprofiler -pthread -std=c99 -g SortedList.c lab2_list.c -o lab2_list

tests: clean default
	./testing.sh

profile: clean default
	rm -f sprof.out profile.out
	LDPRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=sprof.out ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./sprof.out > profile.out
	pprof --list=spinLock ./lab2_list ./sprof.out >> profile.out
	rm -f sprof.out

graphs: tests
	gnuplot lab2_list.gp

dist: graphs profile
	tar -czvf lab2b-004923456.tar.gz Makefile README lab2_list.gp *.png *.csv lab2_list.c SortedList.c *.h testing.sh profile.out

clean:
	rm -rf lab2_list lab2b-004923456.tar.gz
