#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include "perfect_link.h"
using namespace std;


//This method works in one separate threads to receive acks
void perfect_link::recv_ack(){

	while(true){
		char buf[12];
		struct sockaddr_in addr_receiver;
		socklen_t addr_receiver_size = sizeof(addr_receiver);
		recvfrom(recvack_sock, (char*) buf, 12, 0, (struct sockaddr *) &addr_receiver, &addr_receiver_size);
		ack_message ack;
		memcpy(&ack,buf,12);
		acks.push_back(ack);
	}
}
/*
    this method send a message to the intended receiver through UDP
    @param message : the message to be sent (consist of the process id of the
                    sender and the sequence number of the message to be sent)
    @param to: the process id of the receiver
*/
void perfect_link::send(int to, perfect_link* recv_acks) {
    printf("Starting PL thread..waiting for message to send to %d\n", to);
	while(true){
		Message message;
		if (!this->messages.empty()){
			message = this->messages.front();
			this->messages.pop();
		}else{
			usleep(1000);
			continue;
		}
		// send the message (keep re-sending till an ACK is received)
		bool send_again = true;
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
			//Here, I should initiate a timeout to wait for a packet....if timeout fires, send the packet again
			ack_message exp;
			exp.acking_process = to;
			exp.initial_sender = my_process_id;
			exp.seq_no = message.seq_no;

			usleep(1000);
			bool acked = std::find(recv_acks->acks.begin(), recv_acks->acks.end(), exp) != recv_acks->acks.end();
			if(acked)
				send_again = false;
		}//end while send_again
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
		recvfrom(recv_sock, &message, sizeof(message), 0, ( struct sockaddr *) &addr_sender, &addr_sender_size);
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
		ack_message ack_m;
		ack_m.acking_process = my_process_id;
		ack_m.seq_no = message.seq_no;
		ack_m.initial_sender = message.sender;
		addr_sender.sin_port = htons(processes[message.sender - 1].port + 800);
		sendto(send_sock[message.sender-1], (const char *)&ack_m, 12, 0, (const struct sockaddr *) &addr_sender, addr_sender_size);
	}
}
