#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string>
#include <cerrno>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>         // std::thread

#define MAX_PROCESSES_NUM 10
#define MSG_LEN 32
#define MAX_LOG_FILE 100	//let's say after each 100 message, I will write to a file

using namespace std;
static int wait_for_start = 1;
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

//Process neighbors[MAX_PROCESSES_NUM];

static void start(int signum) {
	wait_for_start = 0;
}

//This function writes the logs received to the log file...it takes proc_id as an input
static void write_log(){
	ofstream out_file;
	out_file.open("da_proc_" + to_string(process_id) + ".out");
	for(int i = 0; i < log_pointer; i++) {
		//TODO: write processes[proc_id].log[i] to file
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
static void stop(int signum) {
	//reset signal handlers to default
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);

	//immediately stop network packet processing
	printf("Immediately stopping network packet processing.\n");

	//write/flush output file if necessary
	printf("Writing output.\n");
	write_log();
	// for(Process p: processes)
	// 	if(p.id != process_id)
	// 		write_log(p.id);

	//exit directly from signal handler
	exit(0);
}

static void sendP(string ip, int port, int* messages, int m_len){
	 printf("server....\n");
	 struct sockaddr_in remote;
	 remote.sin_family = AF_INET;
	 remote.sin_port = htons(port);	//send from any port, does not matter
	 remote.sin_addr.s_addr = inet_addr(ip.c_str());
	 socklen_t l = sizeof(remote);
	 for(int i=0;i<m_len;i++){
		 int seq_no = messages[i];
		 bool send_again = true;
		 int ack = -1;
		 while(send_again){
			 int s = sendto(sock_client, (const char *)&seq_no, sizeof(int),
					0, (const struct sockaddr *) &remote,
						l);
			 printf("sent something? %d\n", s);
			 log[log_pointer].message_type = 'b';
			 log[log_pointer].seq_nr = seq_no;
			 log_pointer++;

			 //Here, I should initiate a timeout to wait for a packet....if timeout fires, send the packet again
			 fd_set set;
			 struct timeval timeout;
			 FD_ZERO(&set); /* clear the set */
			 FD_SET(sock_client, &set); /* add our file descriptor to the set */
			 timeout.tv_sec = 1; // SOCKET_READ_TIMEOUT_SEC;
			 timeout.tv_usec = 0;
			 int rv = select(sock_client+1, &set, NULL, NULL, &timeout);
			 char* buf[3];
			 if (rv == 0){
				 printf("timeout, should send again!!\n");
				 send_again = true;
			 }else{
				 ack = recvfrom(sock_client, (char*) buf, 3, 0, ( struct sockaddr *) &remote, &l);
				 printf("Ack received? %d\n", ack);
				 if(ack > 0)
					 send_again = false;
			 }
		 }//end while
	}//end for
}

static int recvP(){
	 struct sockaddr_in remote;

	 socklen_t len = sizeof(remote);
	 char buffer[sizeof(int)];
	 int r = recvfrom(sock_server, (char *)buffer, sizeof(int),
	                 0, ( struct sockaddr *) &remote,
	                 &len);
	 //received the something? who was the clinet? get it
	 struct hostent *hostp;
	 hostp = gethostbyaddr(( const char *) &remote.sin_addr.s_addr, sizeof(remote.sin_addr.s_addr), AF_INET);
	 int recvID = -1;
	 for(Process p: processes)
		 if(p.ip.compare(hostp->h_name) == 0){
			 recvID = p.id;
			 break;
		 }
	 assert(recvID != -1);	//otherwise, there is a problem here!
	 int num = atoi(buffer);
	 log[log_pointer].message_type = 'd';
	 log[log_pointer].sender = recvID;
	 log[log_pointer].seq_nr = num;
	 log_pointer++;
	 if(log_pointer == MAX_LOG_FILE)
		 write_log();
	 printf("recv something? %d\n", r);
	 char * ackstr = "Ack";
	 int ack = 0;
	 ack = sendto(sock_server, (const char *)ackstr, strlen(ackstr), 0, (const struct sockaddr *) &remote, len);
	 printf("Ack sent? %d\n", ack);
	 return ack;
}

int main(int argc, char** argv) {

	//set signal handlers
	signal(SIGUSR1, start);
	signal(SIGTERM, stop);
	signal(SIGINT, stop);

	//Just for testing.......
//	my_ip="127.0.0.1";
//	my_port=atoi(argv[1]);
//	int client_port = atoi(argv[2]);
//	bool server = atoi(argv[3])==0?true:false;


	//parse arguments, including membership
	//initialize application
	//start listening for incoming UDP packets
	printf("Initializing.\n");
	process_id = atoi(argv[1]);
	ifstream membership (argv[2]);
	if(membership.is_open()) {
		membership >> nb_of_processes;
		for(int i = 0; i < nb_of_processes; i++) {
			membership >> processes[i].id;
			membership >> processes[i].ip;
			membership >> processes[i].port;
		}
	}
	else {
		printf("Fail To Open File");
	}
	membership.close();

	my_ip = processes[process_id-1].ip;
	my_port = processes[process_id-1].port;

//	int client_port = processes[1].port;

	//init server (listener socket)
	 sock_server = socket(AF_INET,SOCK_DGRAM,0);
	 struct sockaddr_in serv,client;
	 memset(&serv, 0, sizeof(serv));
	 memset(&client, 0, sizeof(client));

	 socklen_t l = sizeof(client);
	 socklen_t m = sizeof(serv);

	 serv.sin_family = AF_INET;
	 serv.sin_port = htons(my_port);
	 serv.sin_addr.s_addr = inet_addr(my_ip.c_str());

//	 if(server){
	 int b = bind(sock_server, (const struct sockaddr *)&serv, m);
	 printf("result of bind: %d \n", b);
//	 }

	//init client (receiving socket)
	 sock_client = socket(AF_INET,SOCK_DGRAM,0);

	 client.sin_family = AF_INET;
	 client.sin_port = htons(my_port + 10);	//send from any port, does not matter
	 client.sin_addr.s_addr = inet_addr(my_ip.c_str());

//	 if(!server){
	 int b2 = bind(sock_client, (const struct sockaddr *)&client, l);
	 printf("result of bind is %d \n", b2);
//	 }

//	 if(server){
	std::thread recv(recvP);
//	 }else{
		 //send to all other processes
		 int m_len = 10;
		 int* messages = new int[m_len];
		 for(int i=0;i<m_len;i++)
			 messages[i] = i;
		 for(int i=0;i<nb_of_processes;i++)if(processes[i].id != process_id){
			 sendP(processes[i].ip, processes[i].port, messages,m_len);
		 }
//	 }
	 //wait until start signal


	while(wait_for_start) {
		struct timespec sleep_time;
		sleep_time.tv_sec = 0;
		sleep_time.tv_nsec = 1000;
		nanosleep(&sleep_time, NULL);
	}


	//broadcast messages
	printf("Broadcasting messages.\n");


	//wait until stopped
	while(1) {
		struct timespec sleep_time;
		sleep_time.tv_sec = 1;
		sleep_time.tv_nsec = 0;
		nanosleep(&sleep_time, NULL);
	}
}
