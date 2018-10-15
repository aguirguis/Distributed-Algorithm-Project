#include <string>

using namespace std;

struct Message {
    int seq_no;
    int sender;
};

// Message comparator / Functor
// Compare objects with sequence number then with sender id
struct MessageComp
{
	bool operator()(const Message& msg1, const Message& msg2) const
	{
		return (msg1.seq_no < msg2.seq_no) || ((!(msg2.seq_no < msg1.seq_no)) && (msg1.sender < msg2.sender));
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
int sockets[MAX_PROCESSES_NUM + 1];

int my_process_id;
string my_ip;
int my_port;
LogMessage messages_log[MAX_LOG_FILE];
int log_pointer = 0;

class deliver_callback {
    public:
        virtual void deliver(Message, int);
};
