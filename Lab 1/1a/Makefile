.SILENT: check

all:
	gcc -g -o simpsh simpsh.c

clean:
	rm lab1a lab1-004923456.tar.gz *.txt

check:
	touch input1.txt out.txt err.txt input2.txt foo.txt input3.txt
	# TEST CASE 1:
	echo "hello world!" > input1.txt; 
	./simpsh --rdonly input1.txt --wronly out.txt --wronly err.txt --command 0 1 2 cat; 
	diff input1.txt out.txt; \
	if [[ $$? -eq 0 ]] ; then \
		echo "TEST CASE 1 PASSED!"; \
	else \
		echo "TEST CASE 1 FAILED!"; \
	fi ; \

	# TEST CASE 2:
	echo "hello world!" > input2.txt; 
	./simpsh --wronly out.txt --wronly err.txt --rdonly input2.txt --wronly foo.txt --command 2 3 1 cat; 
	diff input2.txt foo.txt; \
	if [[ $$? -eq 0 ]] ; then \
		echo "TEST CASE 2 PASSED!"; \
	else \
		echo "TEST CASE 2 FAILED!"; \
	fi ; \


dist:
	tar -czvf lab1-004923456.tar.gz simpsh.c Makefile README
