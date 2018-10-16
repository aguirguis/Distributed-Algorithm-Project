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
	write_log();
	out_file.close();

	//exit directly from signal handler
	exit(0);
}

class pl_deliver_callback : public deliver_callback {
    public:
        void deliver(Message message) {
            printf("prcoess %d has received message of sequence number %d from process %d\n", my_process_id, message.seq_no, message.sender);
        }
};

void test_perfect_link() {

    if(my_process_id == 1) {
        Message message;
        message.seq_no = message.sender = message.initial_sender = 1;
        perfect_link* pl = new perfect_link();
        pl -> send(message, 2);
    }
    else {
        pl_deliver_callback* callback = new pl_deliver_callback();
        perfect_link* pl = new perfect_link();
        pl -> deliver(callback);
    }

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
		}
		out_file.open("da_proc_" + to_string(my_process_id) + ".out");
	}
	else {
		printf("Fail To Open File");
	}
	membership.close();
	my_ip = processes[my_process_id].ip;
	my_port = processes[my_process_id].port;

	// create the recv socket that the process will be listening on
	recv_sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in recv_addr;
	memset(&recv_addr, 0, sizeof(recv_addr));
	socklen_t recv_addr_size = sizeof(recv_addr);
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = htons(my_port);
	recv_addr.sin_addr.s_addr = inet_addr(my_ip.c_str());
	if(bind(recv_sock, (const struct sockaddr *)&recv_addr, recv_addr_size) == SO_ERROR) {
		printf("Fail to bind the receive socket of process %d \n", my_process_id);
	}

	// create the send socket that the process will be sending by
	send_sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in send_addr;
	memset(&send_addr, 0, sizeof(send_addr));
	socklen_t send_addr_size = sizeof(send_addr);
	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(my_port + 1000);
	send_addr.sin_addr.s_addr = inet_addr(my_ip.c_str());
	if(bind(send_sock, (const struct sockaddr *)&send_addr, send_addr_size) == SO_ERROR) {
		printf("Fail to bind the sending socket of process %d \n", my_process_id);
	}

	// test perfect links
	test_perfect_link();

	//  //wait until start signal
	// while(wait_for_start) {
	// 	struct timespec sleep_time;
	// 	sleep_time.tv_sec = 0;
	// 	sleep_time.tv_nsec = 1000;
	// 	nanosleep(&sleep_time, NULL);
	// }
	//
	//
	// //broadcast messages
	// printf("Broadcasting messages.\n");
	//
	//
	// //wait until stopped
	// while(1) {
	// 	struct timespec sleep_time;
	// 	sleep_time.tv_sec = 1;
	// 	sleep_time.tv_nsec = 0;
	// 	nanosleep(&sleep_time, NULL);
	// }
	out_file.close();
}
