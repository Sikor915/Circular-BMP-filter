;-------------------------------------------------------------------------
;.586
;include \masm32\include\windows.inc
.DATA
inv_21 dd 0.0476190

.CODE

; V3
; RCX -> RAX -> R12 - rgbDataSegments[currentIdx] - pointer na info o pikselach
; RDX -> R13        - segmentSize                 - rozmiar vectora do przejścia
; R8                - width                       - szerokość paska przez który prrzechodzi wątek
; R9                - endPixelAddress             - pointer na ostatni piksel który musi być przetworzony
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

    xor rbx, rbx          ; rbx = 0 (licznik pętli)

	movaps xmm11, [inv_21]
    pshufd xmm11, xmm11, 0

    mov eax, 255
    movd xmm5, eax
    pshufd xmm5, xmm5, 0

    mov r14, r8
    sub r14, 4

loop_pixels:
    cmp rbx, r13          
    jge end_loop

    lea rdi, [r12 + rbx * 4]  ; Pełny adres bieżącego piksela
    cmp rdi, r9
    jge end_loop

    ; Wyczyszczenie akumulatorów R, G, B
    pxor xmm1, xmm1
    pxor xmm2, xmm2
    pxor xmm3, xmm3

    ; Lista sąsiadów w 5x5 (bez rogów):
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

    ; Przetwarzanie zakończone, normalizacja
    cvtdq2ps xmm1, xmm1   ; Konwersja R na float
    mulps xmm1, xmm11    ; Normalizacja

	cvtdq2ps xmm2, xmm2   ; Konwersja G na float
    mulps xmm2, xmm11    ; NNormalizacja
    
	cvtdq2ps xmm3, xmm3   ; Konwersja B na float
    mulps xmm3, xmm11    ; Normalizacja
    

    cvtps2dq xmm1, xmm1   ; Konwersja R na int
	cvtps2dq xmm2, xmm2   ; Konwersja G na int
	cvtps2dq xmm3, xmm3   ; Konwersja B na int

    ; Połączenie RGB
    pslld xmm1, 16
    pslld xmm2, 8 
    por xmm1, xmm2
    por xmm1, xmm3
    por xmm1, xmm5

    psrld xmm5, 24

    ; Zapisanie wyniku
    movdqu xmmword ptr [r12 + rbx * 4], xmm1

    add rbx, 4
    ; Compare rbx to image width, and add another 4 if it's bigger than the width
    ; Do the countdown (decrement width by 4) and check if it's bigger than 0
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
