#include "lib.h"

void umain(int argc, char**  argv){
	int r;
	r = user_create(argv[1], 1);
	if(r < 0){
		writef("cannot create directory \'%s\': %d ",argv[1], r );
	}
}
