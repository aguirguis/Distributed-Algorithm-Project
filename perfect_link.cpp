#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "perfect_link.h"
using namespace std;

/*
    this method send a message to the intended receiver through UDP
    @param message : the message to be sent (consist of the process id of the
                    sender and the sequence number of the message to be sent)
    @param to: the process id of the receiver
*/
void perfect_link::send(Message message, int to) {
    printf("sending through perfect link to %d\n", to);
    message.sender = my_process_id;

    // prepare the receiver socket address
    struct sockaddr_in addr_receiver;
	addr_receiver.sin_family = AF_INET;
	addr_receiver.sin_port = htons(processes[to].port);
	addr_receiver.sin_addr.s_addr = inet_addr(processes[to].ip.c_str());
	socklen_t addr_receiver_size = sizeof(addr_receiver);

    // send the message (keep re-sending till an ACK is received)
    bool send_again = true;
    int ack = -1;
    while(send_again){
        if(!sendto(send_sock, (const char *)&message, sizeof(message), 0, (const struct sockaddr *) &addr_receiver, addr_receiver_size))
        {
            printf("Sending message throught the socket was not successful\n");
        }

        //Here, I should initiate a timeout to wait for a packet....if timeout fires, send the packet again
        fd_set set;
        FD_ZERO(&set); /* clear the set */
        FD_SET(send_sock, &set); /* add our socket to the set */
        struct timeval timeout;
        timeout.tv_sec = 1; // SOCKET_READ_TIMEOUT_SEC;
        timeout.tv_usec = 0;
        int recVal = select(send_sock + 1, &set, NULL, NULL, &timeout);
        char* buf[3];
        if (recVal == 0)
        {
            printf("timeout, should send again!!\n");
            send_again = true;
        }
        else
        {
            ack = recvfrom(send_sock, (char*) buf, 3, 0, (struct sockaddr *) &addr_receiver, &addr_receiver_size);
            printf("Ack received? %d\n", ack);
            if(ack > 0)
                send_again = false;
        }
    }//end while
}

/*
    this method delivers the message received from UDP to the application
    @param callback: the callback function contains the action to be performed
                        when message is received
*/
void perfect_link::deliver(deliver_callback *bclass) {
    struct sockaddr_in addr_sender;
    socklen_t addr_sender_size = sizeof(addr_sender);
    struct Message message;
    int r = recvfrom(recv_sock, &message, sizeof(message), 0, ( struct sockaddr *) &addr_sender, &addr_sender_size);
    printf("recv something? %d\n", r);

    // check if message is already delivered
    const bool is_delivered = delivered.find(message) != delivered.end();

    if(!is_delivered) {
        // deliver the received message
        bclass -> deliver(message);

        // add to delivered
        delivered.insert(message);

        // send an ACK
        const char * ackstr = "Ack";
        int ack = 0;
        ack = sendto(send_sock, (const char *)ackstr, strlen(ackstr), 0, (const struct sockaddr *) &addr_sender, addr_sender_size);
        printf("Ack sent? %d\n", ack);
    }
}
