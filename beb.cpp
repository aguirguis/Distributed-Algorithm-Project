#include <string>
#include <iostream>
#include <functional> // std::mem_fn
#include "beb.h"

//If some callback is passed, it will be called on receive..otherwise, beb callback will be called
void beb::init(void ( *callback )()){
	pl = new perfect_link*[nb_of_processes];
	links = new std::thread[nb_of_processes];

	for(int i=0;i<nb_of_processes;i++)
		pl[i] = new perfect_link();
	recv_link = new perfect_link();

	 //start receiving thread
	 if(callback == NULL)
		 recv = std::thread(&perfect_link::deliver, &(*recv_link), &beb::beb_deliver);
	 else
		 recv = std::thread(&perfect_link::deliver, &(*recv_link), callback);
}

void beb::bebBroadcast(Message message) {
 message.sender = process_id;

 //send this message to all processes
 for(int i=0;i<nb_of_processes;i++)if(processes[i].id != process_id){
	links[i] = std::thread(&perfect_link::deliver, &(*pl[i]), message, processes[i].id);
 }

}

void beb::beb_deliver(Message message, int from) {
	//TODO: implement this...
	cout << "received " << message.seq_no << "from " << from << endl;
}

void beb::pl_deliver(Message message, int from) {
	beb_deliver(message, from);
}
