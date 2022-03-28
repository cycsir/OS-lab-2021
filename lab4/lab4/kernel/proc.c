
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
PRIVATE char colors[] = {BLACK, 0x01,0x02,0x03,0x04,0x05} ;
/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			if (p->isWait == 0 && p->wakeup_tick<=ticks && p->ticks > greatest_ticks){
				greatest_ticks = p->ticks;
				p_proc_ready = p;

			}
            // 选择进程的条件 :没有被阻塞（即没有因为拿不到信号量被挂起），进程仍需要的时间片 （ticks). 进程所需要的时间片
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				p->ticks = p->priority;
			}
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}



/*======================================================================*
                           sys_my_sleep
 *======================================================================*/
PUBLIC void sys_my_sleep(int milli_seconds){
    int sleep_ticks = milli_seconds/1000*HZ;
    p_proc_ready->wakeup_tick = get_ticks()+sleep_ticks;
    schedule();
}

/*======================================================================*
                           sys_my_sprint
 *======================================================================*/

PUBLIC void sys_my_sprint(void *str){
    char *s = str;
    // disp_color_str(str, BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));  //
    disp_str(str);
}

PUBLIC void sys_P(void* mutex){
    disable_irq(CLOCK_IRQ); //
    SEM* sem = mutex;
//    disp_str(p_proc_ready->p_name);
//    disp_str(" P ");
//    disp_str(sem->name);

    if (sem->sem_count==0){
        p_proc_ready->isWait = 1;
        sem->waiting_proc[sem->wait_count]=p_proc_ready;
        sem->wait_count++;
        schedule();
        enable_irq(CLOCK_IRQ);
        return;
    }
    sem->sem_count--;
    enable_irq(CLOCK_IRQ);
}

PUBLIC void sys_V(void* mutex){
    disable_irq(CLOCK_IRQ);
    SEM* sem = mutex;
//    disp_str(p_proc_ready->p_name);
//    disp_str(" V ");
//    disp_str(sem->name);
    sem->sem_count++;
    if(sem->wait_count>0){
        // 出队
        p_proc_ready = sem->waiting_proc[0];
        p_proc_ready->isWait = 0;
        sem->sem_count--;
        for (int i = 0; i < sem->wait_count-1; i++){
            sem->waiting_proc[i] = sem->waiting_proc[i+1];
        }
        sem->wait_count--;
    }
    enable_irq(CLOCK_IRQ);
}