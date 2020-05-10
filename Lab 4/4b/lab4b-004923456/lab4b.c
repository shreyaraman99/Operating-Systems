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

int logFlag = 0;
int stopFlag = 0;
int fd;
int period = 1;
char scale = 'F';


void shutdown() {
	char stringT[10];
	time_t local;
	struct tm* localTimeUnits;

	time(&local);
	localTimeUnits = localtime(&local);
	strftime(stringT, 10, "%H:%M:%S", localTimeUnits);

	fprintf(stdout, "%s SHUTDOWN\n", stringT);

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
		shutdown();
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
	static struct option opts[] = 
	{
		{"period", required_argument, NULL, 'p'},
		{"scale", required_argument, NULL, 's'},
		{"log", required_argument, NULL, 'l'},
		{0, 0, 0, 0}
	};
	int opt = 0;
	while((opt = getopt_long(argc, argv, "p:s:l", opts, NULL)) != -1) {
		switch(opt) {
			case 'p':
				period = atoi(optarg);
				break;
			case 's':
				if(strlen(optarg) == 1 && (optarg[0] == 'F' || optarg[0] == 'C'))
					scale = optarg[0];
				else {
					fprintf(stderr, "Error: Invalid argument\n");
					exit(1);
				}
				break;
			case 'l':
				logFlag = 1;
				if((fd = creat(optarg, S_IRWXU)) < 0) {
					perror("Error: could not access log file\n");
					exit(1);
				}
				break;
			default:
				fprintf(stderr, "Error: Invalid argument\n");
				exit(1);
		}
	}

	mraa_aio_context tempsense;
	mraa_gpio_context button;
	tempsense = mraa_aio_init(1);
	button = mraa_gpio_init(60);
	
	if(tempsense == NULL) {
		fprintf(stderr, "Error: cannot access temperature sensor\n");
		exit(1);
	}

	if(button == NULL) {
		fprintf(stderr, "Error: cannot access button\n");
		exit(1);
	}

	mraa_gpio_dir(button, MRAA_GPIO_IN);

	time_t tStart, tEnd, tCurrent;
	char tString[10];
	struct tm* tInfo;

	struct pollfd pollInput[1];
	pollInput[0].fd = STDIN_FILENO;
	pollInput[0].events = POLLIN | POLLHUP | POLLERR;

	int temper;
	while(1) {
		temper = mraa_aio_read(tempsense);
		double tempCurrent = getTemp(temper, scale);

		time(&tCurrent);
		tInfo = localtime(&tCurrent);
		strftime(tString, 10, "%H:%M:%S", tInfo);

		fprintf(stdout, "%s %.1f\n", tString, tempCurrent);

		if(logFlag)
			dprintf(fd, "%s %.1f\n", tString, tempCurrent);

		time(&tStart);
		time(&tEnd);

		while(difftime(tEnd, tStart) < period) {
			if(mraa_gpio_read(button))
				shutdown();

			int retP;
			if((retP = poll(pollInput, 1, 0)) < 0) {
				perror("Error: Poll error\n");
				exit(1);
			}

			if(pollInput[0].revents & POLLIN) {
				char inp[20];
				if(scanf("%[^\n]%*c", inp) != -1) {
					processInput(inp);
				}
			}

			if(!stopFlag)
				time(&tEnd);
		}
	}
	mraa_gpio_close(button);
	mraa_aio_close(tempsense);
	exit(0);

}


