#include "lib.h"

void umain(int argc, char**argv){
	int fd = open("/.history",O_RDONLY);
	char buf[1024];
	int r;
	if(fd  < 0){
		writef("\n./history not found\n");
	}
	while((r = readn(fd, buf, 1024)) != 0 ){
		writef("%s", buf);
	}
}
