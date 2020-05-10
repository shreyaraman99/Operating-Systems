/* 
 *NAME: Shreya Raman
 *EMAIL: shreyaraman99@gmail.com
 *ID: 004923456
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <math.h>
#include <poll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

int logFlag = 0;
int stopFlag = 0;
int fd;
int period = 1;
char scale = 'F';
struct sockaddr_in serverAddr;
struct hostent* server;
int port;
int socketFD;
char* lf = NULL;
char* id;
char* host;

void doShutdown() {
	char stringT[10];
	time_t local;
	struct tm* localTimeUnits;

	time(&local);
	localTimeUnits = localtime(&local);
	strftime(stringT, 10, "%H:%M:%S", localTimeUnits);

	dprintf(socketFD, "%s SHUTDOWN\n", stringT);

	if(logFlag)
		dprintf(fd, "%s SHUTDOWN\n", stringT);

	exit(0);	
}

double getTemp(int temp, char scale) {
        int th = 4275;
	float r = 100000.0;
		                        
	float k = 1023.0/((double)temp) - 1.0;
	k *= r;
	float retemp = 1.0/(log(k/r)/th+1/298.15) - 273.15;
	if(scale == 'F')
		return (retemp * 9)/5 + 32;									                                     return retemp;
																	
}


void processInput(char* input){
	
	if(strcmp(input, "SCALE=C") == 0) {
		scale = 'C';
		if(logFlag)
			dprintf(fd, "%s\n", input);
	}
	else if(strcmp(input, "SCALE=F") == 0) {
		scale = 'F';
		if(logFlag)
			dprintf(fd, "%s\n", input);
	}
	else if(strcmp(input, "STOP") == 0) {
		stopFlag = 1;
		if(logFlag)
			dprintf(fd, "%s\n", input);
	}
	else if(strcmp(input, "START") == 0) {
		stopFlag = 0;
		if(logFlag)
			dprintf(fd, "%s\n", input);
	}
	else if(strncmp(input, "LOG", 3) == 0) {
		if(logFlag)
			dprintf(fd, "%s\n", input);
	}
	else if(strcmp(input, "OFF") == 0) {
		if(logFlag)
			dprintf(fd, "%s\n", input);
		doShutdown();
	}
	else if(strncmp(input, "PERIOD=", 7) == 0) {
		int n = 0;
		char tempString[20];
		while(input[n + 7] != '\0') {
			if(input[n + 7] == '\n')
				tempString[n] = '\0';	
			else
				tempString[n] = input[n + 7];
			n++;
		
		}
		
		period = atoi(tempString);
		if(logFlag)
			dprintf(fd, "%s\n", input);
	}
	else {
		fprintf(stderr, "Error: Invalid argument\n");
		exit(1);
	}
}


int main(int argc, char** argv) {
	logFlag = 0;
	static struct option opts[] = 
	{
		{"period", required_argument, NULL, 'p'},
		{"scale", required_argument, NULL, 's'},
		{"log", required_argument, NULL, 'l'},
		{"id", required_argument, NULL, 'i'},
		{"host", required_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};
	int opt = 0;
	while((opt = getopt_long(argc, argv, "p:s:l:i:h", opts, NULL)) != -1) {
		switch(opt) {
			case 'p':
				period = atoi(optarg);
				break;
			case 's':
				scale = optarg[0];
				break;
			case 'l':
				logFlag = 1;
				lf = optarg;
				if((fd = creat(optarg, S_IRWXU)) < 0) {
					perror("Error: could not access log file\n");
					exit(1);
				}
				break;
			case 'i':
				id = optarg;
				break;
			case 'h':
				host = optarg;
				break;
			default:
				fprintf(stderr, "Error: Invalid argument\n");
				exit(1);
		}
	}

	port = atoi(argv[argc-1]);
	mraa_aio_context tempsense;
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	tempsense = mraa_aio_init(1);
	server = gethostbyname(host);
	
	if(tempsense == NULL) {
		fprintf(stderr, "Error: cannot access temperature sensor\n");
		exit(1);
	}

	if (socketFD < 0) {
		perror("Error: Socket");
		exit(1);
	}

	if (server == NULL) {
		perror("Error: Host");
		exit(1);
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	memcpy((char*) &serverAddr.sin_addr.s_addr, (char*)server->h_addr, server->h_length);
	if(connect(socketFD, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {
		fprintf(stderr, "Error: cannot connect to server");
		exit(1);
	}

	dprintf(socketFD, "ID=%s\n", id);
	if(logFlag) {
		dprintf(fd, "ID=%s\n", id);
	}

	time_t tStart, tEnd, tCurrent;
	char tString[10];
	struct tm* tInfo;

	struct pollfd pollInput[1];
	pollInput[0].fd = socketFD;
	pollInput[0].events = POLLIN | POLLHUP | POLLERR;

	int temper;
	while(1) {
		temper = mraa_aio_read(tempsense);
		double tempCurrent = getTemp(temper, scale);

		time(&tCurrent);
		tInfo = localtime(&tCurrent);
		strftime(tString, 10, "%H:%M:%S", tInfo);

		dprintf(socketFD, "%s %.1f\n", tString, tempCurrent);

		if(logFlag)
			dprintf(fd, "%s %.1f\n", tString, tempCurrent);

		time(&tStart);
		time(&tEnd);

		while(difftime(tEnd, tStart) < period) {
			if(poll(pollInput, 1, 0) < 0) {
				perror("Error: Poll error\n");
				exit(1);
			}


			if(pollInput[0].revents & POLLIN) {
				char inp[256];
				int bRead;
				if((bRead = read(socketFD, inp, 256)) < 0) {
					perror("Error: Read\n");
					exit(1);
				}
				inp[bRead-1] = '\0';
				processInput(inp);
			}

			if(!stopFlag)
				time(&tEnd);
		}
	}
	mraa_aio_close(tempsense);
	exit(0);

}


