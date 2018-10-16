#include <string>
#include <fstream>
#include "structures.h"
using namespace std;

int nb_of_processes;
Process processes[MAX_PROCESSES_NUM + 1];

int my_process_id;
string my_ip;
int my_port;
int send_sock, recv_sock;
LogMessage messages_log[MAX_LOG_FILE];
int log_pointer = 0;
ofstream out_file;
