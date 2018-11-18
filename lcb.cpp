#include "lcb.h"

void lcb::init() {
    lsn = 0;
    pending = new std::list<Message>[1];
    urb_instance.init(this);
}

void lcb::lcb_broadcast(Message message) {
    // update message details
    std::copy(std::begin(vector_clock), std::end(vector_clock), std::begin(message.vector_clock));
    message.seq_no = ++vector_clock[my_process_id - 1]; // TODO: check if this is correct
    message.sender = my_process_id;
    message.initial_sender = my_process_id;

    // save to log
    LogMessage lm;
    lm.message_type='b';
    lm.seq_nr = message.seq_no;
    lm.sender = my_process_id;
    messages_log[log_pointer] = lm;
    log_pointer++;
    if(log_pointer == MAX_LOG_FILE) write_log();

    // broadcast the message using urb
    urb_instance.urbBroadcast(message);
}

void lcb::lcb_deliver(Message message) {
    pending[0].push_front(message);
    pending[0].sort(MessageLCBComp());

    std::list<Message>::iterator message_iterator = pending[0].begin();
    while(message_iterator != pending[0].end()) {
        // if W' <= V
        bool isLessThan = compare_vector_clocks(message_iterator -> vector_clock, vector_clock);
        if(isLessThan) {
            // update my vector_clock
            vector_clock[(message_iterator -> initial_sender) - 1]++;

            // deliver the message
            urb_instance.bbb.deliver(*message_iterator);

            // erase the message
            message_iterator = pending[0].erase(message_iterator);
        }
        else {
            ++message_iterator;
        }
    }
}

void lcb::deliver(Message message) {
    lcb_deliver(message);
}
