/*
 * perfect_link.h
 *
 *  Created on: 15 Oct 2018
 *      Author: aguirguis
 */

#include <set>
#include <string.h>
#include <queue>
#include <mutex>
#include <algorithm>
#include "assert.h"
#include "structures.h"
#ifndef PERFECT_LINK_H_
#define PERFECT_LINK_H_


class perfect_link {

    private:
        set<Message, MessageComp> delivered;
        std::mutex del_m;

    public:
        std::queue<Message> messages;
        std::vector<ack_message> acks;
        void send(int to, perfect_link* recv_acks);
        void deliver(deliver_callback *bclass);
        void recv_ack();
};



#endif /* PERFECT_LINK_H_ */
