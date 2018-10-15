/*
 * perfect_link.h
 *
 *  Created on: 15 Oct 2018
 *      Author: aguirguis
 */

#include <set>
#include "structures.h"
#ifndef PERFECT_LINK_H_
#define PERFECT_LINK_H_


class perfect_link {

    private:
        set<Message, MessageComp> delivered;

    public:
        void send(Message message, int to);
        void deliver(void* callback);
};



#endif /* PERFECT_LINK_H_ */
