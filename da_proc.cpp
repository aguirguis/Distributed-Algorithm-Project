/*
 * da_proc.cpp
 *
 *  Created on: 2 Oct 2018
 *      Author: aguirguis
 */

/*
 * da_proc.cpp
 *
 *  Created on: 30 Sep 2018
 *      Author: aguirguis
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX_PROCESSES_NUM 10;
#define MSG_LEN 32;

using namespace std;
static int wait_for_start = 1;
struct Process{
	int id;
	string ip;
	int port;
};

int my_id;
string my_ip;
int my_port;

//Process neighbors[MAX_PROCESSES_NUM];

static void start(int signum) {
	wait_for_start = 0;
}

static void stop(int signum) {
	//reset signal handlers to default
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);

	//immediately stop network packet processing
	printf("Immediately stopping network packet processing.\n");

	//write/flush output file if necessary
	printf("Writing output.\n");

	//exit directly from signal handler
	exit(0);
}

int main(int argc, char** argv) {

	//set signal handlers
	signal(SIGUSR1, start);
	signal(SIGTERM, stop);
	signal(SIGINT, stop);

	//Just for testing.......
	my_ip="127.0.0.1";
	my_port=atoi(argv[1]);
	int client_port = atoi(argv[2]);
	bool server = atoi(argv[3])==0?true:false;


	//parse arguments, including membership
	//initialize application
	//start listening for incoming UDP packets
	printf("Initializing.\n");
	//init server (listener socket)
	 int sock_server = socket(AF_INET,SOCK_DGRAM,0);
	 struct sockaddr_in serv,client;
	 memset(&serv, 0, sizeof(serv));
	 memset(&client, 0, sizeof(client));

	 socklen_t l = sizeof(client);
	 socklen_t m = sizeof(serv);

	 serv.sin_family = AF_INET;
	 serv.sin_port = htons(my_port);
	 serv.sin_addr.s_addr = inet_addr(my_ip.c_str());

	 if(server){
		 int b = bind(sock_server, (const struct sockaddr *)&serv, m);
		 printf("result of bind: %d \n", b);
	 }

	//init client (receiving socket)
	 int sock_client = socket(AF_INET,SOCK_DGRAM,0);

	 client.sin_family = AF_INET;
	 client.sin_port = htons(my_port + 10);	//send from any port, does not matter
	 client.sin_addr.s_addr = inet_addr(my_ip.c_str());

	 if(!server){
		 int b2 = bind(sock_client, (const struct sockaddr *)&client, l);
		 printf("reult of bind is %d \n", b2);
	 }

	 printf("before send or receive\n");
	 if(server){
		 printf("server....\n");
		 struct sockaddr_in remote;
		 remote.sin_family = AF_INET;
		 remote.sin_port = htons(client_port);	//send from any port, does not matter
		 remote.sin_addr.s_addr = inet_addr(my_ip.c_str());
		 int seq_no=0;
		 int s = sendto(sock_server, (const char *)&seq_no, sizeof(int),
		        0, (const struct sockaddr *) &remote,
		            l);
		 printf("sent something, result is %d \n", s);
	 }else{
		 printf("my port is %d and I am receiving a message on %d \n", my_port+10, client_port);
		 struct sockaddr_in remote;
		 remote.sin_family = AF_INET;
		 remote.sin_port = htons(client_port);	//send from any port, does not matter
		 remote.sin_addr.s_addr = inet_addr(my_ip.c_str());

		 socklen_t len;
		 char buffer[sizeof(int)];
		 int r = recvfrom(sock_client, (char *)buffer, sizeof(int),
		                 0, ( struct sockaddr *) &remote,
		                 &len);
		 printf("received something, result is %d \n", r);
	 }
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


