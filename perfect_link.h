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
        std::vector<Message> delivered;
        std::mutex del_m;
        std::mutex send_sock_m;

    public:
        std::queue<Message> messages_all[MAX_PROCESSES_NUM];
        void send_all();
        void send(int to);
        void deliver(deliver_callback *bclass);
        void recv_ack();
        void resend();
};



#endif /* PERFECT_LINK_H_ */
