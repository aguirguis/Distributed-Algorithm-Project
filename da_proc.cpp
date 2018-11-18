#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>         // std::thread
#include <unistd.h>			//sleep
// #include "frb.h"
#include "lcb.h"
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
	printf("Immediately stopping network packet processing at process %d.\n", my_process_id);

	//write/flush output file if necessary
	printf("Writing output....number of lines in log: %d \n", log_pointer);
	write_log();


	// close the sockets
	if(signum == SIGTERM) {
		int close1 = close(recv_sock);
		assert(close1 >= 0);
		int close2 = close(recvack_sock);
		assert(close2 >= 0);
		int close3 = close(send_sock_all);
		assert(close3 >= 0);
		out_file.close();
	}

	//exit directly from signal handler
	exit(0);
}

class pl_deliver_callback : public deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")
    public:
        void deliver(Message message) {
            printf("prcoess %d has received message of sequence number %d from process %d\n", my_process_id, message.seq_no, message.initial_sender);
        }
};
/*
void test_perfect_link() {

    if(my_process_id == 1) {
        Message message;
        message.seq_no = message.sender = message.initial_sender = 1;
        perfect_link* pl = new perfect_link();
        pl->messages.push(message);
        pl -> send(2);
    }
    else {
        pl_deliver_callback* callback = new pl_deliver_callback();
        perfect_link* pl = new perfect_link();
        pl -> deliver(callback);
    }
}
*/
beb test_bebBroadcast(){
	beb bb;
	bb.init(0);
	return bb;
}

int main(int argc, char** argv) {

	//set signal handlers
	signal(SIGUSR2, start);
	signal(SIGTERM, stop);
	signal(SIGINT, stop);


	//parse arguments, including membership
	//initialize application
	my_process_id = atoi(argv[1]);
	ifstream membership (argv[2]);
	int num_messages = atoi(argv[3]);
	printf("Initializing at process %d.\n", my_process_id);
	if(membership.is_open()) {
		membership >> nb_of_processes;
		// read each process information
		for(int i = 0; i < nb_of_processes; i++) {
			membership >> processes[i].id;
			membership >> processes[i].ip;
			membership >> processes[i].port;
		}
		// read each process dependencies
		std::string line;
		int n;
		std::getline(membership, line); // this is placed to read the end of line
		for(int i = 0; i < nb_of_processes; i++) {
			std::getline(membership, line);
			std::istringstream iss(line);
			while (iss >> n)
    			processes_dependencies[i].push_back(n);
		}
		out_file.open("da_proc_" + to_string(my_process_id) + ".out");
	}
	else {
		printf("Fail To Open File");
	}
	//some assertions for our assumptions
	assert(nb_of_processes <= MAX_PROCESSES_NUM);
	assert(num_messages <= MAX_MESSAGE_NUM);
	assert(my_process_id <= nb_of_processes);
	messages_log = new LogMessage[MAX_LOG_FILE];
	membership.close();
	my_ip = processes[my_process_id - 1].ip;
	my_port = processes[my_process_id - 1].port;
	my_dependencies = processes_dependencies[my_process_id -1];
	// create the recv socket that the process will be listening on
	recv_sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(recv_sock > 0);
	struct sockaddr_in recv_addr;
	memset(&recv_addr, 0, sizeof(recv_addr));
	socklen_t recv_addr_size = sizeof(recv_addr);
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = htons(my_port);
	recv_addr.sin_addr.s_addr = inet_addr(my_ip.c_str());
	if(bind(recv_sock, (const struct sockaddr *)&recv_addr, recv_addr_size) == SO_ERROR) {
		printf("Fail to bind the receive socket of process %d \n", my_process_id);
		exit(1);
	}

	recvack_sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(recvack_sock > 0);
	struct sockaddr_in recvack_addr;
	memset(&recvack_addr, 0, sizeof(recvack_addr));
	socklen_t recvack_addr_size = sizeof(recvack_addr);
	recvack_addr.sin_family = AF_INET;
	recvack_addr.sin_port = htons(my_port+800);
	recvack_addr.sin_addr.s_addr = inet_addr(my_ip.c_str());
	if(bind(recvack_sock, (const struct sockaddr *)&recvack_addr, recvack_addr_size) == SO_ERROR) {
		printf("Fail to bind the receive ack socket of process %d \n", my_process_id);
		exit(1);
	}

	send_sock_all = socket(AF_INET, SOCK_DGRAM, 0);
	assert(send_sock_all > 0);
	struct sockaddr_in send_addr;
	memset(&send_addr, 0, sizeof(send_addr));
	socklen_t send_addr_size = sizeof(send_addr);
	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(my_port + 1000);
	send_addr.sin_addr.s_addr = inet_addr(my_ip.c_str());
	if(bind(send_sock_all, (const struct sockaddr *)&send_addr, send_addr_size) == SO_ERROR) {
		printf("Fail to bind the sending socket of process %d \n", my_process_id);
		exit(1);
	}

	//  //wait until start signal
	 while(wait_for_start) {
	 	struct timespec sleep_time;
	 	sleep_time.tv_sec = 0;
	 	sleep_time.tv_nsec = 1000;
	 	nanosleep(&sleep_time, NULL);
	 }


	 //test lcb_broadcast
	 lcb lcb_instance;
	 lcb_instance.init();
	 printf("Broadcasting messages at process %d.\n", my_process_id);
	 for(int i = 0; i < num_messages; i++) {
		Message m;
		lcb_instance.lcb_broadcast(m);
	 }
	 lcb_instance.urb_instance.bbb.recv.join();
	 lcb_instance.urb_instance.bbb.recv_ack.join();
	 lcb_instance.urb_instance.bbb.resend.join();
	 lcb_instance.urb_instance.bbb.send.join();

	// test frb_broadcast
	// frb fb;
	// fb.init(new pl_deliver_callback());
	//  printf("Broadcasting messages at process %d.\n", my_process_id);
	//  for(int i = 0; i < num_messages; i++) {
	// 	Message m;
	// 	fb.frb_broadcast(m);
	// }
	// fb.urb_instance.bbb.recv.join();
	// fb.urb_instance.bbb.recv_ack.join();
	// fb.urb_instance.bbb.resend.join();
	// fb.urb_instance.bbb.send.join();

	 //wait until stopped
	 while(1) {
	 	struct timespec sleep_time;
	 	sleep_time.tv_sec = 1;
	 	sleep_time.tv_nsec = 0;
	 	nanosleep(&sleep_time, NULL);
	 }
	out_file.close();
}
