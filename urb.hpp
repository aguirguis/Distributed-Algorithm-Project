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
#include "beb.h"
#include "structures.h"

using namespace std;

class urb: public deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")
private:
    set<Message, MessageComp> delivered;
    set<Message, MessageComp> pending;
    set<Message, MessageComp>::iterator it = pending.begin();
    int nMessages = 0;
    Message messageInd[0];
    set<int, greater <int> > ack[0];
    //Process self;
    
public:
    beb bbb;
    bool candeliver(Message message);
    void init();
    void urbBroadcast(Message message);
    void deliver(Message message);
    void urb_deliver(Message message, int from, set<int, greater <int> > ack[]);
};

#endif /* urb_hpp */
