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

void urb::init(){
	bbb.init(this);
}

void urb::urbBroadcast(Message message) {
    // add itself as the immediate sender
    // and put into the pending set
    message.sender = my_process_id;
    message.initial_sender = my_process_id;
    pending.insert(message);
    // since we did insert something in pending here,
    // we should probably check the condition
    // upon exists
    /*
    while(it != pending.end()) {
        if(candeliver(*it) && (delivered.find(*it) == delivered.end())) {
            delivered.insert(*it);
            urb_deliver(*it, (*it).sender, ack);
        }
        it++;
    }
    */
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
    it = pending.begin();
    while(it != pending.end()) {
        if((*it).initial_sender == message.initial_sender
           && (*it).seq_no == message.seq_no) {
            notInPending = false;
            break;
        }
        else {
            it++;
        }
    }
    printf("============= Process %d Retransmit message %d %d ? %d\n", my_process_id, message.initial_sender, message.seq_no, notInPending);
    if(notInPending) {
        pending.insert(message);
        bbb.bebBroadcast(message);
    }
}

void urb::deliver(Message message) {

	printf("Process %d Before urb deliver at some process \n", my_process_id);
    urb_deliver(message, message.sender);
    printf("Process %d After urb at the same process \n", my_process_id);
    
    // upon exists
    it = pending.begin();
    while(it != pending.end()) {
        if(candeliver(*it)) {
            jt = delivered.begin();
            while(jt != delivered.end()) {
                if((*it).seq_no == (*jt).seq_no && (*it).initial_sender == (*jt).initial_sender) {
                    delivered.insert(*it);
                    bbb.beb_deliver(*it);
                }
            }
        }
        it++;
        /*
        if(candeliver(*it) && (delivered.find(*it) == delivered.end())) {
            delivered.insert(*it);
            bbb.beb_deliver(*it);

        }
        it++;
        */
    }
    printf("Process %d end of deliver at URB..\n", my_process_id);
}



bool urb::candeliver(Message message) {
    // calculate the number of acks for this message
    int nAcks = (ack[message.initial_sender][message.seq_no]).size();
    // return statement whether majority or not
    printf("????nAcks = %d \n", nAcks);
    return nAcks > nb_of_processes/2;
}




