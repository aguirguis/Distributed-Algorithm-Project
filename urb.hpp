//
//  urb.hpp
//  project1
//
//  Created by Henrik Åslund on 2018-10-14.
//  Copyright © 2018 Henrik Åslund. All rights reserved.
//

#ifndef urb_hpp
#define urb_hpp

#include <stdio.h>
#include <iostream>
#include <set>
#include <iterator>
#include <mutex>
#include "beb.h"
#include "structures.h"

using namespace std;

class urb: public deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")
private:
    set<Message, MessageComp> delivered;
    std::mutex del_m;
    set<Message, MessageComp> pending;
    std::mutex pen_m;
    set<Message, MessageComp>::iterator it;
    std::mutex it_m;
    std::mutex ack_m;
    set<int, greater <int> > ack[MAX_PROCESSES_NUM][MAX_MESSAGE_NUM];
    deliver_callback* frb_callback;
    //Process self;

public:
    beb bbb;
    bool candeliver(Message message);
    bool not_in_deliver(Message message);
    void init(deliver_callback* callback);
    void urbBroadcast(Message message);
    void deliver(Message message);
    void urb_deliver(Message message, int from);
};

#endif /* urb_hpp */
