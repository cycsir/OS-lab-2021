
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_my_sleep        equ 1;
_NR_my_sprint       equ 2;
_NR_my_p            equ 3;
_NR_my_v            equ 4;

INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global  my_sleep
global  my_sprint
global  my_p
global  my_v

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

;
my_sleep:
    mov eax, _NR_my_sleep
    mov ecx, [esp+4]
    int INT_VECTOR_SYS_CALL
    ret

my_sprint:
    mov eax, _NR_my_sprint
    mov ecx, [esp+4]
    int INT_VECTOR_SYS_CALL
    ret

my_p:
    mov eax, _NR_my_p
    mov ecx, [esp+4]
    int INT_VECTOR_SYS_CALL
    ret

my_v:
    mov eax, _NR_my_v
    mov ecx, [esp+4]
    int INT_VECTOR_SYS_CALL
    ret
