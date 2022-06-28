#include "lib.h"

void umain(int argc, char** argv){
	int r;
	
	r = user_create(argv[1], 0);
	if(r < 0){
		writef("\ncannot create dirctory %s: error: %d", argv[1], r );
	}

}
