#include "urb.hpp"
#include <list>
#include <algorithm>
#include <mutex>
#ifndef LCB_H_
#define LCB_H_

class lcb : public deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")

    public:
        int lsn;
        int vector_clock[MAX_PROCESSES_NUM];
        std::mutex vec_m;
        std::list<Message>* pending;
        std::mutex pen_m;
        urb urb_instance;

		void init();
        void lcb_broadcast(Message message);
        void lcb_deliver(Message message);
        void deliver(Message message);
};

// TODO: update this for the localized causal broadcast
static bool compare_vector_clocks(int v1[], int v2[]) {
    for(int i = 0; i < nb_of_processes; i++) {
        if(v1[i] > v2[i]) {
            return false;
        }
    }
    return true;
}

struct MessageLCBComp {
    // TODO: update this for the localized causal broadcast
    bool operator ()(const Message & m1, const Message & m2) {
        for(int i = 0; i < nb_of_processes; i++) {
            if(m1.vector_clock[i] > m2.vector_clock[i]) {
                return false;
            }
        }
        return true;
	}
};

#endif /* LCB_H_ */
