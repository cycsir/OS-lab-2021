
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


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

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

void draw_color(u8 *p_vmem, int len);

void finish_search(CONSOLE *p_con);

void begin_search(CONSOLE *p_con);

void init_search();

void recover(CONSOLE *p_con);

static void init_char();

void init_screen(TTY* p_tty);

extern int RED_COLOR_FLAG;
char outputs[1000];
int  out_endAdd[1000];
char searchInput[100];
char *p_char = &outputs[0];
int *p_int = &out_endAdd[0];
char *p_search = &searchInput[0];
char *p_search_begin;
char command[1000];
int command_len[1000];
char *p_command = &command[0];
int *p_commandLen = &command_len[0];


int MODE = NORMAL;


PRIVATE  void init_char() {
    out_endAdd[0] = 0;
    p_char = &outputs[0];
    p_int = &out_endAdd[0];
    outputs[100] = '\0';
}

void clean(CONSOLE* p_con)
{
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    while(p_con->cursor > p_con->original_addr){
        p_con->cursor--;
        *(p_vmem-2) = ' ';
        *(p_vmem-1) = DEFAULT_CHAR_COLOR;
        p_vmem -= 2;
    }
    init_search();
    init_char();
    flush(p_con);
}
//PRIVATE void reset_stay_time()
//{
//    stay_time = get_ticks();
//}
void clean_screen(CONSOLE *p_console){
    if(MODE == NORMAL){
        clean(p_console);
    }

//    if(((get_ticks() - stay_time) * 1000 / HZ) > 200000){
//        clean(p_console);
//    }

}
PUBLIC void begin_search(CONSOLE *p_con){
    init_search();
}

PUBLIC void search_char(char ch){
}

PUBLIC void search(CONSOLE* p_con){

    p_search = p_char; // 查找段结束
    int len_searchInput = p_search - p_search_begin; //  查找段长度
    int len_output = p_char - outputs - len_searchInput; // 被查找段长度

    if(len_output <= 0){
        return;
    }
    for (int k = 0; k < len_searchInput; k++) {
        searchInput[k] = *(p_search_begin + k);
    }
    int j = 0;
    int i = 0;
    while (j < len_searchInput && i < len_output){

        u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2 - (*(p_int-1) * 2)); // 定位到开始处
        if(outputs[i] == searchInput[j]){
            j++;
            i++;
        } else{
            j = 0;
            i++;
        }

        if (j == len_searchInput){
            j = 0;
            p_vmem = p_vmem + (out_endAdd[i-1]-len_searchInput)*2;
            draw_color(p_vmem, len_searchInput);
        }
    }
}

void finish_search(CONSOLE *p_con) {

    int len_searchInput = p_search - p_search_begin;
    int len_output = p_char - outputs - len_searchInput;
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    int last_cursor = p_con->cursor - len_searchInput;

    // 清空search状态下输入的内容，包括显示的和数组里的。
    while (p_con->cursor > last_cursor){
        p_con->cursor--;
        p_char--;
        p_int--;
        *(p_vmem-2) = ' ';
        *(p_vmem-1) = DEFAULT_CHAR_COLOR;
        p_vmem -= 2;

    }
    set_cursor(p_con->cursor);
    p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    int i = out_endAdd[len_output];
    while (i>0){
        i--;
        *(p_vmem-2);
        *(p_vmem-1) = DEFAULT_CHAR_COLOR;
        p_vmem -= 2;
    }
    init_search();

}

void init_search() {
    p_search = &searchInput[0];
    p_search_begin = p_char; // 查找段开始
    for (int i = 0; i < 100; ++i) {
        searchInput[i] = '\0';
    }
}

void draw_color(u8 *p_vmem, int len) {
    for (int i = 0; i < len; i++){
        *p_vmem++;
        *p_vmem++ = RED_COLOR_FLAG;
    }
}

void recover(CONSOLE *p_con){
    p_command--;
    char *current_p = p_command;
    char cd = *p_command;

    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    if(cd == '\b'){
        char re_ch = *current_p;
        int b_num = 0;
        while (current_p > &command[0]){  // 找到不是\b的来恢复
            current_p--;
            re_ch = *current_p;
            if(re_ch != '\b'){
                break;
            } else{
                b_num++;   // 连续的删除，123\b\b，目标字符是2
            }
        }
        while (b_num > 0){
            b_num --;
            current_p--;
        }
        re_ch = *current_p;
        *p_char = re_ch;
        p_char++;
        switch (re_ch) {
            case '\n':
                if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH) {
                    unsigned int offset = p_con->original_addr + SCREEN_WIDTH * ((p_con->cursor - p_con->original_addr) / SCREEN_WIDTH + 1);
                    p_con->cursor =offset;
                    *p_int = p_con->cursor;
                    p_int++;
                }
                break;
            case '\t':
                if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 4) {
                    for (int i = 0; i < 4; i++){
                        *p_vmem++ = ' ';
                        *p_vmem++ = RED_COLOR_FLAG;
                        p_con->cursor++;
                    }
                    *p_int = p_con->cursor;
                    p_int++;
                }
                break;
            default:
                if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) {
                    *p_vmem++ = re_ch;
                    *p_vmem++ = RED_COLOR_FLAG;
                    p_con->cursor++;
                    *p_int = p_con->cursor;
                    p_int++;
                    break;
                }
        }

    } else {
        if (p_con->cursor > p_con->original_addr) {
            p_int--;
            if(*p_int == 1 || *p_int == 0){
                p_con->cursor = 0;
            } else{
                p_con->cursor = p_con->cursor - (*p_int- *(p_int -1));
            }
//            p_con->cursor--;
            *(p_vmem - 2) = ' ';
            *(p_vmem - 1) = DEFAULT_CHAR_COLOR;

            p_char--;
            *p_char = '\0';
        }
    }
    *p_command = '\0';
    while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
        scroll_screen(p_con, SCR_DN);
    }
    flush(p_con);
}
/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
    int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
    init_char();
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

    switch(ch) {
	case '\n':
		if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH) {
            unsigned int offset = p_con->original_addr + SCREEN_WIDTH * ((p_con->cursor - p_con->original_addr) / SCREEN_WIDTH + 1);
            *p_commandLen = *p_int;
            p_con->cursor =offset;
            *p_int = p_con->cursor;
            p_int++;
            *p_char = ch;
            p_char++;
		}
		break;
    // 其他都为增加字符，只有遇到‘\b’需要删除字符
	case '\b':
		if (p_con->cursor > p_con->original_addr) {
            p_int--;
            if(*p_int == 1 || *p_int == 0){
                p_con->cursor = 0;
            } else{
                p_con->cursor = p_con->cursor - (*p_int- *(p_int -1));

            }
//            p_con->cursor--;
            *(p_vmem-2) = ' ';
            *(p_vmem-1) = DEFAULT_CHAR_COLOR;
            *p_commandLen = 0;
            p_char--;
            *p_char = '\0';
            *p_int = p_con->cursor;

		}
		break;
    case '\t':
        if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 4) {
            for (int i = 0; i < 4; i++){
            *p_vmem++ = ' ';
            *p_vmem++ = RED_COLOR_FLAG;
            p_con->cursor++;
            }
            *p_commandLen = 4;
            *p_int = p_con->cursor;
            p_int++;
            *p_char = ch;
            p_char++;
        }
        break;
	default:
		if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			*p_vmem++ = RED_COLOR_FLAG;
			p_con->cursor++;

            *p_command = 1;
            *p_int = p_con->cursor;
            p_int++;
            *p_char = ch;
            p_char++;
		}
        break;
	}
    *p_command = ch;
    p_command++;
    p_commandLen++;
	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}
//    reset_stay_time();
	flush(p_con);
}
/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

