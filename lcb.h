#include "urb.hpp"
#include <list>
#include <algorithm>
#include <mutex>
#ifndef LCB_H_
#define LCB_H_

class lcb : public deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")

    public:
        int* vector_clock;
        std::mutex vec_m;
        std::list<Message>* pending;
        std::mutex pen_m;
        urb urb_instance;

		void init();
        void lcb_broadcast(Message message);
        void lcb_deliver(Message message);
        void deliver(Message message);
};


#endif /* LCB_H_ */
