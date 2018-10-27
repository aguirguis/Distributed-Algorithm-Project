#include "frb.h"

void frb::init(deliver_callback* callback) {
    lsn = 0;
    pending = new std::list<Message>[nb_of_processes];
    next = new int[nb_of_processes];
    std::fill_n(next, nb_of_processes, 1);

//    this -> callback = callback;
    urb_instance.init(this);
}

void frb::frb_broadcast(Message message) {
    message.seq_no = ++lsn;
    message.sender = my_process_id;
    message.initial_sender = my_process_id;

//    cout << my_process_id << " FRBBroadcast from this process" << endl;
    // broadcast the message using urb 
    urb_instance.urbBroadcast(message);
}

void frb::frb_deliver(Message message) {
    int from = message.initial_sender;
    int from_index = from - 1;
//    cout << my_process_id << " FRB deliver: received " << message.seq_no << " from " << from << " send throught " << message.sender << endl;
    pending[from_index].push_back(message);

    std::list<Message>::iterator message_iterator = pending[from_index].begin();
    while(message_iterator != pending[from_index].end()) {
        if(message_iterator -> seq_no <= next[from_index]) {
        	//TODO: This also may have problems with concurrency
            if(message_iterator -> seq_no == next[from_index]) next[from_index]++;

            // since FRB is in contact with the application layer then it should also execute the callback received from the application layer
 //           if (callback != NULL) callback -> deliver(*message_iterator);
            urb_instance.bbb.deliver(*message_iterator);

            // erase the message
            //TODO: This is suspicious....problems maybe here
            message_iterator = pending[from_index].erase(message_iterator);
        }
        else {
            ++message_iterator;
        }
    }
}

void frb::deliver(Message message) {
    frb_deliver(message);
}
