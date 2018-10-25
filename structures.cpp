#include <string>
#include <fstream>
#include "structures.h"
using namespace std;

int nb_of_processes;
Process processes[MAX_PROCESSES_NUM + 1];

int my_process_id;
string my_ip;
int my_port;
int recv_sock;
int* send_sock;
std::mutex log_m;
LogMessage* messages_log;
int log_pointer = 0;
ofstream out_file;
