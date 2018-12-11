#!/bin/bash
#
# Tests the correctness of the Uniform Reliable Broadcast application.
#
# This is an example script that shows the general structure of the
# test. The details and parameters of the actual test might differ.
#

#time to wait for correct processes to broadcast all messages (in seconds)
#(should be adapted to the number of messages to send)

time_to_finish=150	#150
num_msgs=1000		#1000
init_time=2

#configure lossy network simulation
sudo tc qdisc change dev lo root netem delay 50ms 200ms loss 10% 25% reorder 25% 50%

# compile (should output: da_proc)
make

echo "5
1 127.0.0.1 11011
2 127.0.0.1 11012
3 127.0.0.1 11013
4 127.0.0.1 11014
5 127.0.0.1 11015
1 4 5
2 1
3 1 2
4 1 2
5 3 4" > membership

#start 5 processes, each broadcasting 1000 messages
for i in `seq 1 5`
do
    ./da_proc $i membership $num_msgs &
    da_proc_id[$i]=$!
done

#leave some time for process initialization
sleep $init_time

#do some nasty stuff like process crashes and delays
#example:
kill -TERM "${da_proc_id[5]}" #crash process 5
da_proc_id[5]=""
#kill -TERM "${da_proc_id[4]}" #crash process 4
#da_proc_id[4]=""

# start to broadcast in the following order
kill -USR2 "${da_proc_id[3]}"
sleep 0.1
kill -USR2 "${da_proc_id[4]}"
sleep 0.1
kill -USR2 "${da_proc_id[1]}"
sleep 0.1
kill -USR2 "${da_proc_id[2]}"


#do some more nasty stuff
#example:
kill -STOP "${da_proc_id[1]}" #pause process 1
sleep 0.1
kill -CONT "${da_proc_id[1]}" #resume process 1

#leave some time for the correct processes to broadcast all messages
sleep $time_to_finish

#stop all processes
for i in `seq 1 5`
do
    if [ -n "${da_proc_id[$i]}" ]; then
	kill -TERM "${da_proc_id[$i]}"
    fi
done
#wait until all processes stop
for i in `seq 1 5`
do
    if [ -n "${da_proc_id[$i]}" ]; then
        wait "${da_proc_id[$i]}"
    fi
done

#check logs for correctness
./check_output.sh 1 2 3 4

echo "Correctness test done."
