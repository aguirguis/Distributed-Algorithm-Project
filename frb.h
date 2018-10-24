#include "urb.hpp" 
#include <list>

#ifndef FRB_H_
#define FRB_H_

class frb : public deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")

    public:
		int lsn;
        std::list<Message>* pending;
		int* next;
        urb urb_instance;
        deliver_callback* callback;

		void init(deliver_callback* callback);
        void frb_broadcast(Message message);
        void frb_deliver(Message message);
        void deliver(Message message);
};


#endif /* FRB_H_ */
