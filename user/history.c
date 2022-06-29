#include "lib.h"

void umain(int argc, char**argv){
	int fd = open("/.history",O_RDONLY);
	char buf[128];
	int r;
	if(fd  < 0){
		writef("\n./history not found\n");
	}
	int cnt = 1;
	while((r = read_line(fd, buf, 128)) != 0 ){
		writef("%d\t%s\n",cnt, buf);
		cnt++;
	}
}
