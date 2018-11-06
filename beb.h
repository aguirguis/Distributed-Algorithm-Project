/*
 * beb.h
 *
 *  Created on: 15 Oct 2018
 *      Author: aguirguis
 */

#include "perfect_link.h"
#include <thread>
#include <vector>

#ifndef BEB_H_
#define BEB_H_

class beb: public deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")

    public:
		perfect_link* pl;
		std::thread recv;
		std::thread recv_ack;
        std::thread resend;
        std::thread send;
		deliver_callback* bclass;

		void init(deliver_callback* bclass);
		//If some callback is passed, it will be called on receive..otherwise, beb callback will be called
        void bebBroadcast(Message message);
        void beb_deliver(Message message);
        void deliver(Message message);
};




#endif /* BEB_H_ */
