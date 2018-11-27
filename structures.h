#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <vector>
#ifndef STRUCTURES_H_
#define STRUCTURES_H_

using namespace std;

#define MAX_PROCESSES_NUM 10
#define MAX_MESSAGE_NUM 10000
#define MSG_LEN 64
#define MAX_LOG_FILE 1000	//let's say after each 100 message, I will write to a file
#define MAX_CONTAINER_NUM 50

struct Message {
    int seq_no;
    int sender;				//last hop = immediate sender
    int initial_sender;		//the initial sender
    // TODO: try to find a way to assign the size on creating the message (note: int* does not work
    // since sending the message will send only the memory address without the content causing segmentation fault)
    int vector_clock[MAX_PROCESSES_NUM];
    bool operator <(const Message & m) const
	{
		return seq_no < m.seq_no;
	}
    bool operator ==(const Message & m) const
    {
        return ((seq_no == m.seq_no) && (initial_sender == m.initial_sender) && (sender == m.sender));// && (vector_clock == m.vector_clock));
    }
};

struct m_container{
	Message c[MAX_CONTAINER_NUM];
	int num;
};

struct ack_message {
    int acking_process;				//process to send the ack
    Message message;
	bool operator ==(const ack_message & m) const
	{
		return ((acking_process == m.acking_process) && (message == m.message));
	}
};

struct ack_container{
	ack_message a[MAX_CONTAINER_NUM];
	int num;
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

extern int nb_of_processes;
extern Process processes[MAX_PROCESSES_NUM];

extern std::vector<Message> un_acked_messages[MAX_PROCESSES_NUM];
extern std::mutex un_acked_messages_m;
extern int my_process_id;
extern string my_ip;
extern int my_port;
extern int recv_sock;
extern int recvack_sock;
extern int* send_sock;
extern int send_sock_all; // TODO: added
extern std::mutex log_m;
extern LogMessage* messages_log;
extern int log_pointer;
extern ofstream out_file;
extern std::vector<int> processes_dependencies[MAX_PROCESSES_NUM];
extern std::vector<int> my_dependencies;

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


// This is for the localized causal broadcast


#endif /* STRUCTURES_H_ */
