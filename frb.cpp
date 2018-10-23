#include "frb.h"

void frb::init(deliver_callback* callback) {
    lsn = 0;
    pending = new std::list<Message>[nb_of_processes];
    next = new int[nb_of_processes];
    std::fill_n(next, MAX_PROCESSES_NUM, 1);

    this -> callback = callback;
    beb_instance = new beb();
    beb_instance -> init(this);
}

void frb::frb_broadcast(Message message) {
    message.seq_no = ++lsn;
    message.sender = my_process_id;
    message.initial_sender = my_process_id;

    // broadcast the message using urb (TODO: change beb_instance to urb_instance)
    beb_instance -> bebBroadcast(message);
}

void frb::frb_deliver(Message message) {
    int from = message.initial_sender;
    pending[from].push_back(message);

    std::list<Message>::iterator message_iterator;
    for(message_iterator = pending[from].begin(); message_iterator != pending[from].end(); ++message_iterator) {
        if (message_iterator -> seq_no <= next[from]) {
            if(message_iterator -> seq_no == next[from]) next[from]++;
            pending[from].erase(message_iterator--);
            beb_instance -> beb_deliver(*message_iterator); //(TODO: change beb_instance to urb_instance)
            // since FRB is in contact with the application layer then it should also execute the callback received from the application layer
            if (callback != NULL) callback -> deliver(message);
        }
    }
}

void frb::deliver(Message message) {
    frb_deliver(message);
}
