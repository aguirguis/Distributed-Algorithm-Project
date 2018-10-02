/*
 * da_proc.cpp
 *
 *  Created on: 2 Oct 2018
 *      Author: aguirguis
 */

/*
 * da_proc.cpp
 *
 *  Created on: 30 Sep 2018
 *      Author: aguirguis
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string>
#define MAX_PROCESSES_NUM 10;

using namespace std;
static int wait_for_start = 1;
struct Process{
	int id;
	string ip;
	int port;
};

//Process neighbors[MAX_PROCESSES_NUM];

static void start(int signum) {
	wait_for_start = 0;
}

static void stop(int signum) {
	//reset signal handlers to default
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);

	//immediately stop network packet processing
	printf("Immediately stopping network packet processing.\n");

	//write/flush output file if necessary
	printf("Writing output.\n");

	//exit directly from signal handler
	exit(0);
}

int main(int argc, char** argv) {

	//set signal handlers
	signal(SIGUSR1, start);
	signal(SIGTERM, stop);
	signal(SIGINT, stop);


	//parse arguments, including membership
	//initialize application
	//start listening for incoming UDP packets
	printf("Initializing.\n");


	//wait until start signal
	while(wait_for_start) {
		struct timespec sleep_time;
		sleep_time.tv_sec = 0;
		sleep_time.tv_nsec = 1000;
		nanosleep(&sleep_time, NULL);
	}


	//broadcast messages
	printf("Broadcasting messages.\n");


	//wait until stopped
	while(1) {
		struct timespec sleep_time;
		sleep_time.tv_sec = 1;
		sleep_time.tv_nsec = 0;
		nanosleep(&sleep_time, NULL);
	}
}


