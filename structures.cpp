#include <string>
#include <fstream>
#include "structures.h"
using namespace std;

int nb_of_processes;
Process processes[MAX_PROCESSES_NUM];

int my_process_id;
string my_ip;
int my_port;
int recv_sock;
int recvack_sock;
int* send_sock;
std::mutex log_m;
LogMessage* messages_log;
int log_pointer = 0;
ofstream out_file;
std::vector<Message> un_acked_messages[MAX_PROCESSES_NUM];
std::mutex un_acked_messages_m;
int send_sock_all;
std::vector<int> processes_dependencies[MAX_PROCESSES_NUM];
std::vector<int> my_dependencies;
