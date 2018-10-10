#include <string>

using namespace std;

struct Message{
  int sender;
  int seq_no;
}

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
