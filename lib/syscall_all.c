#include "../drivers/gxconsole/dev_cons.h"
#include <mmu.h>
#include <env.h>
#include <printf.h>
#include <pmap.h>
#include <sched.h>
#include <error.h>
#include <print.h>

#define NHASH (1<<8)
extern char *KERNEL_SP;
extern struct Env *curenv;

/* Overview:
 * 	This function is used to print a character on screen.
 *
 * Pre-Condition:
 * 	`c` is the character you want to print.
 */
void sys_putchar(int sysno, int c, int a2, int a3, int a4, int a5)
{
	printcharc((char) c);
	return ;
}

u_int hash(char* str){
	u_int h = 0;
	char * s;
	for(s = str; *s!='\0'; s++){
		h = 31 * h + *s;
		h =  h % NHASH;
	}
	return h % NHASH;
}

int strcmp(const char *p, const char *q ){
	while (*p && *q && *p == *q){
		p++;
		q++;
	}
	if((u_int)*p < (u_int)*q ){
		return -1;
	}
	if((u_int)*p > (u_int)*q ){
		return 1;
	}
	return 0;
}

char *strcpy(char *dst, const char *src){
	char *ret;
	ret = dst;
	while((*src) != '\0'){
		*dst = *src;
		dst++;
		src++;
	}
	return ret;
}

#define ENVVAR_CLEAR (1 << 7)
#define GET (1 << 6)
#define LIST (1 << 5)
#define UNSET (1 << 4)
#define CREATE (1 << 3)//if set, return value, else set name
#define SET (1 << 2) // recover value, or glob readonly 
#define GLOB (1 << 1)
#define RDONLY (1 << 0)
//char name_list[NHASH][32];
int sys_env_var(int sysno, char* name, char* value, u_int env_id, u_int option){
	static int cnt = 0;
	static char name_list[NHASH][32] = {0};
	static u_int envid_list[NHASH] = {0};
	static char value_list[NHASH][128];
	static int readonly[NHASH] = {0} ;
	static int glob[NHASH] = {0};
	int i;
//	printf("\n in sys_env_var:");
//	if(name){
//		printf("%s", name);
//	}
//	if(value){
//		printf("%s,", value);
//	}
//	printf("envid:%x",env_id);
//	printf("option: %b", option);
	if(option & ENVVAR_CLEAR){
		for(i = 0; i < NHASH; i++ ){
			if(glob[i] != 1 && envid_list[i] == env_id ){
				name_list[i][0] = 0;
				value_list[i][0] = 0;
				envid_list[i] = 0;
				glob[i] = 0;
				readonly[i] = 0;
			}
		}
		return 0;
	}

	if(option & LIST){
		printf(LIGHT_GREEN(-------------------ENV_VAR-------------------\n));
		for(i=0; i < NHASH ; i++){
			if(name_list[i][0] != 0){
//				printf(" name: %s glob: %d, env_id %d, envid_list: %d", name_list[i], glob[i], env_id, envid_list[i]);
				if(glob[i] == 1 || envid_list[i] == env_id ){
					printf("%s=%s\n", name_list[i], value_list[i], glob[i], envid_list[i]);
				}
			}
		}
		printf(LIGHT_GREEN(--------------------------------------------\n));
		return 0;
	}
	u_int index = hash(name);
//	printf("hash: %d ", index);

	while(name_list[index][0]){
		if(strcmp(name_list[index], name) == 0 && (env_id == envid_list[index] || glob[index] == 1 )){
			if ((option & CREATE) && (!(option & GLOB)) && (!(option & RDONLY)))	{
				printf("%s=%s", name_list[index],value_list[index]);
				return -16;
			}
			break;		
		}else{
			index++;
			if(index == NHASH ) index =0;	
		}
	}
	
	if((option & SET)){
		strcpy(name_list[index], name);
		if(!readonly[index]){
			strcpy(value_list[index], value);
		}else{
			printf("\033[0m\033[1;31m\nfailed,\"%s\" is readonly\n\033[0m", name);
		}
		if((option & GLOB) && glob[index] == 0 ){
			glob[index] = 1;
			envid_list[index] = 0;
		}else if(glob[index] == 0 ) {
			envid_list[index] = env_id;
		}
		readonly[index] |= (option & RDONLY);
	}else if(option & CREATE){
		strcpy(name_list[index], name);
		readonly[index] |= (option & RDONLY);
		if(!readonly[index]){
			//value_list[index][0] = 0;
		}
		if((option & GLOB) && glob[index] == 0 ){
			glob[index] = 1;
			envid_list[index] = 0;
		}else if(glob[index] == 0 ) {
			envid_list[index] = env_id;
		}
	}else if(option == GET){
		if(strcmp(name_list[index], name) != 0){
			value[0] = 0;
			return -E_VAR_NOT_FOUND;
		}
		strcpy(value, value_list[index]);
	}else if(option & UNSET){
		if(strcmp(name_list[index], name) != 0){
			printf("\033[0m\033[1;31m\nfailed,\"%s\" is not found\n\033[0m", name);
			return -E_VAR_NOT_FOUND;
		}
		if(readonly[index] == 1){
			printf("\033[0m\033[1;31m\nfailed,\"%s\" is readonly\n\033[0m", name);
			return -E_VAR_READONLY;
		}
		name_list[index][0] = 0;
		value_list[index][0] = 0;
		glob[index] = 0;
		readonly[index] = 0;
		envid_list[index] = 0;
	}
	return 0;
}

/* Overview:
 * 	This function enables you to copy content of `srcaddr` to `destaddr`.
 *
 * Pre-Condition:
 * 	`destaddr` and `srcaddr` can't be NULL. Also, the `srcaddr` area
 * 	shouldn't overlap the `destaddr`, otherwise the behavior of this
 * 	function is undefined.
 *
 * Post-Condition:
 * 	the content of `destaddr` area(from `destaddr` to `destaddr`+`len`) will
 * be same as that of `srcaddr` area.
 */
void *memcpy(void *destaddr, void const *srcaddr, u_int len)
{
	char *dest = destaddr;
	char const *src = srcaddr;

	while (len-- > 0) {
		*dest++ = *src++;
	}

	return destaddr;
}

/* Overview:
 *	This function provides the environment id of current process.
 *
 * Post-Condition:
 * 	return the current environment id
 */
u_int sys_getenvid(void)
{
	return curenv->env_id;
}

/* Overview:
 *	This function enables the current process to give up CPU.
 *
 * Post-Condition:
 * 	Deschedule current process. This function will never return.
 *
 * Note:
 *  For convenience, you can just give up the current time slice.
 */
/*** exercise 4.6 ***/
void sys_yield(void)
{
	bcopy((void *)KERNEL_SP - sizeof(struct Trapframe), (void *)TIMESTACK - sizeof(struct Trapframe), sizeof(struct Trapframe));
	sched_yield();
}

/* Overview:
 * 	This function is used to destroy the current environment.
 *
 * Pre-Condition:
 * 	The parameter `envid` must be the environment id of a
 * process, which is either a child of the caller of this function
 * or the caller itself.
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 when error occurs.
 */
int sys_env_destroy(int sysno, u_int envid)
{
	/*
		printf("[%08x] exiting gracefully\n", curenv->env_id);
		env_destroy(curenv);
	*/
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0) {
		return r;
	}

	//printf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	env_destroy(e);
	return 0;
}

/* Overview:
 * 	Set envid's pagefault handler entry point and exception stack.
 *
 * Pre-Condition:
 * 	xstacktop points one byte past exception stack.
 *
 * Post-Condition:
 * 	The envid's pagefault handler will be set to `func` and its
 * 	exception stack will be set to `xstacktop`.
 * 	Returns 0 on success, < 0 on error.
 */
/*** exercise 4.12 ***/
int sys_set_pgfault_handler(int sysno, u_int envid, u_int func, u_int xstacktop)
{
	// Your code here.
	struct Env *env;
	int ret;
	ret = envid2env(envid, &env, 0);
	if(ret  < 0){
		return ret;
	}
	env->env_pgfault_handler = func;
	env->env_xstacktop = xstacktop;

	return 0;
	//	panic("sys_set_pgfault_handler not implemented");
}

/* Overview:
 * 	Allocate a page of memory and map it at 'va' with permission
 * 'perm' in the address space of 'envid'.
 *
 * 	If a page is already mapped at 'va', that page is unmapped as a
 * side-effect.
 *
 * Pre-Condition:
 * perm -- PTE_V is required,
 *         PTE_COW is not allowed(return -E_INVAL),
 *         other bits are optional.
 *
 * Post-Condition:
 * Return 0 on success, < 0 on error
 *	- va must be < UTOP
 *	- env may modify its own address space or the address space of its children
 */
/*** exercise 4.3 ***/
int sys_mem_alloc(int sysno, u_int envid, u_int va, u_int perm)
{
	// Your code here.
	struct Env *env;
	struct Page *ppage;
	int ret;
	ret = 0;
	perm = perm & (BY2PG - 1);
	if(va >= UTOP){
		return -E_INVAL;
	}
	if((perm & PTE_COW) || (!(perm & PTE_V))){
		return -E_INVAL;
	}
	ret = envid2env(envid, &env, 1);
	if(ret < 0){
		return ret;
	}
	ret = page_alloc(&ppage);
	if(ret < 0){
		return ret;
	}
	ret = page_insert(env->env_pgdir,ppage, va, perm);
	if(ret < 0){
		return ret;
	}
	return 0;
}

/* Overview:
 * 	Map the page of memory at 'srcva' in srcid's address space
 * at 'dstva' in dstid's address space with permission 'perm'.
 * Perm must have PTE_V to be valid.
 * (Probably we should add a restriction that you can't go from
 * non-writable to writable?)
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Note:
 * 	Cannot access pages above UTOP.
 */
/*** exercise 4.4 ***/
int sys_mem_map(int sysno, u_int srcid, u_int srcva, u_int dstid, u_int dstva,
				u_int perm)
{
	int ret;
	u_int round_srcva, round_dstva;
	struct Env *srcenv;
	struct Env *dstenv;
	struct Page *ppage;
	Pte *ppte;

	ppage = NULL;
	ret = 0;
	round_srcva = ROUNDDOWN(srcva, BY2PG);
	round_dstva = ROUNDDOWN(dstva, BY2PG);

    //your code here
	if(round_srcva >= UTOP || round_dstva >= UTOP){
		return -E_INVAL;
	}
	if(!(perm & PTE_V)){
		return -E_INVAL;
	}
	ret = envid2env(srcid, &srcenv,0);
	if(ret < 0){
		return ret;
	}
	ret = envid2env(dstid, &dstenv,0);
	if(ret < 0){
		return ret;
	}
	ppage = page_lookup(srcenv->env_pgdir, round_srcva, &ppte);
	if(ppage == NULL){
		return -E_INVAL;
	}
	if(((*ppte & PTE_R) == 0) && ((perm & PTE_R) != 0 )){
		return -E_INVAL;
	}
	ret = page_insert(dstenv->env_pgdir, ppage, round_dstva, perm);
	if(ret < 0){
		return ret;
	}

	return ret;
}

/* Overview:
 * 	Unmap the page of memory at 'va' in the address space of 'envid'
 * (if no page is mapped, the function silently succeeds)
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Cannot unmap pages above UTOP.
 */
/*** exercise 4.5 ***/
int sys_mem_unmap(int sysno, u_int envid, u_int va)
{
	// Your code here.
	int ret;
	struct Env *env;
	if(va >= UTOP){
		return -E_INVAL;
	}
	ret = envid2env(envid, &env, 0);
	if(ret < 0){
		return ret;
	}
	page_remove(env->env_pgdir,va);
	return 0;
	//	panic("sys_mem_unmap not implemented");
}

/* Overview:
 * 	Allocate a new environment.
 *
 * Pre-Condition:
 * The new child is left as env_alloc created it, except that
 * status is set to ENV_NOT_RUNNABLE and the register set is copied
 * from the current environment.
 *
 * Post-Condition:
 * 	In the child, the register set is tweaked so sys_env_alloc returns 0.
 * 	Returns envid of new environment, or < 0 on error.
 */
/*** exercise 4.8 ***/
int sys_env_alloc(void)
{
	// Your code here.
	int r;
	struct Env *e;
	r = env_alloc(&e, curenv->env_id);
	if(r < 0){
		return r;
	}
	bcopy((void*)KERNEL_SP - sizeof(struct Trapframe), (void *)&(e->env_tf), sizeof(struct Trapframe));
	e->env_tf.pc = e->env_tf.cp0_epc;
	e->env_tf.regs[2] = 0;
	e->env_status = ENV_NOT_RUNNABLE;
	e->env_pri = curenv->env_pri;

	
	return e->env_id;
	//	panic("sys_env_alloc not implemented");
}

/* Overview:
 * 	Set envid's env_status to status.
 *
 * Pre-Condition:
 * 	status should be one of `ENV_RUNNABLE`, `ENV_NOT_RUNNABLE` and
 * `ENV_FREE`. Otherwise return -E_INVAL.
 *
 * Post-Condition:
 * 	Returns 0 on success, < 0 on error.
 * 	Return -E_INVAL if status is not a valid status for an environment.
 * 	The status of environment will be set to `status` on success.
 */
/*** exercise 4.14 ***/
int sys_set_env_status(int sysno, u_int envid, u_int status)
{
	// Your code here.
	struct Env *env;
	int ret;
	
	if(status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE && status != ENV_FREE)
	{
		return -E_INVAL;
	}
	ret = envid2env(envid, &env, 0);
	if(ret != 0) return ret;
	if(env->env_status != ENV_RUNNABLE && status == ENV_RUNNABLE){
		LIST_INSERT_HEAD(&env_sched_list[0], env, env_sched_link);
	}
	if(env->env_status == ENV_RUNNABLE && status != ENV_RUNNABLE){
		LIST_REMOVE(env, env_sched_link);
	}
	env->env_status = status;
	
	return 0;
	//	panic("sys_env_set_status not implemented");
}

/* Overview:
 * 	Set envid's trap frame to tf.
 *
 * Pre-Condition:
 * 	`tf` should be valid.
 *
 * Post-Condition:
 * 	Returns 0 on success, < 0 on error.
 * 	Return -E_INVAL if the environment cannot be manipulated.
 *
 * Note: This hasn't be used now?
 */
int sys_set_trapframe(int sysno, u_int envid, struct Trapframe *tf)
{

	return 0;
}

/* Overview:
 * 	Kernel panic with message `msg`.
 *
 * Pre-Condition:
 * 	msg can't be NULL
 *
 * Post-Condition:
 * 	This function will make the whole system stop.
 */
void sys_panic(int sysno, char *msg)
{
	// no page_fault_mode -- we are trying to panic!
	panic("%s", TRUP(msg));
}

/* Overview:
 * 	This function enables caller to receive message from
 * other process. To be more specific, it will flag
 * the current process so that other process could send
 * message to it.
 *
 * Pre-Condition:
 * 	`dstva` is valid (Note: NULL is also a valid value for `dstva`).
 *
 * Post-Condition:
 * 	This syscall will set the current process's status to
 * ENV_NOT_RUNNABLE, giving up cpu.
 */
/*** exercise 4.7 ***/
void sys_ipc_recv(int sysno, u_int dstva)
{
	if(dstva >= UTOP){
		return;
	}
	curenv->env_ipc_recving = 1;
	curenv->env_ipc_dstva = dstva;
	curenv->env_status = ENV_NOT_RUNNABLE;
	sys_yield();
}

/* Overview:
 * 	Try to send 'value' to the target env 'envid'.
 *
 * 	The send fails with a return value of -E_IPC_NOT_RECV if the
 * target has not requested IPC with sys_ipc_recv.
 * 	Otherwise, the send succeeds, and the target's ipc fields are
 * updated as follows:
 *    env_ipc_recving is set to 0 to block future sends
 *    env_ipc_from is set to the sending envid
 *    env_ipc_value is set to the 'value' parameter
 * 	The target environment is marked runnable again.
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Hint: the only function you need to call is envid2env.
 */
/*** exercise 4.7 ***/
int sys_ipc_can_send(int sysno, u_int envid, u_int value, u_int srcva,
					 u_int perm)
{

	int r;
	struct Env *e;
	struct Page *p;
	perm = perm & (BY2PG - 1);
	if(srcva >= UTOP){
		return -E_INVAL;
	}
	r = envid2env(envid, &e, 0);
	if(r < 0) return r;
	if(e->env_ipc_recving == 0){
		return -E_IPC_NOT_RECV;
	}
	e->env_ipc_recving = 0;
	e->env_ipc_from = curenv->env_id;
	e->env_ipc_value = value;
	e->env_ipc_perm = perm;
	e->env_status = ENV_RUNNABLE;

	if(srcva != 0){
		p = page_lookup(curenv->env_pgdir, srcva, NULL);
		if(p == NULL){
			return -E_INVAL;
		}
		page_insert(e->env_pgdir, p, e->env_ipc_dstva, perm);
	}
	return 0;
}
/* Overview:
 * 	This function is used to write data to device, which is
 * 	represented by its mapped physical address.
 *	Remember to check the validity of device address (see Hint below);
 * 
 * Pre-Condition:
 *      'va' is the starting address of source data, 'len' is the
 *      length of data (in bytes), 'dev' is the physical address of
 *      the device
 * 	
 * Post-Condition:
 *      copy data from 'va' to 'dev' with length 'len'
 *      Return 0 on success.
 *	Return -E_INVAL on address error.
 *      
 * Hint: Use ummapped segment in kernel address space to perform MMIO.
 *	 Physical device address:
 *	* ---------------------------------*
 *	|   device   | start addr | length |
 *	* -----------+------------+--------*
 *	|  console   | 0x10000000 | 0x20   |
 *	|    IDE     | 0x13000000 | 0x4200 |
 *	|    rtc     | 0x15000000 | 0x200  |
 *	* ---------------------------------*
 */
 /*** exercise 5.1 ***/
int sys_write_dev(int sysno, u_int va, u_int dev, u_int len)
{
        // Your code here
	if((0x10000000 <= dev && dev + len <= 0x10000000 + 0x20)
			|| (0x13000000 <= dev && dev + len <= 0x13000000 + 0x4200)
			|| (0x15000000 <= dev && dev + len <= 0x15000000 + 0x200)){
			bcopy((char *)va, (char *)(dev + 0xA0000000), len);
			return 0;
	}
	return -E_INVAL;
}

/* Overview:
 * 	This function is used to read data from device, which is
 * 	represented by its mapped physical address.
 *	Remember to check the validity of device address (same as sys_write_dev)
 * 
 * Pre-Condition:
 *      'va' is the starting address of data buffer, 'len' is the
 *      length of data (in bytes), 'dev' is the physical address of
 *      the device
 * 
 * Post-Condition:
 *      copy data from 'dev' to 'va' with length 'len'
 *      Return 0 on success, < 0 on error
 *      
 * Hint: Use ummapped segment in kernel address space to perform MMIO.
 */
 /*** exercise 5.1 ***/
int sys_read_dev(int sysno, u_int va, u_int dev, u_int len)
{
        // Your code here
		if((0x10000000 <= dev && dev + len <= 0x10000000 + 0x20)
				|| (0x13000000 <= dev && dev + len <= 0x13000000 + 0x4200)
				|| (0x15000000 <= dev && dev + len <= 0x15000000 + 0x200)){
			bcopy((char *)(dev + 0xA0000000), (char *)va, len);
			return 0;
		}
		return -E_INVAL;
}
