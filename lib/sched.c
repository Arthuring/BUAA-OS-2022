#include <env.h>
#include <pmap.h>
#include <printf.h>

/* Overview:
 *  Implement simple round-robin scheduling.
 *
 *
 * Hints:
 *  1. The variable which is for counting should be defined as 'static'.
 *  2. Use variable 'env_sched_list', which is a pointer array.
 *  3. CANNOT use `return` statement!
 */
/*** exercise 3.15 ***/
struct Env *e = NULL;
void sched_yield(void)
{
    static int count = 0; // remaining time slices of current env
    static int point = 0; // current env_sched_list index
	static struct Env *e = NULL;
    /*  hint:
     *  1. if (count==0), insert `e` into `env_sched_list[1-point]`
     *     using LIST_REMOVE and LIST_INSERT_TAIL.*/
    /*  2. if (env_sched_list[point] is empty), point = 1 - point;
     *     then search through `env_sched_list[point]` for a runnable env `e`, 
     *     and set count = e->env_pri
     *  3. count--
     *  4. env_run()
     *
     *  functions or macros below may be used (not all):
     *  LIST_INSERT_TAIL, LIST_REMOVE, LIST_FIRST, LIST_EMPTY
     */
	
	e = curenv;
	/*
	if(count == 0 || e == NULL || e->env_status == ENV_NOT_RUNNABLE ){
		do{
			e = LIST_FIRST( &env_sched_list[point]);
			if(e == NULL){
				if(LIST_EMPTY(&env_sched_list[point])){
					point = 1- point;
					e = LIST_FIRST(&env_sched_list[point]);
				}
				if(LIST_EMPTY(&env_sched_list[point])){
					continue;
				}
			
			}
			count =  e->env_pri;
			LIST_REMOVE(e, env_sched_link);
			LIST_INSERT_TAIL(&env_sched_list[1-point], e,env_sched_link);
			if(LIST_EMPTY(&env_sched_list[point])){
				point = 1- point;
			}	
		} while(e == NULL || e->env_status != ENV_RUNNABLE);
	}*/
	
	if(count == 0 || e == NULL || e->env_status != ENV_RUNNABLE){
		if(e != NULL){
			LIST_REMOVE(e, env_sched_link);
			LIST_INSERT_TAIL(&env_sched_list[1-point],e , env_sched_link);
		}
		while(1){
			while(LIST_EMPTY(&env_sched_list[point])) point = 1-point;
			e = LIST_FIRST(&env_sched_list[point]);
			if(e->env_status == ENV_RUNNABLE){
				count = e->env_pri;
				break;
			}else{
				if(e->env_status == ENV_NOT_RUNNABLE){
					LIST_REMOVE(e, env_sched_link);
					LIST_INSERT_TAIL(&env_sched_list[1-point], e, env_sched_link);
				}
			}
		}
	}

	count --;
	
	env_run(e);
}
