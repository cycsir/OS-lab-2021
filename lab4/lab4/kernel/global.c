
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"


PUBLIC	PROCESS		proc_table[NR_TASKS];

PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK	task_table[NR_TASKS] = {{task_tty, STACK_SIZE_TTY, "tty"},
					{TestA, STACK_SIZE_TESTA, "TestA"},
					{TestB, STACK_SIZE_TESTB, "TestB"},
					{TestC, STACK_SIZE_TESTC, "TestC"},
                                          {TestD, STACK_SIZE_TESTD, "TestD"},
                                          {TestE, STACK_SIZE_TESTE, "TestE"},
                                          {TestF, STACK_SIZE_TESTF, "TestF"}
                                         // {Writer, STACK_SIZE_TESTC, "Writer"}
};

PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];

PUBLIC	irq_handler	irq_table[NR_IRQ];

PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_my_sleep, sys_my_sprint, sys_P, sys_V};

//int wait_read_count = 0;
//int read_count = 0; // 读者人数
//
//int wait_write_count = 0;
//int write_count = 0; //
//PUBLIC  SEM  sem_table[3] = {{3,"read_sem"}};