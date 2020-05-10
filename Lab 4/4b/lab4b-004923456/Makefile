#NAME: Shreya Raman
#EMAIL: shreyaraman99@gmail.com
#ID: 004923456

default:
	gcc -o lab4b -g -lm -lmraa -Wall -Wextra lab4b.c

check:
	
check_invalid:
	@./lab4b --bogus > /dev/null 2>&1; \
	if [ $$? -ne 1 ]; then \
		echo "Failed test: bogus option didn't result in exit code 1"; \
		exit 1; \
	fi

check_valid:
	@./lab4b --scale=C --period=10 --log=smokeTestFile > /dev/null 2>&1; \
	if [ $$? -ne 0 ]; then \
		echo: "Failed test: valid option didn't result in exit code 0"; \
		exit 1; \
	fi
	@./rm smokeTestFile
	
clean:
	rm -f lab4b lab4b-004923456.tar.gz
	
dist: 
	tar -czvf lab4b-004923456.tar.gz lab4b.c Makefile README
