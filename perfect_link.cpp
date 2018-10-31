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
		usleep(1000);
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
	// while(true){
		m_container mc;
		if (!this->messages_all[to - 1].empty()){
			mc.num=0;
			while((!this->messages_all[to - 1].empty()) && (mc.num<10)){
				Message message = this->messages_all[to - 1].front();
				message.sender = my_process_id;
				mc.c[mc.num] = message;
				mc.num++;
				this->messages_all[to - 1].pop();
			}
		}else{
			// usleep(1000);
			// continue;
			return;
		}
		struct sockaddr_in addr_receiver;
		addr_receiver.sin_family = AF_INET;
		addr_receiver.sin_port = htons(processes[to - 1].port);
		addr_receiver.sin_addr.s_addr = inet_addr(processes[to - 1].ip.c_str());
		socklen_t addr_receiver_size = sizeof(addr_receiver);
		int s;
		if(!(s = sendto(send_sock_all, (const char *)&mc, mc.num*sizeof(Message) + sizeof(int), MSG_WAITALL, (const struct sockaddr *) &addr_receiver, addr_receiver_size)))
		{
			printf("Sending message through the socket was not successful\n");
		}
		for(Message message: mc.c) {
			un_acked_messages_m.lock();
				un_acked_messages[to - 1].push_back(message);
			un_acked_messages_m.unlock();
		}
	// }//end while true
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
		char buf[1000];
		m_container mc;
		int r = recvfrom(recv_sock, &buf, 1000, MSG_WAITALL, ( struct sockaddr *) &addr_sender, &addr_sender_size);
		memcpy(&mc, buf, r);
		// check if message is already delivered
		for(int i=0;i<mc.num;i++){
			Message message = mc.c[i];
			del_m.lock();
			bool is_delivered = std::find(delivered.begin(), delivered.end(), message) != delivered.end();
			del_m.unlock();
			if(!is_delivered) {
				// deliver the received message
				assert (bclass != NULL);
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
			ack_m.message = message;
			addr_sender.sin_port = htons(processes[message.sender - 1].port + 800);
			int a = sendto(send_sock_ack, (const char *)&ack_m, 16, MSG_WAITALL, (const struct sockaddr *) &addr_sender, addr_sender_size);
			assert(a>0);
			usleep(1000);
		}//end while true
	}
}

// This method works in one separate threads to receive acks
void perfect_link::recv_ack(){
	while(true){
		char buf[16];
		struct sockaddr_in addr_receiver;
		socklen_t addr_receiver_size = sizeof(addr_receiver);
		recvfrom(recvack_sock, (char*) buf, 16, MSG_WAITALL, (struct sockaddr *) &addr_receiver, &addr_receiver_size);
		ack_message ack;
		memcpy(&ack,buf,16);

		int to = ack.acking_process;
		un_acked_messages_m.lock();
			std::vector<Message> msg_vector = un_acked_messages[to - 1];
			msg_vector.erase(std::remove(msg_vector.begin(), msg_vector.end(), ack.message), msg_vector.end());
		un_acked_messages_m.unlock();
	}
}


// This method works in one separate threads to resend unacked messages
void perfect_link::resend() {
	while(true) {
		sleep(1);
		for(int i = 0; i < nb_of_processes; i++) {
			if(!un_acked_messages[i].empty()) {

				int to = processes[i].id;
				struct sockaddr_in addr_receiver;
				addr_receiver.sin_family = AF_INET;
				addr_receiver.sin_port = htons(processes[to - 1].port);
				addr_receiver.sin_addr.s_addr = inet_addr(processes[to - 1].ip.c_str());
				socklen_t addr_receiver_size = sizeof(addr_receiver);
				int s;

				m_container mc;
				mc.num = 0;
				// resend all messages that are remaind unacked
				for(Message message : un_acked_messages[i]) {
					mc.c[mc.num] = message;
					mc.num++;
					if(mc.num >= 10) {
						if(!(s = sendto(send_sock_all, (const char *)&mc, mc.num*sizeof(Message) + sizeof(int), MSG_WAITALL, (const struct sockaddr *) &addr_receiver, addr_receiver_size)))
						{
							printf("Sending message through the socket was not successful\n");
						}
						mc.num = 0;
					}
				}
				if(mc.num != 0) {
					if(!(s = sendto(send_sock_all, (const char *)&mc, mc.num*sizeof(Message) + sizeof(int), MSG_WAITALL, (const struct sockaddr *) &addr_receiver, addr_receiver_size)))
					{
						printf("Sending message through the socket was not successful\n");
					}
				}

			}
		}
	}
}
