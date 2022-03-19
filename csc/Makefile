fibo: ./code/fibo.o ./code/main.o
	gcc ./code/fibo.o ./code/main.o -o fibo
./code/main.o: ./code/main.c
	gcc -I./include -c ./code/main.c -o ./code/main.o
./code/fibo.o: ./code/fibo.c
	gcc -I./include -c ./code/fibo.c -o ./code/fibo.o

clean:
	rm ./code/main.o
	rm ./code/fibo.o
	
	



