#include <string>
#include <fstream>
#include <iostream>
#include <mutex>

#ifndef STRUCTURES_H_
#define STRUCTURES_H_

using namespace std;

struct Message {
    int seq_no;
    int sender;				//last hop = immediate sender
    int initial_sender;		//the initial sender
	bool operator <(const Message & m) const
	{
		return seq_no < m.seq_no;
	}
};

struct ack_message {
    int seq_no;
    int acking_process;				//process to send the ack
    int initial_sender;		//the sender of this message
	bool operator ==(const ack_message & m) const
	{
		return ((seq_no == m.seq_no) && (initial_sender == m.initial_sender) && (acking_process == m.acking_process));
	}
};

struct AckMessageComp
{
	bool operator()(const ack_message& msg1, const ack_message& msg2) const
	{
		return true;
	}
};
// Message comparator / Functor
// Compare objects with sequence number then with initial_sender id
struct MessageComp
{
	bool operator()(const Message& msg1, const Message& msg2) const
	{
		return (msg1.seq_no != msg2.seq_no) || (msg1.initial_sender != msg2.initial_sender);
	}
};

struct comp_m
{
	bool operator()(const Message& msg1, const Message& msg2) const
	{
		return (msg1.seq_no < msg2.seq_no);
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

// Process comparator / Functor
// Compare objects with id number.
// Should I compare wrt to something else?
struct ProcessComp
{
    bool operator()(const Process& prcs1, const Process& prcs2) const
    {
        return (prcs1.id < prcs2.id) || ((!(prcs2.id < prcs1.id)));
    }
};

#define MAX_PROCESSES_NUM 10
#define MAX_MESSAGE_NUM 10000
#define MSG_LEN 64
#define MAX_LOG_FILE 1000	//let's say after each 100 message, I will write to a file

extern int nb_of_processes;
extern Process processes[MAX_PROCESSES_NUM + 1];

extern int my_process_id;
extern string my_ip;
extern int my_port;
extern int recv_sock;
extern int recvack_sock;
extern int* send_sock;
extern std::mutex log_m;
extern LogMessage* messages_log;
extern int log_pointer;
extern ofstream out_file;

//This function writes the logs received to the log file
static void write_log(){
	cout << "At process " << my_process_id << " writing output to file... number of lines in log: " << log_pointer << endl;
	for(int i = 0; i < log_pointer; i++) {
		if(messages_log[i].message_type == 'b')
			out_file << "b " << messages_log[i].seq_nr << endl;
		else
			out_file << "d " << messages_log[i].sender << " " << messages_log[i].seq_nr << endl;
	}
	log_pointer = 0;	//return the point to the beginning
}

class deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")
    public:
        virtual void deliver(Message) = 0;
};

#endif /* STRUCTURES_H_ */
