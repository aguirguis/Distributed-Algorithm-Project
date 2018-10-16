#include <string>
#include <fstream>

#ifndef STRUCTURES_H_
#define STRUCTURES_H_

using namespace std;

struct Message {
    int seq_no;
    int sender;				//last hop = immediate sender
    int initial_sender;		//the initial sender
};

// Message comparator / Functor
// Compare objects with sequence number then with initial_sender id
struct MessageComp
{
	bool operator()(const Message& msg1, const Message& msg2) const
	{
		return (msg1.seq_no < msg2.seq_no) || ((!(msg2.seq_no < msg1.seq_no)) && (msg1.initial_sender < msg2.initial_sender));
	}
};

struct LogMessage {
	char message_type;
	int seq_nr;
	int sender;
};

struct Process {
	int id;
	string ip;
	int port;
};

#define MAX_PROCESSES_NUM 10
#define MSG_LEN 64
#define MAX_LOG_FILE 100	//let's say after each 100 message, I will write to a file

extern int nb_of_processes;
extern Process processes[MAX_PROCESSES_NUM + 1];

extern int my_process_id;
extern string my_ip;
extern int my_port;
extern int send_sock, recv_sock;
extern LogMessage messages_log[MAX_LOG_FILE];
extern int log_pointer;
extern ofstream out_file;

//This function writes the logs received to the log file
static void write_log(){
	for(int i = 0; i < log_pointer; i++) {
		if(messages_log[i].message_type == 'b')
			out_file << "b " << messages_log[i].seq_nr << "\n";
		else
			out_file << "d " << messages_log[i].sender << " " << messages_log[i].seq_nr << "\n";
	}
	log_pointer = 0;	//return the point to the beginning
}

class deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")
    public:
        void deliver(Message) {}
};

#endif /* STRUCTURES_H_ */
