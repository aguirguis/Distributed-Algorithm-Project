#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include "perfect_link.h"
using namespace std;

void perfect_link::send_all() {
	while(true) {
		for(int i = 0; i < nb_of_processes; i++) {
			if(processes[i].id != my_process_id) {
				send(processes[i].id);
			}
		}
		sleep(1);
	}
}
/*
    this method send a message to the intended receiver through UDP
    @param message : the message to be sent (consist of the process id of the
                    sender and the sequence number of the message to be sent)
    @param to: the process id of the receiver
*/
void perfect_link::send(int to) {
    // printf("Starting PL thread..waiting for message to send to %d\n", to);
	m_container mc;
	if (!this->messages_all[to - 1].empty()) {
		mc.num=0;
		while((!this->messages_all[to - 1].empty()) && (mc.num<MAX_CONTAINER_NUM)){
			Message message = this->messages_all[to - 1].front();
			message.sender = my_process_id;
			mc.c[mc.num] = message;
			mc.num++;
			this->messages_all[to - 1].pop();
		}
	}
	else {
		return;
	}
	struct sockaddr_in addr_receiver;
	addr_receiver.sin_family = AF_INET;
	addr_receiver.sin_port = htons(processes[to - 1].port);
	addr_receiver.sin_addr.s_addr = inet_addr(processes[to - 1].ip.c_str());
	socklen_t addr_receiver_size = sizeof(addr_receiver);
	int s;
	send_sock_m.lock();
		if(!(s = sendto(send_sock_all, (const char *)&mc, MAX_CONTAINER_NUM*sizeof(Message) + sizeof(int), MSG_WAITALL, (const struct sockaddr *) &addr_receiver, addr_receiver_size)))
		{
			printf("Sending message through the socket was not successful\n");
		}
//		else
//			printf("Process %d sends container of %d messages to %d \n", my_process_id, mc.num, to);
	send_sock_m.unlock();
	for(int i = 0; i < mc.num; i++){
		Message message = mc.c[i];
		un_acked_messages_m.lock();
			un_acked_messages[to - 1].push_back(message);
		un_acked_messages_m.unlock();
	}
}

/*
    this method delivers the message received from UDP to the application
    @param callback: the callback function contains the action to be performed
                        when message is received
*/
void perfect_link::deliver(deliver_callback *bclass) {
	cout << "PL: waiting to deliver some messages " << endl;
        char buf[10000];
        m_container mc;
        ack_container ac;
	while(1){	//always true, always waiting for messages to deliver
		struct sockaddr_in addr_sender;
		socklen_t addr_sender_size = sizeof(addr_sender);
//		char buf[10000];
//		m_container mc;
		int r = recvfrom(recv_sock, &buf, 10000, MSG_WAITALL, ( struct sockaddr *) &addr_sender, &addr_sender_size);
		memcpy(&mc, buf, r);
		// check if message is already delivered
//		printf("Process %d received a container of %d messages from %d \n", my_process_id, mc.num, mc.c[0].sender);
//		ack_container ac;
		ac.num = mc.num;
		for(int i=0;i<mc.num;i++){
			Message message = mc.c[i];
			bool is_delivered = std::find(delivered.begin(), delivered.end(), message) != delivered.end();
			if(!is_delivered) {
				// deliver the received message
				assert (bclass != NULL);
				bclass -> deliver(message);

				// add to delivered
				delivered.push_back(message);
			}
			ack_message ack_m;
			ack_m.acking_process = my_process_id;
			ack_m.message = message;
			ac.a[i] = ack_m;
		}//end for loop
        addr_sender.sin_port = htons(processes[mc.c[0].sender - 1].port + 800);
        send_sock_m.lock();
        int a = sendto(send_sock_all, (const char *)&ac, MAX_CONTAINER_NUM*sizeof(ack_message) + sizeof(int), MSG_WAITALL, (const struct sockaddr *) &addr_sender, addr_sender_size);
        send_sock_m.unlock();
	}//end while True
}

// This method works in one separate threads to receive acks
void perfect_link::recv_ack(){
        char buf[10000];
        ack_container ac;
	while(true){
		struct sockaddr_in addr_receiver;
		socklen_t addr_receiver_size = sizeof(addr_receiver);
		int r = recvfrom(recvack_sock, (char*) buf, 10000, MSG_WAITALL, (struct sockaddr *) &addr_receiver, &addr_receiver_size);
//		ack_container ac;
		memcpy(&ac,buf,r);

		for(int i=0;i<ac.num;i++){
			ack_message ack = ac.a[i];
			int to = ack.acking_process;
			un_acked_messages_m.lock();
			un_acked_messages[to - 1].erase(std::remove(un_acked_messages[to - 1].begin(), un_acked_messages[to - 1].end(), ack.message), un_acked_messages[to - 1].end());
			un_acked_messages_m.unlock();
		}//end for loop
	}//end while true
}


// This method works in one separate threads to resend unacked messages
void perfect_link::resend() {
        m_container mc;
	while(true) {
//		usleep(1000);
		for(int i = 0; i < nb_of_processes; i++) {
			if(!un_acked_messages[i].empty()) {

				int to = processes[i].id;
				struct sockaddr_in addr_receiver;
				addr_receiver.sin_family = AF_INET;
				addr_receiver.sin_port = htons(processes[to - 1].port);
				addr_receiver.sin_addr.s_addr = inet_addr(processes[to - 1].ip.c_str());
				socklen_t addr_receiver_size = sizeof(addr_receiver);
				int s;

//				m_container mc;
				mc.num = 0;
				// resend all messages that are remaind unacked
				for(Message message : un_acked_messages[i]) {
					mc.c[mc.num] = message;
					mc.num++;
					if(mc.num >= MAX_CONTAINER_NUM) {
						send_sock_m.lock();
							if(!(s = sendto(send_sock_all, (const char *)&mc, MAX_CONTAINER_NUM*sizeof(Message) + sizeof(int), MSG_WAITALL, (const struct sockaddr *) &addr_receiver, addr_receiver_size)))
							{
								printf("Sending message through the socket was not successful\n");
							}
						send_sock_m.unlock();
						mc.num = 0;
					}
				}
				if(mc.num != 0) {
//					usleep(1000);
					send_sock_m.lock();
						if(!(s = sendto(send_sock_all, (const char *)&mc, MAX_CONTAINER_NUM*sizeof(Message) + sizeof(int), MSG_WAITALL, (const struct sockaddr *) &addr_receiver, addr_receiver_size)))
						{
							printf("Sending message through the socket was not successful\n");
						}
					send_sock_m.unlock();
				}

			}
		}
	}
}
