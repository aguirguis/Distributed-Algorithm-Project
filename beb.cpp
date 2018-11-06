#include <string>
#include <iostream>
#include <functional> // std::mem_fn
#include "beb.h"

//If some callback is passed, it will be called on receive..otherwise, beb callback will be called
void beb::init(deliver_callback* bclass){
	pl = new perfect_link();

	 //start receiving thread
	 this->bclass = bclass;
	 if(bclass == NULL){
		 bclass = this;
		 this->bclass = this;
		 cout << "Use bebBroadcast to deliver" << endl;
	 }

	 recv = std::thread(&perfect_link::deliver, pl, std::move(bclass));
	 recv_ack = std::thread(&perfect_link::recv_ack, pl);
	 resend = std::thread(&perfect_link::resend, pl);
	 send = std::thread(&perfect_link::send_all, pl);
}

void beb::bebBroadcast(Message message) {
	 message.sender = my_process_id;
	 //send this message to all processes
	 for(int i = 0; i < nb_of_processes; i++) {
	 	if(processes[i].id != my_process_id) {
			pl->messages_all[processes[i].id - 1].push(std::move(message));
	 	}
	 	else {
			//deliver it to myself
		 	bclass->deliver(message);
		}
	 }
}

void beb::beb_deliver(Message message) {
//	log_m.lock();
	LogMessage lm;
	lm.message_type='d';
	lm.sender = message.initial_sender;
	lm.seq_nr = message.seq_no;
	messages_log[log_pointer] = lm;
	log_pointer++;
	if(log_pointer == MAX_LOG_FILE)
		write_log();
//	log_m.unlock();
}

void beb::deliver(Message message) {
	beb_deliver(message);
}
