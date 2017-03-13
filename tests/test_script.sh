for ((a = 1; a <= 100; a++))
do
	./client localhost 3001 ../tests/Transactions.txt 0.2 &
done
wait