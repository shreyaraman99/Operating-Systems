#NAME: Shreya Raman
#EMAIL: shreyaraman99@gmail.com
#ID: 004923456

.SILENT:

default:
	gcc -o lab4c_tcp lab4c_tcp.c -Wall -Wextra -g -lmraa -lm
	gcc -o lab4c_tls lab4c_tls.c -Wall -Wextra -g -lmraa -lm -lssl -lcrypto

clean:
	rm -rf lab4c_tcp lab4c_tls lab4c-004923456.tar.gz

dist:
	tar -czf lab4c-004923456.tar.gz lab4c_tcp.c lab4c_tls.c README Makefile

