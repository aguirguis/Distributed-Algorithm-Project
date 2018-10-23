#include <string>
#include <iostream>
#include <functional> // std::mem_fn
#include "beb.h"

//If some callback is passed, it will be called on receive..otherwise, beb callback will be called
void beb::init(deliver_callback* bclass){
	pl = new perfect_link*[nb_of_processes];
//	links = new std::thread[nb_of_processes];
	this->bclass = bclass;

	for(int i=0;i<nb_of_processes;i++)
		pl[i] = new perfect_link();
	recv_link = new perfect_link();

	 //start receiving thread
	 if(bclass == NULL){
		 bclass = this;
		 cout << "Use bebBroadcast to deliver" << endl;
	 }

	 recv = std::thread(&perfect_link::deliver, recv_link, std::move(bclass));
}

void beb::bebBroadcast(Message message) {
 LogMessage lm;
 lm.message_type='b';
 lm.seq_nr = message.seq_no;
 lm.sender = my_process_id;
 messages_log[log_pointer] = lm;
 log_pointer++;
 if(log_pointer == MAX_LOG_FILE)
	 write_log();

 //send this message to all processes
 for(int i=0;i<nb_of_processes;i++)
	 if(processes[i].id != my_process_id){
		 links.push_back(std::thread(&perfect_link::send, pl[i], std::move(message), std::move(processes[i].id)));
	 }else	//deliver it to myself
		 bclass->deliver(message);
}

void beb::beb_deliver(Message message) {
	int from = message.initial_sender;
	cout << "BEB deliver: received " << message.seq_no << " from " << from << endl;
	LogMessage lm;
	lm.message_type='d';
	lm.sender = message.initial_sender;
	lm.seq_nr = message.seq_no;
	messages_log[log_pointer] = lm;
	log_pointer++;
	if(log_pointer == MAX_LOG_FILE)
		write_log();
}

void beb::deliver(Message message) {
	beb_deliver(message);
}
