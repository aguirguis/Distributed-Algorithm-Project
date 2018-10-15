#include <string>

using namespace std;

#define MAX_PROCESSES_NUM 10
#define MSG_LEN 64
#define MAX_LOG_FILE 100	//let's say after each 100 message, I will write to a file

struct Message{
  int sender;
  int seq_no;
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

int nb_of_processes;
Process processes[MAX_PROCESSES_NUM];

int process_id;
string my_ip;
int my_port;
int sock_client, sock_server;
LogMessage log[MAX_LOG_FILE];
int log_pointer = 0;
