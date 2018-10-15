#include <string>

using namespace std;

#define MAX_PROCESSES_NUM 10
#define MSG_LEN 64
#define MAX_LOG_FILE 100	//let's say after each 100 message, I will write to a file

struct Message{
  int initial_sender;
  int direct_sender;
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
ofstream out_file;

//This function writes the logs received to the log file...it takes proc_id as an input
static void write_log(){
	for(int i = 0; i < log_pointer; i++) {
		//write processes[proc_id].log[i] to file
		if(log[i].message_type == 'b') {
			out_file << "b " << log[i].seq_nr << "\n";
		}
		else {
			out_file << "d " << log[i].sender << " " << log[i].seq_nr << "\n";
		}
	}
	out_file.close();
	log_pointer = 0;	//return the point to the beginning
}
