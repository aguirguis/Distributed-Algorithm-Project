/*
 * beb.h
 *
 *  Created on: 15 Oct 2018
 *      Author: aguirguis
 */

#include "perfect_link.h"
#include <thread>

#ifndef BEB_H_
#define BEB_H_

class beb: public deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")

    public:
		std::thread* links;
		perfect_link** pl;
		perfect_link* recv_link;
		std::thread recv;

		void init(deliver_callback* bclass);
		//If some callback is passed, it will be called on receive..otherwise, beb callback will be called
        void bebBroadcast(Message message);
        void beb_deliver(Message message);
        void deliver(Message message);
};




#endif /* BEB_H_ */
