#include "urb.hpp"
#include <list>
#include <algorithm>
#include <mutex>
#ifndef FRB_H_
#define FRB_H_

class frb : public deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")

    public:
		int lsn;
        std::list<Message>* pending;
        std::mutex pen_m;
		int* next;
		std::mutex next_m;
        urb urb_instance;

		void init();
        void frb_broadcast(Message message);
        void frb_deliver(Message message);
        void deliver(Message message);
};


#endif /* FRB_H_ */
