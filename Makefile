all: da_proc

da_proc: da_proc.cpp
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"beb.d" -MT"beb.o" -o "beb.o" "beb.cpp"
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"urb.d" -MT"urb.o" -o "urb.o" "urb.cpp"
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"da_proc.d" -MT"da_proc.o" -o "da_proc.o" "da_proc.cpp"
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"perfect_link.d" -MT"perfect_link.o" -o "perfect_link.o" "perfect_link.cpp"
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"structures.d" -MT"structures.o" -o "structures.o" "structures.cpp"
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"frb.d" -MT"frb.o" -o "frb.o" "frb.cpp"
	g++ -pthread -o "da_proc"  ./beb.o ./urb.o ./frb.o ./da_proc.o ./perfect_link.o ./structures.o

clean:
	rm da_proc
