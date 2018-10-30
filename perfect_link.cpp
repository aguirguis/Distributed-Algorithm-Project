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
		char buf[16];
		struct sockaddr_in addr_receiver;
		socklen_t addr_receiver_size = sizeof(addr_receiver);
		recvfrom(recvack_sock, (char*) buf, 16, MSG_WAITALL, (struct sockaddr *) &addr_receiver, &addr_receiver_size);
		ack_message ack;
		memcpy(&ack,buf,16);
		acks.push_back(ack);
//		if(ack.acking_process == 1)
//			printf("Process %d received ack %d %d %d \n", my_process_id, ack.initial_sender, ack.seq_no, ack.acking_process);
	}
}
/*
    this method send a message to the intended receiver through UDP
    @param message : the message to be sent (consist of the process id of the
                    sender and the sequence number of the message to be sent)
    @param to: the process id of the receiver
*/
void perfect_link::send(int to) {
    printf("Starting PL thread..waiting for message to send to %d\n", to);
	while(true){
//		Message message;
		m_container mc;
		if (!this->messages.empty()){
			mc.num=0;
			while((!this->messages.empty()) && (mc.num<10)){
//				printf("Size of queue: %d, mc.num %d\n", messages.size(), mc.num);
				Message message = this->messages.front();
				message.sender = my_process_id;
				mc.c[mc.num] = message;
				mc.num++;
				this->messages.pop();
			}
//			printf("Done with preparing this package, process %d will send %d messages to %d\n", my_process_id, mc.num, to);
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
			int s;
			if(!(s = sendto(send_sock[to-1], (const char *)&mc, mc.num*sizeof(Message) + sizeof(int), MSG_WAITALL, (const struct sockaddr *) &addr_receiver, addr_receiver_size)))
			{
				printf("Sending message through the socket was not successful\n");
			}
//			else
//				printf("Process %d sends to %d: %d messages in %d bytes\n", my_process_id, to, mc.num, s);
			//Here, I should initiate a timeout to wait for a packet....if timeout fires, send the packet again
			ack_message exp;
			sleep(1);
			for(Message message: mc.c){
//			Message message = mc.c[0];
				exp.acking_process = to;
				exp.initial_sender = message.initial_sender;
				exp.seq_no = message.seq_no;
				exp.sender = message.sender;

				bool acked = std::find(acks.begin(), acks.end(), exp) != acks.end();
				if(acked){
					send_again = false;
				}else{
					send_again = true;
					break;
				}
			}
//			else
//				printf("Process %d will send again to %d \n", my_process_id, to);
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
//		struct Message message;
		char buf[1000];
		m_container mc;
		int r = recvfrom(recv_sock, &buf, 1000, MSG_WAITALL, ( struct sockaddr *) &addr_sender, &addr_sender_size);
//		printf("Process %d received %d bytes \n", my_process_id, r);
		memcpy(&mc, buf, r);
//		printf("Number of messages included with this received container is %d\n", mc.num);
		// check if message is already delivered
		for(int i=0;i<mc.num;i++){
			Message message = mc.c[i];
//			printf("Details of received message: %d %d %d \n", message.initial_sender, message.seq_no, message.sender);
			del_m.lock();
			bool is_delivered = std::find(delivered.begin(), delivered.end(), message) != delivered.end();
			del_m.unlock();
	//		is_delivered = false;
			if(!is_delivered) {
				// deliver the received message
				assert (bclass != NULL);
	//			if(my_process_id == 1)
	//				printf("Process %d BEB deliver %d %d \n", my_process_id, message.initial_sender, message.seq_no);
				bclass -> deliver(message);

				// add to delivered
				del_m.lock();
				delivered.push_back(message);
				del_m.unlock();
			}
			ack_message ack_m;
			ack_m.acking_process = my_process_id;
			ack_m.seq_no = message.seq_no;
			ack_m.initial_sender = message.initial_sender;
			ack_m.sender = message.sender;
			addr_sender.sin_port = htons(processes[message.sender - 1].port + 800);
			int a = sendto(send_sock[message.sender-1], (const char *)&ack_m, 16, MSG_WAITALL, (const struct sockaddr *) &addr_sender, addr_sender_size);
			usleep(1000);
			assert(a>0);
		}//end loop on received messages
//		Message message = mc.c[0];
	}
}
