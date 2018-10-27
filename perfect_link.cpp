#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include "perfect_link.h"
using namespace std;

/*
    this method send a message to the intended receiver through UDP
    @param message : the message to be sent (consist of the process id of the
                    sender and the sequence number of the message to be sent)
    @param to: the process id of the receiver
*/
void perfect_link::send(int to) {
    printf("Starting PL thread..waiting for message to send to %d\n", to);

	while(true){
		Message message;
		if (!this->messages.empty()){
			message = this->messages.front();
			this->messages.pop();
//			printf("Process %d destined to %d , popping new message ..size now us %d \n", my_process_id, to, messages.size());
		}else{
			sleep(1);
//			printf("Process %d destined to %d queue size? %d\n", my_process_id, to, messages.size());
			continue;
		}
		// send the message (keep re-sending till an ACK is received)
		bool send_again = true;
		int ack = -1;
//		printf("++++++++++++++Process %d should send a message from sender %d of seq no %d to %d queue size %d\n", my_process_id, message.initial_sender, message.seq_no, to, this->messages.size());
		while(send_again){
		    // prepare the receiver socket address
		    struct sockaddr_in addr_receiver;
			addr_receiver.sin_family = AF_INET;
			addr_receiver.sin_port = htons(processes[to - 1].port);
			addr_receiver.sin_addr.s_addr = inet_addr(processes[to - 1].ip.c_str());
			socklen_t addr_receiver_size = sizeof(addr_receiver);
			message.sender = my_process_id;
			if(!sendto(send_sock[to-1], (const char *)&message, sizeof(message), 0, (const struct sockaddr *) &addr_receiver, addr_receiver_size))
			{
				printf("Sending message through the socket was not successful\n");
			}
//			else
//				printf("Process %d send message %d %d \n", my_process_id, message.initial_sender, message.seq_no);

			//Here, I should initiate a timeout to wait for a packet....if timeout fires, send the packet again
			fd_set set;
			FD_ZERO(&set); /* clear the set */
			FD_SET(send_sock[to-1], &set); /* add our socket to the set */
			struct timeval timeout;
			timeout.tv_sec = 0; // SOCKET_READ_TIMEOUT_SEC;
			timeout.tv_usec = 1000;	//this is in microseconds
			int recVal = select(send_sock[to-1] + 1, &set, NULL, NULL, &timeout);
			char buf[3];
			if (recVal == 0)
			{
				if(to != 2 && to != 4)
//					printf("timeout, Process %d should send again to process %d!!\n", my_process_id, to);
				send_again = true;
			}
			else
			{
				ack = recvfrom(send_sock[to-1], (char*) buf, 3, MSG_WAITALL, (struct sockaddr *) &addr_receiver, &addr_receiver_size);
//				printf("Process %d received ack from %d \n", my_process_id, to);
				if(ack > 0 && buf[0] == 'A'){
					send_again = false;
//					printf("Process %d will not resend the packet, queue size is %d\n", my_process_id, this->messages.size());
				}
//				printf("Ack received is %s \n", buf);
			}
		}//end while send_again
//		printf("At process %d, message %d %d sent to %d------------------------\n", my_process_id, message.initial_sender, message.seq_no, to);
	}//end while true
}

/*
    this method delivers the message received from UDP to the application
    @param callback: the callback function contains the action to be performed
                        when message is received
*/
void perfect_link::deliver(deliver_callback *bclass) {
	cout << "PL: waiting to deliver some messages " << endl;
	while(1){	//always true, always waiting for messages to deliver
		struct sockaddr_in addr_sender;
		socklen_t addr_sender_size = sizeof(addr_sender);
		struct Message message;
//		printf("Process %d waiting for something to receive \n", my_process_id);
		int r = recvfrom(recv_sock, &message, sizeof(message), 0, ( struct sockaddr *) &addr_sender, &addr_sender_size);
//		printf("Process %d PL: recv something? %d\n", my_process_id, r);
//		printf("ALLEEEEEEEEEEEEEEEEEEEEEEEEEEEZ Process %d Received one more message %d %d\n", my_process_id, message.initial_sender, message.seq_no);

		// check if message is already delivered
		del_m.lock();
		bool is_delivered = delivered.find(message) != delivered.end();
		del_m.unlock();
		is_delivered = false;
		if(!is_delivered) {
			// deliver the received message
			assert (bclass != NULL);
			bclass -> deliver(message);

			// add to delivered
			del_m.lock();
			delivered.insert(message);
			del_m.unlock();
		}
//		else
//			printf("Process %d ... the message %d %d has been already delivered!!!!!!! ...................\n", my_process_id, message.initial_sender, message.seq_no);
		// send an ACK
		const char * ackstr = "Ack";
		int ack = 0;
		ack = sendto(send_sock[message.sender-1], (const char *)ackstr, strlen(ackstr), 0, (const struct sockaddr *) &addr_sender, addr_sender_size);
//		printf("Process %d sent ack to %d\n", my_process_id, message.sender);
	}
}
