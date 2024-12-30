;-------------------------------------------------------------------------
;.586
;include \masm32\include\windows.inc
; V3
; RCX -> RAX -> R12 - rgbDataSegments[currentIdx] - pixel data pointer
; RDX -> R13        - segmentSize                 - the size of the vector a thread must go through
; R8                - width                       - the width of the image
; R9                - endPixelAddress             - pointer to the last pixel to be processed
.CODE
CompressionFuncCircus PROC

setup:
    push r12
    push r13
    push r14
    push rbx

    mov r12, rcx
    mov r13, rdx          
    ; r8 = width
    ; r9 = endPixelAddress

    xor rbx, rbx          ; rbx = 0 (loop counter)

    mov eax, 3120
    movd xmm11, eax
    pshufd xmm11, xmm11, 0

    mov eax, 255
    movd xmm5, eax
    pshufd xmm5, xmm5, 0

    mov r14, r8
    sub r14, 4

loop_pixels:
    cmp rbx, r13          
    jge end_loop

    lea rdi, [r12 + rbx * 4]  ; Current pixel address
    cmp rdi, r9
    jge end_loop

    ; Clear R, G, B accumulators
    pxor xmm1, xmm1
    pxor xmm2, xmm2
    pxor xmm3, xmm3

    ;           - (-2, -1), (-2, 0), (-2, 1)
    ; - (-1, -2), (-1, -1), (-1, 0), (-1, 1), (-1, 2)
    ; - ( 0, -2), ( 0, -1), (0, 0),  ( 0, 1), ( 0, 2)
    ; - ( 1, -2), ( 1, -1), ( 1, 0), ( 1, 1), ( 1, 2)
    ;           - ( 2, -1), ( 2, 0), ( 2, 1)

    ; Sąsiedzi według maski 5x5 bez rogów
    mov rax, -8
    push rax
    mov rax, -4
    push rax
    call PROCESS_NEIGHBOR

    mov rax, -8
    push rax
    mov rax, 0
    push rax
    call PROCESS_NEIGHBOR

    mov rax, -8
    push rax
    mov rax, 4
    push rax
    call PROCESS_NEIGHBOR

    mov rax, -4
    push rax
    mov rax, -8
    push rax
    call PROCESS_NEIGHBOR

    mov rax, -4
    push rax
    mov rax, -4
    push rax
    call PROCESS_NEIGHBOR

    mov rax, -4
    push rax
    mov rax, 0
    push rax
    call PROCESS_NEIGHBOR

    mov rax, -4
    push rax
    mov rax, 4
    push rax
    call PROCESS_NEIGHBOR

    mov rax, -4
    push rax
    mov rax, 8
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 0
    push rax
    mov rax, -8
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 0
    push rax
    mov rax, -4
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 0
    push rax
    mov rax, 0
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 0
    push rax
    mov rax, 4
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 0
    push rax
    mov rax, 8
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 4
    push rax
    mov rax, -8
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 4
    push rax
    mov rax, -4
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 4
    push rax
    mov rax, 0
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 4
    push rax
    mov rax, 4
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 4
    push rax
    mov rax, 8
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 8
    push rax
    mov rax, -4
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 8
    push rax
    mov rax, 0
    push rax
    call PROCESS_NEIGHBOR

    mov rax, 8
    push rax
    mov rax, 4
    push rax
    call PROCESS_NEIGHBOR

    pslld xmm5, 24

    ; Normalization
    pmulld xmm1, xmm11
    pmulld xmm2, xmm11
    pmulld xmm3, xmm11

    psrad xmm1, 16
    psrad xmm2, 16
    psrad xmm3, 16

    ; Glue the channels together
    pslld xmm1, 16
    pslld xmm2, 8 
    por xmm1, xmm2
    por xmm1, xmm3
    por xmm1, xmm5

    psrld xmm5, 24

    ; Save
    movdqu xmmword ptr [r12 + rbx * 4], xmm1

    ; Compare rbx to image width, and add another 4 if it's bigger than the width
    ; Do the countdown (decrement width by 4) and check if it's bigger than 0
    add rbx, 4
	sub r14, 4
    jg loop_pixels
    add rbx, 4            ; Add 4 to the pixel index to skip edges
    mov r14, r8           ; Reset width for the next line
    sub r14, 4
    jmp loop_pixels

end_loop:
    pop rbx
    pop r14
    pop r13
    pop r12
    ret

CompressionFuncCircus ENDP

; Procedura do przetwarzania jednego sąsiada
PROCESS_NEIGHBOR proc
    
    pop rax ; Back address
    pop rcx ; Horizontal movement
    pop r15 ; Vertical movement
    push rax

    mov rax, r15
    imul rax, r8          ; rax = vert * padded_width
    add rax, rcx        ; rax = (vert * padded_width) + horiz

    ; Adjust the current pixel address
    lea rdi, [r12 + rbx * 4] 
    add rdi, rax

    ; Load the neighbor pixel
    ; rdi = address of current pixel
    movdqu xmm4, xmmword ptr [rdi]
    
    movdqa xmm7, xmm4
    pand xmm7, xmm5 ; B
    paddd xmm3, xmm7  

    psrld xmm4, 8
    movdqa xmm7, xmm4
    pand xmm7, xmm5 ; G
    paddd xmm2, xmm7

    psrld xmm4, 8
    movdqa xmm7, xmm4
    pand xmm7, xmm5 ; R
    paddd xmm1, xmm7

    pxor xmm4, xmm4       ; Clear the register
    ret
PROCESS_NEIGHBOR endp

END
