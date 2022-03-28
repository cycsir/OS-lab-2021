
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
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
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)
PUBLIC int RED_COLOR_FLAG = 0x07; /* 0000 1100 黑底红字*/
PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

void change_color();



PUBLIC void __stack_chk_fail_local(){}
PUBLIC void __stack_chk_fail(){}
extern PUBLIC void clean_screen(CONSOLE *p_console);

void input_char(TTY *p_tty, u32 key);

extern int RED_COLOR_FLAG;
extern int MODE;
extern PUBLIC void search_char(char ch);
extern PUBLIC void search(CONSOLE* p_con);
extern PUBLIC void finish_search(CONSOLE* p_con);
extern PUBLIC void recover(CONSOLE* p_con);
extern PUBLIC void begin_search(CONSOLE* p_con);
/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;

	init_keyboard();
    MODE = NORMAL;
	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
        clean_screen(p_tty->p_console);
	}
	select_console(0);
	while (1) {
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
            // clean_screen(p_tty->p_console);
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}

PUBLIC void task_tty1(){
    int i = 1;
    TTY*	p_tty;

    select_console(0);
    while (i) {
//        for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
//            clean_screen(p_tty->p_console);
//        }
        milli_delay(240000);
    }
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key)
{
    int raw_code = key & MASK_RAW;
    switch (MODE) {
        case NORMAL:
            if(raw_code == ESC){
                change_color();
                begin_search(p_tty->p_console);
                MODE = SEARCH;
                break;
            }
            input_char(p_tty, key);
            break;
        case SEARCH:
            switch (raw_code) {
                case ENTER:
                    search(p_tty->p_console);
                    MODE = FINISH_SEARCH;
                    break;
                case ESC:
                    change_color();
                    finish_search(p_tty->p_console);
                    MODE = NORMAL;
                    break;
                default:
                    input_char(p_tty, key);
                    ;
            }
        case FINISH_SEARCH:
            if(raw_code == ESC){
                change_color();
                MODE = NORMAL;
                finish_search(p_tty->p_console);
            }
            break;
        default:
            ;
    }



}

void input_char(TTY *p_tty, u32 key) {
    char output[2] = {'\0', '\0'};
    if (!(key & FLAG_EXT)) {
        if(key == FLAG_CTRL_L+'Z' || key == FLAG_CTRL_L+'z' || key == FLAG_CTRL_R+'Z' || key == FLAG_CTRL_R+'z'){
            recover(p_tty->p_console);
        } else{
            put_key(p_tty, key);
        }
    }
    else {
        int raw_code = key & MASK_RAW;
        switch(raw_code) {
            case ENTER:
                put_key(p_tty, '\n');
                break;
            case BACKSPACE:
                put_key(p_tty, '\b');
                break;
            case UP:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                    scroll_screen(p_tty->p_console, SCR_DN);
                }
                break;
            case DOWN:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                    scroll_screen(p_tty->p_console, SCR_UP);
                }
                break;
            case TAB:
                put_key(p_tty, '\t');
                break;
            case ESC:
//                change_color();
//                MODE = SEARCH;
                break;
            case F1:
            case F2:
            case F3:
            case F4:
            case F5:
            case F6:
            case F7:
            case F8:
            case F9:
            case F10:
            case F11:
            case F12:
                /* Alt + F1~F12 */
                if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
                    select_console(raw_code - F1);
                }
                break;
            default:
                break;
        }
    }
}

void change_color() {
    if(RED_COLOR_FLAG == 0x07){
        RED_COLOR_FLAG = 0x0c;
    } else RED_COLOR_FLAG = 0x07;
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}


