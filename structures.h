#include <string>
#include <fstream>

using namespace std;

struct Message {
    int seq_no;
    int sender;				//last hop = immediate sender
    int initial_sender;		//the initial sender
};

// Message comparator / Functor
// Compare objects with sequence number then with sender id
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

int nb_of_processes;
Process processes[MAX_PROCESSES_NUM + 1];
// int sockets[MAX_PROCESSES_NUM + 1];

int my_process_id;
string my_ip;
int my_port;
int send_sock, recv_sock;
LogMessage messages_log[MAX_LOG_FILE];
int log_pointer = 0;
ofstream out_file;

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

//(TODO: there is no need for the second argument as it is already in the message)
class deliver_callback { // @suppress("Class has a virtual method and non-virtual destructor")
    public:
        virtual void deliver(Message, int);
};
