
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"


enum {
    READER_FIRST,
    WRITER_FIRST,
};
enum {
    ACCEPT,
    AVOID
};
const int MODE = WRITER_FIRST;
const int H_MODE = AVOID;
const int MAX_READERS = 2;

SEM sem_table[] = {
        {MAX_READERS,{},0,"read_sem"},
        {1,{},0,"write_sem"},
        {1,{},0,"write_block"},
        {1,{},0,"change_read"},
        {1,{},0,"read_block"},
        {1,{},0,"change_write"}

};
SEM *read_sem = &sem_table[0];

SEM *write_sem = &sem_table[1];

SEM *change_read = &sem_table[3];

SEM  *change_write = &sem_table[5];



char colors[] = {BLACK, 0x01,0x02,0x03,0x04,0x05 };
int counts[4] = {0, 0, 0, 0};

//int wait_read_count;
//int read_count; // 读者人数
//
//int wait_write_count;
//int write_count; //
/*======================================================================*
                            kernel_main
 *======================================================================*/


PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid
        p_proc->wakeup_tick = 0;
        p_proc->isWait = 0;
        p_proc->isFinish = 0;
		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

    // 分配时间片
	proc_table[0].ticks = proc_table[0].priority =  15;

    // reader: A:2 BC:3
	proc_table[1].ticks = proc_table[1].priority =  2;
	proc_table[2].ticks = proc_table[2].priority =  2;
    proc_table[3].ticks = proc_table[3].priority =  2;

    // writer: D:3 E:4
    proc_table[4].ticks = proc_table[4].priority =  2;
    proc_table[5].ticks = proc_table[5].priority =  2;


    proc_table[6].ticks = proc_table[6].priority =  2;

    wait_read_count = 0;
    read_count = 0; // 读者人数

    wait_write_count = 0;
    write_count = 0; //
    avoid_hungry = 0;


    k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
        init_keyboard();

	restart();

	while(1){}
}




/*======================================================================*
                               Reader
 *======================================================================*/

/*======================================================================*
                               TestA
 *======================================================================*/

PUBLIC void read(int time){
    disp_color_str(p_proc_ready->p_name, BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
    disp_color_str(":readstart ", BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
    // my_sprint(":readstart");

    disp_color_str(p_proc_ready->p_name, BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
    disp_color_str(":reading ", BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
    // my_sprint(":reading ");
    milli_delay(time);
    disp_color_str(p_proc_ready->p_name, BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
    disp_color_str(":readfinish ", BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
    // my_sprint(":readfinish ");

}


void reader(int time)
{
    int sleep_time = time;
	while (1) {
		/* disp_str("A."); */


        switch (MODE) {
            case READER_FIRST:
                my_p(change_read);
                if (read_count == 0 && wait_read_count == 0) { // 说明当前并不在读，所以要等写进程释放信号量
                    my_p(write_sem);
                }
                wait_read_count++;
                my_v(change_read);

                my_p(read_sem);//开始读

                my_p(change_read);
                wait_read_count--;
                read_count++;
                my_v(change_read);
                //读
                read(time);

                my_p(change_read);
                read_count--;
                if (read_count == 0 && wait_read_count == 0) {  // 没有读进程了，包括等待中和进行中的。这才释放信号量给写进程
                    my_v(write_sem);
                }
                p_proc_ready->isFinish = 1;
                my_v(change_read);

                my_v(read_sem);//读完成
                break;
            case WRITER_FIRST:
                my_p(read_sem);//开始读

                my_p(change_read);
                read_count++;
                my_v(change_read);
                //读
                read(time);

                my_p(change_read);
                read_count--;
                p_proc_ready->isFinish = 1;
                my_v(change_read);

                my_v(read_sem);//读完成

                break;
            default:
                break;
        }
		milli_delay(10);
        switch (H_MODE) {
            case ACCEPT:
                my_sleep(1);
                break;
            case AVOID:
                my_sleep(sleep_time);
                break;
            default: ;
        }

	}
}



void TestA(){
    int i = 0;
    int time = 20000;
    reader(time);
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	int i = 0x1000;
    int time = 30000;
    reader(time);
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestC()
{
	int i = 0x2000;
    int time = 30000;
    reader(time);
}

/*======================================================================*
                               Writer
 *======================================================================*/

PUBLIC void write(int time){
    disp_color_str(p_proc_ready->p_name, BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
    disp_color_str(":writestart ", BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));

    disp_color_str(p_proc_ready->p_name, BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
    disp_color_str(":writing ", BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
    milli_delay(time);
    disp_color_str(p_proc_ready->p_name, BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
    disp_color_str(":writefinish ", BRIGHT | MAKE_COLOR(BLACK, colors[p_proc_ready->pid]));
}

void writer(int time){
    int i = 0x3000;
    int sleep_time = time;
    while (1){
        switch (MODE) {
            case READER_FIRST:
                my_p(write_sem);
                my_p(change_write);
                write_count++;
                // 写
                write(time);
                write_count--;
                p_proc_ready->isFinish = 1;
                my_v(change_write);
                my_v(write_sem);
                break;
            case WRITER_FIRST:
                my_p(change_write);
                if (write_count == 0 && wait_write_count == 0) {// 说明当前并不在写，所以要等读进程释放信号量
                    for (int j = 0; j < MAX_READERS; ++j) {
                        my_p(read_sem);

                    }
                }
                wait_write_count++;
                my_v(change_write);

                my_p(write_sem);//开始写

                my_p(change_write);
                wait_write_count--;
                write_count++;
                my_v(change_write);
                //写
                write(time);

                my_p(change_write);
                write_count--;
                if (write_count == 0 && wait_write_count == 0) {  // 没有写进程了，包括等待中和进行中的。这才释放信号量给读进程
                    for (int j = 0; j < MAX_READERS; ++j) {
                        my_v(read_sem);
                    }
                }
                p_proc_ready->isFinish = 1;
                my_v(change_write);
                my_v(write_sem);//写完成
                break;
            default:
                break;
        }
        milli_delay(10);
        switch (H_MODE) {
            case ACCEPT:
                my_sleep(1);
                break;
            case AVOID:
                my_sleep(sleep_time);
                break;
            default: ;
        }
    }

}
void TestD(){
    int i = 0x3000;
    int time = 30000;
    writer(time);

}

void TestE(){
    int i = 0x4000;
    int time = 40000;
    writer(time);
}

void TestF(){
    int i = 0x5000;
    int time = 10000;
    // my_sprint("start_F");
    while (1){
//        my_p(change_read);
//        my_p(change_write);
        if(read_count!=0){
            my_sprint("reading:");
            disp_int(read_count);
            my_sprint(" ");
        } else{
            my_sprint("writing:");
            disp_int(write_count);
            my_sprint(" ");
        }
//        my_v(change_read);
//        my_p(change_write);
        my_sleep(time);
    }
    my_sleep(1);
}

