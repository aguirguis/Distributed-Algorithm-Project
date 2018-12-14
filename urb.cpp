//
//  urb.cpp
//  project1
//
//  Created by Henrik Åslund on 2018-10-14.
//  Copyright © 2018 Henrik Åslund. All rights reserved.
//

#include "structures.h"
#include "beb.h"
#include "urb.hpp"

void urb::init(deliver_callback* callback){
	bbb.init(this);
	frb_callback = callback;
}

void urb::urbBroadcast(Message message) {
    // add itself as the immediate sender and put into the pending set
    message.sender = my_process_id;
//    pen_m.lock();
    pending.push_back(message);
//    pen_m.unlock();

    bbb.bebBroadcast(message);
}

void urb::urb_deliver(Message message, int from) {
    // add process to acknowledgement array
    // here, we identify messages by their initial sender
//	ack_m.lock();
    ack[message.initial_sender][message.seq_no].insert(from);
//    ack_m.unlock();

    // check if pending
    bool notInPending = true;

//    it_m.lock();
    it = pending.begin();
    while(it != pending.end()) {
        if((((Message)*it).initial_sender == message.initial_sender) && (((Message)*it).seq_no == message.seq_no)) {
            notInPending = false;
            break;
        }
        else {
            it++;
        }
    }
//    it_m.unlock();
    if(notInPending) {
//    	pen_m.lock();
        pending.push_back(message);
//        pen_m.unlock();
        bbb.bebBroadcast(message);
    }
}

void urb::deliver(Message message) {
	if(message.seq_no == 0)
		return;
    urb_deliver(message, message.sender);

    // upon exists
//    it_m.lock();
    it = pending.begin();
    while(it != pending.end()) {
        if(candeliver(*it) && not_in_deliver(*it)) {
//        	del_m.lock();
            delivered.push_back(*it);
//            printf("Process %d deliver %d %d \n", my_process_id, (*it).initial_sender, (*it).seq_no);
//            del_m.unlock();
			if (frb_callback != NULL){
				frb_callback -> deliver(*it);
			}
			else
	            bbb.beb_deliver(*it);
		pen_m.lock();
		it = pending.erase(it);
		pen_m.unlock();
        }
	else
	        it++;
    }
//    it_m.unlock();
}



bool urb::candeliver(Message message) {
    // calculate the number of acks for this message
//	ack_m.lock();
    int nAcks = (ack[message.initial_sender][message.seq_no]).size();
//    ack_m.unlock();
    // return statement whether majority or not
    return nAcks > (nb_of_processes/2);
}
bool urb::not_in_deliver(Message message){
	for(Message m: delivered){
		if(m.initial_sender == message.initial_sender && m.seq_no == message.seq_no)
			return false;
	}
	return true;
}
