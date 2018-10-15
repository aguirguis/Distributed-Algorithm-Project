#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <thread>         // std::thread
#include "beb.h"

using namespace std;

static int wait_for_start = 1;

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
	// write_log();

	//exit directly from signal handler
	exit(0);
}

//This function writes the logs received to the log file
static void write_log(){
	ofstream out_file;
	out_file.open("da_proc_" + to_string(my_process_id) + ".out");
	for(int i = 0; i < log_pointer; i++) {
		if(messages_log[i].message_type == 'b')
			out_file << "b " << messages_log[i].seq_nr << "\n";
		else
			out_file << "d " << messages_log[i].sender << " " << messages_log[i].seq_nr << "\n";
	}
	out_file.close();
	log_pointer = 0;	//return the point to the beginning
}

int main(int argc, char** argv) {

	//set signal handlers
	signal(SIGUSR1, start);
	signal(SIGTERM, stop);
	signal(SIGINT, stop);


	//parse arguments, including membership
	//initialize application
	printf("Initializing.\n");
	my_process_id = atoi(argv[1]);
	ifstream membership (argv[2]);
	if(membership.is_open()) {
		membership >> nb_of_processes;
		for(int i = 1; i <= nb_of_processes; i++) {
			membership >> processes[i].id;
			membership >> processes[i].ip;
			membership >> processes[i].port;

			// create a socket for this process
            sockets[i] = socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in addr_process;
            memset(&addr_process, 0, sizeof(addr_process));
            socklen_t addr_size = sizeof(addr_process);
            addr_process.sin_family = AF_INET;
       	    addr_process.sin_port = htons(processes[i].port);
       	    addr_process.sin_addr.s_addr = inet_addr(processes[i].ip.c_str());
            if(!bind(sockets[i], (const struct sockaddr *)&addr_process, addr_size))
            {
                printf("Fail to bind socket of process %d \n", i);
            }
		}
	}
	else {
		printf("Fail To Open File");
	}
	membership.close();
	my_ip = processes[my_process_id].ip;
	my_port = processes[my_process_id].port;

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
