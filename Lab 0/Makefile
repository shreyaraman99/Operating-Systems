# NAME: Shreya Raman
# EMAIL: shreyaraman99@gmail.com
# ID: 004923456

FILES=lab0.c Makefile README backtrace.png breakpoint.png
.SILENT:

default:
	gcc -Wall -Wextra -g lab0.c -o lab0

check: smokeTests

smokeTests: testSuccess testOpenFailure testOutputFailure testSegFaultAndCatch testIncorrect

clean:
	rm -f  lab0 lab0-004923456.tar.gz

dist:
	tar -czf lab0-004923456.tar.gz $(FILES)

# smoke check cases below
testSuccess:
	echo "Hello World" > input.txt;\
	./lab0 --input=input.txt --output=output.txt ; \
	if [ $$? -ne 0 ] ; \
	then\
		echo "testSuccess failed" ;\
	fi;\
	rm *.txt 

testOutputFailure:
	echo "Hello World" > input.txt;\
        touch output.txt;\
        chmod u-w output.txt ;\
        ./lab0 --input=input.txt --output=output.txt ; \
        if [ $$? -ne 3 ] ;\
        then\
                echo "testOutputFailure failed";\
        fi;\
        chmod u+w output.txt
	rm *.txt

testOpenFailure:
	echo "Hello World" > input.txt;\
	chmod ugo-r input.txt ;\
	./lab0 --input=input.txt --output=output.txt ; \
	if [ $$? -ne 2 ] ;\
	then\
		echo "testOpenFailure failed" ;\
	fi;\
	chmod 777 input.txt ;\
	rm *.txt

testSegFaultAndCatch:
	./lab0 --segfault --catch;\
	if [ $$? -ne 4 ];\
	then\
		echo "testSegFaultAndCatch";\
	fi;\

testIncorrect:
	echo "Hello World" > input.txt; \
        ./lab0 --input=input.txt --x &>/dev/null; \
        if [ $$? -n1 1 ];\
        then\
                echo "testIncorrect failed" ;\
        fi ;\
        rm *.txt
