#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <thread>         // std::thread
#include "perfect_link.h"
using namespace std;

class pl_deliver_callback : public deliver_callback {
    public:
        void deliver(Message message, int sender) {
            printf("prcoess %d has received message of sequence number %d from process %d\n", my_process_id, message.seq_no, sender);
        }
};

int main(int argc, char** argv) {

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
	if(!bind(recv_sock, (const struct sockaddr *)&recv_addr, recv_addr_size)) {
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
	if(!bind(send_sock, (const struct sockaddr *)&send_addr, send_addr_size)) {
		printf("Fail to bind the sending socket of process %d \n", my_process_id);
	}

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

    return 0;
}
