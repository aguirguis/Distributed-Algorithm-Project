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
//#include "beb.h"

void urb::init(deliver_callback* callback){
	bbb.init(this);
	frb_callback = callback;
}

void urb::urbBroadcast(Message message) {
    // add itself as the immediate sender
    // and put into the pending set
    message.sender = my_process_id;
    pending.insert(message);
    // since we did insert something in pending here,
    // we should probably check the condition
    // upon exists
    // Broadcast
    bbb.bebBroadcast(message);
}

void urb::urb_deliver(Message message, int from) {
    // add process to acknowledgement array
    // here, we identify messages by their initial sender
    // this may not be entirely correct,
    // since the initial sender could send several messages.
    // however, we leave this for another level of abstraction
    ack[message.initial_sender][message.seq_no].insert(from);
    // check if pending
    // here again, it's not entirely clear
    // how to identify messages.
    // this is wrong?!
    bool notInPending = true;
    //TODO: concurrent execution with pending may create problems
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
    if(notInPending) {
        pending.insert(message);
        bbb.bebBroadcast(message);
    }
}

void urb::deliver(Message message) {

    urb_deliver(message, message.sender);

    // upon exists
    it = pending.begin();
    while(it != pending.end()) {
        if(candeliver(*it) && not_in_deliver(*it)) {
        	//TODO: concurrency with delivered may create problems
            delivered.insert(*it);
			if (frb_callback != NULL)
				frb_callback -> deliver(*it);
			else
	            bbb.beb_deliver(*it);
        }
        it++;
    }
}



bool urb::candeliver(Message message) {
    // calculate the number of acks for this message
    int nAcks = (ack[message.initial_sender][message.seq_no]).size();
    // return statement whether majority or not
    return nAcks > nb_of_processes/2;
}
bool urb::not_in_deliver(Message message){
	for(Message m: delivered){
		if(m.initial_sender == message.initial_sender && m.seq_no == message.seq_no)
			return false;
	}
	return true;
}
