#include "frb.h"

void frb::init(deliver_callback* callback) {
    lsn = 0;
    pending = new std::list<Message>[nb_of_processes];
    next = new int[nb_of_processes];
    std::fill_n(next, nb_of_processes, 1);

    this -> callback = callback;
    beb_instance = new beb();
    beb_instance -> init(this);
}

void frb::frb_broadcast(Message message) {
    message.seq_no = ++lsn;
    message.sender = my_process_id;
    message.initial_sender = my_process_id;

    cout << my_process_id << " FRBBroadcast from this process" << endl;
    // broadcast the message using urb (TODO: change beb_instance to urb_instance)
    beb_instance -> bebBroadcast(message);
}

void frb::frb_deliver(Message message) {
    int from = message.initial_sender;
    int from_index = from - 1;
    cout << my_process_id << " FRB deliver: received " << message.seq_no << " from " << from << endl;
    pending[from_index].push_back(message);

    std::list<Message>::iterator message_iterator = pending[from_index].begin();
    while(message_iterator != pending[from_index].end()) {
        if(message_iterator -> seq_no <= next[from_index]) {
            if(message_iterator -> seq_no == next[from_index]) next[from_index]++;

            // deliver the messages
            beb_instance -> beb_deliver(*message_iterator); //(TODO: change beb_instance to urb_instance)
            // since FRB is in contact with the application layer then it should also execute the callback received from the application layer
            if (callback != NULL) callback -> deliver(*message_iterator);

            // erase the message
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
