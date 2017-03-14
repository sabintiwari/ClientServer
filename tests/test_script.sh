#	Sabin Raj Tiwari
#	CMSC 621
#	Project 1

PORT=3000
TYPE=$1
COUNT=$2

# Remove the time log file and create it again.
rm -f ./logs/_time_log_file.txt
touch ./logs/_time_log_file.txt

# If the first argument passed equals 1
if [ $TYPE -eq 1 ]
then
	# Perform the test for the second number passed in the arguments.
	for ((a = 1; a <= $COUNT; a++))
	do
		sleep 0.1s
		./client localhost $PORT 0.5 ./Transactions.txt TEST &
	done
	wait
# If the first argument passed equals 2
elif [ $TYPE -eq 2 ]
then
	# Perform the test for 20 clients with differing timesteps.
	for ((a = 1; a <= 20; a++))
	do
		sleep 0.1s
		./client localhost $PORT $COUNT ./Transactions.txt TEST &
	done
	wait
fi