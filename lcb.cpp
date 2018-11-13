#include "lcb.h"

void lcb::init() {
    urb_instance.init(this);
}

void lcb::lcb_broadcast(Message message) {

}

void lcb::lcb_deliver(Message message) {

}

void lcb::deliver(Message message) {
    lcb_deliver(message);
}
