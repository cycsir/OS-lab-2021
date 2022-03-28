; Hello World Program - asmtutor.com
; Compile with: nasm -f elf helloworld.asm
; Link with (64 bit systems require elf_i386 option): ld -m elf_i386 helloworld.o -o helloworld
; Run with: ./helloworld
 
%include    'functions.asm'
SECTION .data
msg     db      'Hello World!', 0Ah, 0h     ; assign msg variable with your message string
msg1    db      'Please input x and y : ', 0Ah, 0h
msg2    db      'x + y = ', 0Ah, 0h
msg3    db      'x * y = ', 0Ah, 0h 
msg4    db      'x = ', 0Ah, 0h
 
space   db      32

SECTION .bss
input:  resb    255
x:      resb    255
y:      resb    255
x_len:  resd    1
y_len:  resd    1
x_sign:  resb    1
y_sign:  resb    1
long_len: resd    1

sum:    resb    255
sum_len resd    1
zero:   resb    255

a:      resb    255
b:      resb    255
a_len:  resd    1
b_len:  resd    1

_sum: resb 255
_sum_len:   resd 1
_sum_sign:  resb 1

_:  resb 1
reverse_sum: resb 255
reverse_pro: resb 255


mul_sum: resb 255
mul_sum_len: resd 1
input_len resd  1
prd:    resb    255
pro_len: resd   1
pro_sign:       resb 1
z:      resb    1
SECTION .text
global  _start
 

_start:
    call    init
    ; mov     eax, msg
    ; call    sprint

    mov     eax, msg1
    call    sprint

    mov     eax, input
    mov     ebx, input_len
    call    sinput



    mov eax, input

    mov     eax, input
    call    sslen
    mov     dword[x_len], eax ;xlen


    
    mov     edx, eax ;
    mov     eax, input
    call    slen
    sub     eax, 1
    mov     dword[input_len], eax;      

    sub     eax, edx
    sub     eax, 1
    mov     dword[y_len], eax


;------------------------------
;for(int i = x_len; i >= 0; i--){}
;reverse

    mov     eax, x
    mov     ebx, input
    mov     ecx, dword[x_len]
    add     eax, ecx
    mov     byte[eax],0         ;x[x_len] = 0
    dec     eax



.reverse_x:

    cmp     ecx, 0 ;remain num
    jz      .finished_x

    push    ebx
    mov     bl,byte[ebx]
    mov     byte[eax], bl
    pop     ebx
    inc     ebx
    dec     eax
    dec     ecx
    jmp     .reverse_x


.finished_x:

    mov     eax, y
    mov     ebx, input
    mov     ecx, [x_len]
    add     ebx, ecx
    add     ebx, 1
    mov     edx, ebx        ;y[0]
    
    mov     ebx, input
    mov     ecx, [input_len]
    add     ebx, ecx
      
.reverse_y:
    cmp     ebx, edx
    jz      .finished_y
    dec     ebx
    mov     cl, byte[ebx]
    mov     byte[eax], cl
    inc     eax
    jmp     .reverse_y

.finished_y:

    mov     byte[eax], 0

    
    ; mov     eax, x
    ; call    sprintLF
    ; mov     eax, y
    ; call    sprintLF
 ;pos or neg
    mov     dl, '+'
    mov     byte[x_sign], dl
    mov     eax, x
    mov     ebx, [x_len]
    add     eax, ebx
    dec     eax
    cmp     byte[eax], '-'
    jnz      .x_pos
    mov     byte[eax], 0
    dec     ebx
    mov     dword[x_len], ebx
    mov     dl, '-'
    mov     byte[x_sign], dl
.x_pos:
    mov     dl, '+'
    mov     byte[y_sign], dl
    mov     eax, y
    mov     ebx, [y_len]
    add     eax, ebx
    dec     eax
    cmp     byte[eax], '-'
    jnz      .y_pos
    mov     byte[eax], 0
    dec     ebx
    mov     dword[y_len], ebx
    mov     dl, '-'
    mov     byte[y_sign], dl
.y_pos:
    ; mov     eax, x
    ; call    sprintLF
    ; mov     eax, y
    ; call    sprintLF

    mov     eax, [x_len]
    mov     ebx, [y_len]
    cmp     eax, ebx
    js      .y_len_long
    mov     dword[long_len], eax
    jmp     _Add
    .y_len_long:
    mov     dword[long_len], ebx    

_Add:
    mov     eax, [long_len]
    inc     eax
    mov     dword[long_len], eax
    ; call    iprintLF    
    cmp     byte[x_sign], '+'
    jz      .x_pos
    cmp     byte[y_sign], '+'
    jz      .x_neg_y_pos
    jmp     .x_neg_y_neg
    .x_pos:
    cmp     byte[y_sign], '+'
    jz      .x_pos_y_pos
    jmp     .x_pos_y_neg

; x>0, y>0
.x_pos_y_pos:
    mov     byte[_sum_sign], '+'
    mov     byte[pro_sign], '+'

    mov     esi, x 
    mov     edi, a 
    mov     ecx, [x_len]
    rep  movsb    
    ; push    eax
    ; mov     eax, a
    ; call    sprintLF
    ; pop     eax
    mov     esi, y 
    mov     edi, b 
    mov     ecx, [y_len]
    rep  movsb
    mov     esi, x_len
    mov     edi, a_len
    movsd
    mov     eax, dword[a_len]
    mov     esi, y_len
    mov     edi, b_len
    movsd    

    jmp     .finished_sign
; x<0, y<0
.x_neg_y_neg:
    mov     byte[_sum_sign], '-'
    mov     byte[pro_sign], '+'

    mov     esi, x 
    mov     edi, a 
    mov     ecx, [x_len]
    rep  movsb    
    ; push    eax
    ; mov     eax, a
    ; call    sprintLF
    ; pop     eax
    mov     esi, y 
    mov     edi, b 
    mov     ecx, [y_len]
    rep  movsb
    mov     esi, x_len
    mov     edi, a_len
    movsd
    mov     eax, dword[a_len]
    mov     esi, y_len
    mov     edi, b_len
    movsd

    jmp     .finished_sign
;x>0, y<0
.x_pos_y_neg:
    mov     byte[pro_sign], '-'

    mov     esi, y
    mov     edi, a
    mov     ecx, [y_len]
    rep   movsb
    mov     esi, long_len
    mov     edi, a_len
    movsd
    call    qufanjia1

    mov     esi, sum            ;b = sum = qufanjia1(y)
    mov     edi, b
    mov     ecx, [sum_len]
    rep  movsb
    mov     esi, sum_len
    mov     edi, b_len
    movsd
    
    mov     esi, x              ;a = x
    mov     edi, a 
    mov     ecx, [x_len]
    rep movsb
    mov     esi, x_len
    mov     edi, a_len
    movsd

    jmp     .finished_sign
;x<0, y>0
.x_neg_y_pos:
    mov     byte[pro_sign], '-'

    mov     esi, x
    mov     edi, a
    mov     ecx, [x_len]
    rep   movsb
    mov     esi, long_len
    mov     edi, a_len
    movsd
    call    qufanjia1

    mov     esi, sum            ;b = sum = qufanjia1(x)
    mov     edi, b
    mov     ecx, [sum_len]
    rep  movsb
    mov     esi, sum_len
    mov     edi, b_len
    movsd

    mov     esi, y              ;a = y
    mov     edi, a 
    mov     ecx, [y_len]
    rep movsb
    mov     esi, y_len
    mov     edi, a_len
    movsd

    jmp     .finished_sign
.finished_sign:
._Add:
    call    Add_x_y
    ; mov     eax, sum
    ; call    sprintLF

    mov     byte[_sum_sign], '+' 

    mov     eax, sum
    mov     ebx, [long_len]
    add     eax, ebx
    dec     eax
    cmp     byte[eax], '9'
    jz      .sum_neg            ;sum<0
    jmp     ._Mul

    .sum_neg:
    mov     byte[_sum_sign], '-'

    mov     esi, zero
    mov     edi, a
    mov     ecx, 255
    rep movsb
    mov     esi, sum
    mov     edi, a
    mov     ecx, [long_len]
    rep movsb
    mov     esi, long_len
    mov     edi, a_len
    movsd
    
    call    qufanjia1

._Mul:
    mov     esi, sum            ;_sum = qufanjia1(sum)
    mov     edi, _sum
    mov     ecx, [long_len]
    rep  movsb
    mov     esi, long_len
    mov     edi, _sum_len
    movsd

    mov     ecx, [_sum_len]  
    mov     eax, _sum
    mov     ebx, [_sum_len]
    add     eax, ebx
    dec     eax
.del_0:

    cmp     ecx, 1
    jz      .finished_del0
    dec     ecx
        
    mov     dl, byte[eax]
    cmp     dl, '0'
    jnz     .finished_del0
    mov     byte[eax], 0
    dec     eax
    dec     ebx
    jmp     .del_0
.finished_del0:
    mov     dword[_sum_len], ebx
    ; mov     eax,  _sum   
    ; call    sprintLF
    ; mov     eax, [_sum_len]
    ; call    iprintLF

    mov     esi, zero
    mov     edi, a
    mov     ecx, 255
    rep  movsb
    mov     esi, zero
    mov     edi, b
    mov     ecx, 255
    rep  movsb

    call    mul_x_y
    ; mov     eax, prd
    ; call    sprintLF
        ; mov     eax, prd
    ; call    sprintLF
    mov     eax, x
    cmp     byte[eax], 48
    jz      .x_0
    jmp     .x_no_0
    .x_0:
    
    mov     eax, [x_len]
    cmp     eax, 1
    jnz     .x_no_0
    mov     byte[prd], 48
    mov     dword[pro_len], 1
    mov     byte[pro_sign], '+'
    jmp     reverse

    .x_no_0:
    mov     eax, y
    cmp     byte[eax], 48
    jz      .y_0
    jmp     reverse
     .y_0:
    mov     eax, [y_len]
    cmp     eax, 1
    jnz     reverse
    
    mov     byte[prd], 48
    mov     dword[pro_len], 1
    mov     byte[pro_sign], '+'   
   
reverse:    
    ; mov     eax, [_sum_len]
    ; call    iprintLF
    
    mov     eax, _sum
    mov     ebx, [_sum_len]
    add     eax, ebx
    dec     eax
    cmp     byte[eax], 0
    jnz      .no_del_len
    dec     ebx
    mov     dword[_sum_len], ebx
.no_del_len:
    ; mov     eax, [_sum_len]
    ; call    iprintLF

    mov     eax, reverse_sum
    mov     ebx, _sum
    mov     ecx, dword[_sum_len]
    add     eax, ecx
    mov     byte[eax],0         ;x[x_len] = 0
    dec     eax

.reverse_sum:
    cmp     ecx, 0 ;remain num
    jz      .finished_sum

    push    ebx
    mov     bl,byte[ebx]
    mov     byte[eax], bl
    pop     ebx
    inc     ebx
    dec     eax
    dec     ecx

    jmp     .reverse_sum

.finished_sum:    
    ; mov     eax, reverse_sum
    ; call    sprintLF
  
    mov     eax, reverse_pro
    mov     ebx, prd
    mov     ecx, dword[pro_len]
    add     eax, ecx
    mov     byte[eax],0         ;x[x_len] = 0
    dec     eax

.reverse_pro:
    cmp     ecx, 0 ;remain num
    jz      .finished_pro

    push    ebx
    mov     bl,byte[ebx]
    mov     byte[eax], bl
    pop     ebx
    inc     ebx
    dec     eax
    dec     ecx
    jmp     .reverse_pro

.finished_pro:    
    ; mov     eax, reverse_pro
    ; call    sprintLF    
print:
    ;     push    eax
    ; mov     eax, reverse_sum
    ; call    sprintLF
    ; pop     eax
    .print_sum:
    mov    eax, _sum_sign
    cmp    byte[eax], '+'
    jz     .sum_pos
    call    sprint
    .sum_pos:
    mov    eax, reverse_sum
    call   sprintLF

    .print_pro:
    mov    eax, pro_sign
    cmp    byte[eax], '+'
    jz     .pro_pos
    call    sprint
    .pro_pos:
    mov    eax, reverse_pro
    call   sprintLF

    call   quit
;------------------------ --------
;x_y same sign, over 0
Add_x_y:
;------------------------------
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov     esi, zero
    mov     edi, sum
    mov     ecx, [sum_len]
    rep  movsb
    mov     edx, 0
    mov     dword[sum_len], edx       ;sum_len init: 0

    mov     eax, [a_len]
    mov     ebx, [b_len]       
    mov     ecx, eax
                
    cmp     eax, ebx          ;ecx = (x_len > y_len) ? x_len : y_len
    js      for_y_len

    mov     eax, a 
    mov     ebx, b
    

    mov     dh, 0 
for_len:
    cmp     ecx, 0                              ;for(;ecx>=0;ecx--)
    jz      finished_forlen

    mov     dl, byte[eax]
    add     dl, dh
    mov     dh, byte[ebx]
    add     dl, dh
    mov     dh, 0
    sub     dl, 48
    cmp     dl, 48
    jns     over0           ;if dl < 0
    add     dl, 48

over0: 
  

    cmp     dl, 58          ;if dl >= 10
    jc      no_over10
    sub     dl, 10
    mov     dh, 1
no_over10:
   
    push    eax
    push    ebx

    mov     eax, sum
    mov     ebx, [sum_len]
    add     eax, ebx
    mov     byte[eax], dl
    add     ebx, 1
    mov     dword[sum_len], ebx

    pop     ebx
    pop     eax
    inc     eax
    inc     ebx


    dec     ecx

    jmp     for_len 

finished_forlen:
    
    cmp     dh, 0
    jz      finished_add
    add     dh, 48
    push    eax
    push    ebx

    mov     eax, sum
    mov     ebx, [sum_len]
    add     eax, ebx
    mov     byte[eax], dh
    add     ebx, 1
    mov     dword[sum_len], ebx

    pop     ebx
    pop     eax  

finished_add:
    pop     edx
    pop     ebx
    pop     ecx
    pop     eax

    ret

for_y_len:

    mov     eax, a 
    mov     ebx, b
    mov     ecx, [b_len]
    mov     dh, 0 
    jmp     for_len

qufanjia1:
    push    edx
    push    ecx
    push    ebx
    push    eax

.init:


    mov     esi, zero               ; b = 0
    mov     edi, b 
    mov     ecx, 255
    rep  movsb

    mov     esi, a_len
    mov     edi, b_len
    movsd

    mov     ecx, [a_len]
    mov     eax, a
    mov     ebx, b
    

.for_len:


    cmp     ecx, 0
    jz      .finished_forlen
    dec     ecx
    mov     dl, byte[eax]
    cmp     dl, 0
    jz      .a_0

    cmp     dl, 53                  ;a<5?
    js     .small_5      
    mov     dh, 52                  
    sub     dl, 53                 ;a-5
    sub     dh, dl
    mov     byte[ebx], dh

    jmp     .finished_qufan

    .a_0:
    add     dl, 48
    .small_5:
    mov     dh, 52                  ;4
    sub     dh, dl                  ;4-a
    mov     dl, 53
    add     dl, dh
    mov     byte[ebx], dl



    .finished_qufan:
    inc     eax
    inc     ebx
    jmp     .for_len

.finished_forlen:
    ; push    eax
    ; mov     eax, b
    ; call    sprintLF
    ; pop     eax
    
    mov     esi, zero
    mov     edi, a
    mov     ecx, [a_len]
    rep  movsb

    mov     byte[a], 49
    mov     eax, 1
    mov     dword[a_len], eax
    call    Add_x_y
    
    ; push    eax
    ; mov     eax, sum
    ; call    sprintLF
    ; pop     eax

    pop     eax
    pop     ebx
    pop     ecx
    pop     edx
    
    ret

mul_x_y:    
    
    mov     esi, y                  ; args of add_a_y
    mov     edi, a                  ; a = y
    mov     ecx, [y_len]
    rep  movsb   

    mov     esi, zero               ; b = 0
    mov     edi, b 
    mov     ecx, 255
    rep  movsb
    
    mov     esi, y_len
    mov     edi, a_len
    movsd
    
    mov     edx, 0
    mov     dword[b_len], edx

    mov     eax, x                  ;from high to low
    mov     ebx, [x_len]
    add     eax, ebx
    dec     eax

    mov     ebx, y
    mov     ecx, [x_len]
    push    ecx
    
loop1:
    


    pop     ecx
    cmp     ecx, 0      ;ecx counts
    jz      finished_loop1

    dec     ecx
    push    ecx
    mov     ecx, 0
    mov     cl, byte[eax]
    sub     cl, 48
    push    ecx
    ;call    debug
loop2:
    
    pop     ecx
    cmp     cl, 0
    jz      finished_loop2

    dec     cl
    push    ecx

    call    Add_x_y


    mov     esi, sum               ;b = sum
    mov     edi, b 
    mov     ecx, [sum_len]
    rep  movsb
    mov     esi, sum_len
    mov     edi, b_len
    movsd

    jmp     loop2

finished_loop2:

    pop     ecx
    cmp     ecx, 0      ;ecx counts
    jz      finished_loop1
    push    ecx
    
    mov     ecx, [b_len]
    mov     edx, b 
    add     edx, ecx
    inc     edx
    mov     byte[edx], 0
    dec     edx

right_mov_loop:
    cmp     ecx, 0
    jz      finished_right_mov

    
    mov     edi, edx
    dec     edx
    mov     esi, edx
    movsb   
    dec     ecx
    jmp     right_mov_loop
    
finished_right_mov:


    mov     dl, 48
    mov     byte[b], dl
    mov     edx, [b_len]
    inc     edx
    mov     [b_len], edx

    dec     eax



    jmp     loop1
finished_loop1:
    mov     esi, b
    mov     edi, prd
    mov     ecx, [b_len]
    rep     movsb

    mov     esi, b_len
    mov     edi, pro_len
    movsd
    ret


    
debug:
    push eax
    push ebx
    push ecx
    push edx
    mov  ecx, eax
    mov  eax, msg
    call sprintLF
    mov  eax, ecx
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

init:
    push    eax
    push    ebx
    push    ecx
    push    edx

    mov     bl, 48
    mov     byte[zero], bl

    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    ret

