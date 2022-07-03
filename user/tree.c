#include "lib.h"
#include "color.h"

int tree(char *path, int deep, int *isEnd);
void print_bar(int deep, int *isEnd);


void umain(int argc, char **argv){
	char path[MAXPATHLEN];
	int isEnd[32] = {0};
	if(argc == 1){
		strcpy(path, "/");
	}else{
		strcpy(path, argv[1]);
	}
	int fd = open(path, O_RDONLY);
	if(fd < 0){
		writef("\033[0m\033[1;31mfaied in open dirctory \"%s\"\n\033[0m", path);
		return;
	}
	struct Filefd* ffd = (struct Filefd *)num2fd(fd);
	if(ffd->f_file.f_type != FTYPE_DIR){
		writef("\033[0m\033[1;31mnot a dirctory \"%s\"\n\033[0m", path);
		return;	
	}
	tree(path, 0, isEnd);	
}

int tree(char *path,int deep, int *isEnd){
	int fd;
	struct File f, fnext;
	int r,n;
	int nowEnd = 0;
	char next_path[MAXPATHLEN];
	if((fd = open(path, O_RDONLY)) < 0 ){
		writef(RED(faied in open dirctory)"\"%s\"\n", path);
		return fd;
	}
	if(deep == 0){
		if(path[0] == '/'){
			writef("%s\n", path + 1);
			if(path[1] == 0) writef(LIGHT_BLUE(.\n));
		}else{
			writef(LIGHT_BLUE(%s\n), path);
		}
	}

	if((r = readn( fd, &fnext, sizeof f)) != sizeof f || fnext.f_name[0] == '\0'){
		return;
	}
	while(nowEnd != 1){
		f = fnext;
		if((r = readn( fd, &fnext, sizeof f)) != sizeof f || fnext.f_name[0] == '\0'){
			nowEnd = 1;
		}
		if(f.f_name[0] != '\0'){
			print_bar(deep, isEnd);
			if(nowEnd) writef("`-- ");
			else writef("|-- ");
			if(f.f_type == FTYPE_DIR) writef(LIGHT_BLUE(%s\n), f.f_name);
			else  if(f.f_name[strlen(f.f_name)-2] == '.'
				   &&f.f_name[strlen(f.f_name)-1] == 'b') writef("\033[0m\033[1;32m%s\n\033[0m", f.f_name);
			else writef("%s\n", f.f_name);
		}
		if(f.f_type == FTYPE_DIR){
			isEnd[deep] = nowEnd;
			strcpy(next_path, path);
			strcpy(next_path + strlen(next_path), "/");
			strcpy(next_path + strlen(next_path), f.f_name);
			tree(next_path, deep + 1, isEnd);
		}
	}
	close(fd);
}

void print_bar(int deep, int* isEnd){
	int i;
	int tot = deep * 3;
	for(i=0 ;i<tot; i++){
		if(i % 3 == 0 && isEnd[i/3] != 1){
			writef("|");
		}else{
			writef(" ");
		}
	}
}
