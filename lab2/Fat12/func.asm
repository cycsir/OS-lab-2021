;------------------------------------------
; int slen(String message)
; String length calculation function
global sprint

slen:                       ; this is our first function declaration
    push    ebx             ; push the value in EBX onto the stack to preserve it while we use EBX in this function
    mov     ebx, eax        ; move the address in EAX into EBX (Both point to the same segment in memory)

nextchar:                   ; this is the same as lesson3
    cmp     byte [eax], 0
    jz      finished
    inc     eax
    jmp     nextchar

finished:
    sub     eax, ebx
    pop     ebx             ; pop the value on the stack back into EBX
    ret

;------------------------------------------
; void sprint(String message)
; String printing function
sprint:
;    push    edx
;    push    ecx
;    push    ebx
;    push    eax
;    call    slen
;    mov     edx, eax    ;the msg len
;    pop     eax

    mov     eax, [esp+4]    ;
    mov     edx, [esp+8]    ; len

    mov     ecx, eax    ;the msg address
    mov     ebx, 1      ;stdout
    mov     eax, 4      ;invoke SYS_WRITE (kernel opcode 4)
    int     80h

;    pop     ebx
;    pop     ecx
;    pop     edx
    ret
;------------------------------------------
; void exit()
; Exit program and restore resources
quit:
    mov     ebx, 0
    mov     eax, 1
    int     80h
    ret


