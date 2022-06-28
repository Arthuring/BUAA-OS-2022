#include "lib.h"

void umain(int argc, char **argv){
	int N = (argv[1][0] - '0') * 10000000 , i;
	for(i = 0; i < N; i++ ){
		if(i == N/10){
			writef("\n-%%10-");	
		}		
		if(i == N/10 * 2){
			writef("\n--%%20--");
		}
		if(i == N / 10 *3){
			writef("\n---%%30---");
		}
		if(i == N / 10 *4){
			writef("\n----%%40----");
		}
		if(i == N / 10 *5){
			writef("\n-----%%50-----");
		}
		if(i == N / 10 *6){
			writef("\n------%%60------");
		}
		if(i == N / 10 *7){
			writef("\n--------%%70-------");
		}
		if(i == N / 10 *8){
			writef("\n---------%%80--------");
		}
		if(i == N / 10 *9){
			writef("\n-----------%%90---------");
		}
	}

	writef("\nFINISHED");
	
}
