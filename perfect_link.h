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

#define PACKET_SIZE sizeof(m_container)

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

static void serializeMessage(Message* message, char *data) {
    int *q = (int*)data;
    *q = message->seq_no; q++;
    *q = message->sender; q++;
    *q = message->initial_sender; q++;

    int i = 0;
    while (i < MAX_PROCESSES_NUM) {
        *q = message->vector_clock[i]; q++;
        i++;
    }
}

static void deserializeMessage(char *data, Message* message) {
    int *q = (int*)data;
    message->seq_no = *q; q++;
    message->sender = *q; q++;
    message->initial_sender = *q; q++;

    int i = 0;
    while (i < MAX_PROCESSES_NUM) {
        message->vector_clock[i] = *q; q++;
        i++;
    }
}

static void serialize(m_container* container, char *data) {
    int i = 0;
    while (i < MAX_CONTAINER_NUM) {
		serializeMessage(&(container->c[i]), data);
        i++;
    }
	int *q = (int*)data;
    *q = container->num; q++;
}

static void deserialize(char *data, m_container* container) {
    int i = 0;
    while (i < MAX_CONTAINER_NUM) {
		deserializeMessage(data, &(container->c[i]));
        i++;
    }
	int *q = (int*)data;
    container->num = *q; q++;
}

#endif /* PERFECT_LINK_H_ */
