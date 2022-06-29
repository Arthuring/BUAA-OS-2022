#include "lib.h"
#include <args.h>

int debug_ = 0;


int history_cnt = 0;

int read_history_cached(char history_cmd[128][128]){
	int index = ((history_cnt - 128) > 0) ? history_cnt-128 : 0;
	int i;
	char buf[128];
	int fd = open("/.history", O_RDONLY);
	if(fd < 0){
//		writef("error in open");
		return 0;
	}
	for(i = 0; i< index; i++){
//		writef("\njump index\n");
		read_line(fd, buf, 128);
	}
	for(i=index; i < history_cnt; i++){
//		writef("\n goning to read\n");
		read_line(fd, buf, 128);
//		writef("\nread end");
//		writef("\nbuf is:  %s", buf);
		strcpy(history_cmd[i-index], buf);
//		writef("\nread line end : %s\n ", buf);
	}
	close(fd);
	return history_cnt - index;
}

void write_history(char* buf, int n ){
	static int run_time = 0;
	int fd, r;
	if(run_time == 0){
		user_create("/.history", 0);
	}
	fd = open("/.history", O_WRONLY | O_APPEND );
	r = write(fd, buf, n);
	if(r < n){
		writef("\nerror in write_history\n");
	}
	r = write(fd, "\n", 1);
	if(r < 1){
		writef("\nerror in write_history\n ");
	}
	close(fd);
	history_cnt += 1;
//	writef("\nin write history:%d\n", history_cnt);
	run_time++;
}
//
// get the next token from string s
// set *p1 to the beginning of the token and
// *p2 just past the token.
// return:
//	0 for end-of-string
//	> for >
//	| for |
//	w for a word
//
// eventually (once we parse the space where the nul will go),
// words get nul-terminated.
#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

int
_gettoken(char *s, char **p1, char **p2)
{
	int t;

	if (s == 0) {
		//if (debug_ > 1) writef("GETTOKEN NULL\n");
		return 0;
	}

	//if (debug_ > 1) writef("GETTOKEN: %s\n", s);

	*p1 = 0;
	*p2 = 0;


	while(strchr(WHITESPACE, *s))
		*s++ = 0;
	if(*s == 0) {
	//	if (debug_ > 1) writef("EOL\n");
		return 0;
	}
	if(*s == '\"'){
		s++;
		*p1 = s;
		while( *s && *s != '\"' && *(s-1) != '\\'  ) s++;
		*p2 = s;
		*s = ' ';
		return 'w';
	}
	if(*s == '\''){
		s++;
		*p1 = s;
		while( *s && *s != '\'' && *(s-1) != '\\'  ) s++;
		*p2 = s;
		*s = ' ';
		return 'w';
	}
	if(strchr(SYMBOLS, *s)){
		t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
//		if (debug_ > 1) writef("TOK %c\n", t);
		return t;
	}
	*p1 = s;
	while(*s && !strchr(WHITESPACE SYMBOLS, *s))
		s++;
	*p2 = s;
	if (debug_ > 1) {
		t = **p2;
		**p2 = 0;
//		writef("WORD: %s\n", *p1);
		**p2 = t;
	}
	return 'w';
}

int
gettoken(char *s, char **p1)
{
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 16
void
runcmd(char *s, u_int env_id)
{
//	writef("\nrunning : %s", s);
	char *argv[MAXARGS], *t;
	int argc, c, i, r, p[2], fd, rightpipe;
	int fdnum;
	int run_back = 0, back_id;
	int isright;
	rightpipe = 0;
	gettoken(s, 0);
again:
	argc = 0;
	for(;;){
		c = gettoken(0, &t);
		switch(c){
		case 0:
			goto runit;
		case 'w':
			if(argc == MAXARGS){
				writef("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case '<':
			if(gettoken(0, &t) != 'w'){
				writef("syntax error: < not followed by word\n");
				exit();
			}
			// Your code here -- open t for reading,
			// dup it onto fd 0, and then close the fd you got.
			r=open(t, O_RDONLY);
			if(r < 0){
				user_panic("< open file failed");
			}
			fd = r;
			dup(fd, 0);
			close(fd);
			
			//user_panic("< redirection not implemented");
			break;
		case '>':
			// Your code here -- open t for writing,
			// dup it onto fd 1, and then close the fd you got.
			if(gettoken(0, &t) != 'w'){
				writef("syntax error: < not followed by word\n");
				exit();
			}
			r = open(t, O_WRONLY);
			if(r < 0){
				user_panic("> open file failed!");
			}
			fd = r;
			dup(fd, 1);
			close(fd);
			break;
			
			//user_panic("> redirection not implemented");
			//break;
		case '|':
			pipe(p);
			if((rightpipe = fork()) == 0){
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				goto again;
			}
			else{
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				goto runit;
			}
			//user_panic("| not implemented");
			break;
		case '&':
			run_back = 1;
			break;
		case ';':
			if(isright = fork() == 0 ){
				goto again;
			}else{
				goto runit;
			}
			break;	
		}
	}

runit:

	if(argc == 0) {
		if (debug_) writef("EMPTY COMMAND\n");
		return;
	}
	argv[argc] = 0;
	if (1) {
		writef("[%08x] SPAWN:", env->env_id);
		for (i=0; argv[i]; i++)
			writef(" %s", argv[i]);
		writef("\n");
	}
	
	if( strcmp(argv[0], "declare") == 0 || 
		strcmp(argv[0], "unset") == 0 ){
		r = -1;
		run_incmd(argc, argv, env_id);
	}
	else if ((r = spawn(argv[0], argv)) < 0){
		writef("spawn %s: %d\n", argv[0], r);
	}
	
//	if((r = syscall_set_env_status(r, ENV_RUNNABLE)) < 0){
//		writef("set child status wrong");
//	}


	close_all();
	if (r >= 0) {
		if(!run_back){
		if (debug_) writef("[%08x] WAIT %s %08x\n", env->env_id, argv[0], r);
			wait(r);
		}else{
			back_id = fork();
			if(back_id == 0){
				writef("\n[%08x] running\t", r);
				for(i=0; i<argc; i++){
					writef("%s ", argv[i]);
				}
				wait(r);
				writef("\n[%08x] done\t", r);
				for(i = 0; i<argc;i++){
					writef("%s ", argv[i]);
				}
				writef("\n");
				exit();
			}
		}
	}
	if (rightpipe) {
		if (debug_) writef("[%08x] WAIT right-pipe %08x\n", env->env_id, rightpipe);
		wait(rightpipe);
	}
	if(isright){
		if(debug_) writef("[%08x] WAIT right arg %08x\n", env->env_id, isright);
		wait(isright);
	}

	exit();
}

void
readline(char *buf, u_int n)
{
	int i, r;
	char historys[128][128];
	int  cnt = read_history_cached(historys);
	int sum = cnt;
	
//	writef("\nhistory_cnt: %d\n", history_cnt);
//	writef("\ncnt :%d\n ", cnt);

	r = 0;
	for(i=0; i<n; i++){
		if((r = read(0, buf+i, 1)) != 1){
			if(r < 0)
				writef("read error: %e", r);
			exit();
		}
		if(i >= 2 && buf[i - 2] == 27 && buf[i-1] == 91 && buf[i] == 65){
			writef("%c%c%c", 27, 91, 66);
			for(i -= 2; i; --i) writef("\b \b");
		//	writef("read up");
			if(cnt > 0){
				strcpy(buf, historys[--cnt]);
			}else{
				strcpy(buf, historys[cnt]);
			}
			writef("%s", buf);
			i =  strlen(buf) -1;
		}
		if(i >= 2 && buf[i - 2] == 27 && buf[i - 1] == 91 && buf[i] == 66){
//			writef("%c%c%c", 27, 91, 65);
//			writef("read down");
			if(cnt < sum - 1){
				for(i -= 2; i; --i) writef("\b \b");
//				writef("read down");
				strcpy(buf, historys[++cnt]);
				writef("%s", buf);
				i = strlen(buf) - 1;
			}else if (cnt == sum) {
				buf[i - 2] = buf[i - 1] = buf[i] = 0;
				i =  strlen(buf) - 1;
			}else{
				for(i-=2 ; i;--i) writef("\b \b");
				cnt++;
				buf[0]=0;
				i = -1; 
			}
		}
		
		if(buf[i] == '\b'){
			if(i > 0)
				i -= 2;
			else
				i = 0;
		}
		if(buf[i] == '\r' || buf[i] == '\n'){
			buf[i] = 0;
			return;
		}
	}
	writef("line too long\n");
	while((r = read(0, buf, 1)) == 1 && buf[0] != '\n');
	buf[0] = 0;
}	

char buf[1024];

void
usage(void)
{
	writef("usage: sh [-dix] [command-file]\n");
	exit();
}

void
umain(int argc, char **argv)
{
	int r, interactive, echocmds;
	u_int env_id = syscall_getenvid();
	interactive = '?';
	echocmds = 0;
	writef("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	writef("::                                                         ::\n");
	writef("::              Super Shell  V0.0.0_1                      ::\n");
	writef("::                                                         ::\n");
	writef(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN{
	case 'd':
		debug_++;
		break;
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}ARGEND

	if(argc > 1)
		usage();
	if(argc == 1){
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0)
			user_panic("open %s: %e", r);
		user_assert(r==0);
	}
	if(interactive == '?')
		interactive = iscons(0);
	for(;;){
		if (interactive)
			fwritef(1, "\n$ ");
		readline(buf, sizeof buf);
		write_history(buf, strlen(buf));
		if(buf[0] == 'e' && buf[1] == 'x' && buf[2] == 'i' && buf[3] == 't'){
			clear_envvar(env_id, ENVVAR_CLEAR);
			exit();
		}
		replace_envvar(buf, env_id);
		if (buf[0] == '#')
			continue;
		if (echocmds)
			fwritef(1, "# %s\n", buf);
		if ((r = fork()) < 0)
			user_panic("fork: %e", r);
		if (r == 0) {
			runcmd(buf, env_id);
			exit();
			return;
		} else
			wait(r);
	}
}

#define BREAK_SIG " \'\t\r\n$"
void replace_envvar(char *buf, u_int env_id){
	int ignore = 0;
	char* p = buf;
	char name[32] ={0};
	int namelen = 0;
	char value[128] = {0};
	char temp[1024] = {0};
	int i;
	while(*p){
	//	writef("\nin while");
		if(*p == '\''){
			ignore = 1 - ignore;
			p++;
		}else if(*p == '$' && !ignore){
	//		writef("\ngot $");
			for(namelen = 1; *(p + namelen) != 0 && !(strchr(BREAK_SIG, *(p+namelen))); namelen++){
		//		writef("\n*p: %c", *(p+namelen));
				name[namelen-1] = *(p + namelen);
			}
	//		writef("\nnamelen %d ", namelen);
			name[namelen-1] = 0;
			for(i=0; i<namelen-1;i++){
	//			writef("\nname[%d] :%c ",i, name[i]);
			}
	//		writef("\n name is %s", name);
			strcpy(temp, p+namelen);
			getvar(name, env_id, value);
	//		writef("\nvalue is %s", value);
		//	p++;
			strcpy(p, value);
			p += strlen(value);
			strcpy(p, temp);
		}else{
			p++;
		}
	}
//	writef("\nbuf is %s", buf);
}

void run_incmd(int argc, char ** argv, u_int env_id){
	if(strcmp(argv[0], "declare") == 0 ){
		declare(argc, argv, env_id);
	}else if(strcmp(argv[0], "unset") == 0 ){
		unset(argc, argv, env_id );
	}
}

void declare(int argc, char ** argv, u_int env_id){
	u_int option = 0;
//	writef("\n in declare: argc-%d, argv- ", argc);
	int i;
	for(i=0;i<argc;i++){
		writef("%s ", argv[i]);
	}
	ARGBEGIN
	{
		default:
			fwritef(1,  "\nusage: declare [-xr] [NAME[=VALUE]]\n");
			return;
		case 'r':
			option |= ENVVAR_RDONLY;
			break;
		case 'x':
			option |=ENVVAR_GLOB;
			break;
	}
	ARGEND	
	char name[32], value[128];
	char *p; 
	int r;
	if(argc > 1){
		fwritef(1, "\ntoo many argvs\n");
		return;
	}
	if(argc == 1){
		if((p = strchr(argv[0], '=')) > 0){
			option |= ENVVAR_SET;
			strcpy(name, argv[0]);
			name[p-argv[0]] = 0;
			p++;
			strcpy(value, p);
		}else{
			option |= ENVVAR_CREATE;
			strcpy(name, argv[0]);
		}
		syscall_env_var(name, value, env_id, option);
	}else{
		option |= ENVVAR_LIST;
		syscall_env_var( 0, 0, env_id, option);
	}
}

void unset(int argc, char **argv, u_int env_id){
	u_int option = 0;
	if(argc > 2){
		fwritef(1, "\ntoo many argvs\n");
		return;
	}
	else if(argc < 2){
		fwritef(1, "\ntoo few argvs\n");
		return;
	}
	else{
		option |= ENVVAR_UNSET;
		syscall_env_var(argv[1], 0, env_id, option);
	}
}

void getvar(char *name, u_int env_id, char* value){
	u_int option = 0;
	option |= ENVVAR_GET;
	syscall_env_var(name, value, env_id, option);
}

void clear_envvar(u_int env_id, u_int option){
	syscall_env_var(0, 0, env_id, option);
}
