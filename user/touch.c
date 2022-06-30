#include "lib.h"
#include "color.h"
void umain(int argc, char** argv){
	int r;
	
	if(argc < 2){
		writef(RED(too few argv\n));
		return;
	}
	else if(argc > 2){
		writef(RED(too many argv\n));
	}
	r = user_create(argv[1], 0);
	if(r < 0){
		writef("\033[0m\033[1;31m\ncannot create file \"%s\": error: %d\033[0m", argv[1], r );
	}

}
