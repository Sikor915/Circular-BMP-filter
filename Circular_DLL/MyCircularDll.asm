;-------------------------------------------------------------------------
;.586
;include \masm32\include\windows.inc
.DATA
inv_25 dd 0.04

.CODE

; V2
; RCX -> RAX -> R12 - rgbDataSegments[currentIdx] - pointer na info o pikselach
; RDX -> R13        - segmentSize                 - rozmiar vectora do przejścia
; R8  -> R14        - width                       - szerokość paska przez który prrzechodzi wątek
; R9  -> R15        - segmentHeight               - wysokość paska przez który przechodzi wątek
CompressionFuncCircus PROC

setup:
    push r12
    push r13
    push rbx

    mov r12, rcx          ; r12 = pixelData
    mov r13, rdx          ; r13 = size
    ; r8 = width
    ; r9 = segmentHeight

    xor rbx, rbx          ; rbx = 0 (licznik pętli)

    ; Predefiniowana odwrotność dla filtra (1/25 = 0.04)
    ;mov rax, 85899346   ; 1/25 zapisane jako fixed-point 32-bit (Q31)
    ;movd xmm11, eax
    ;pshufd xmm11, xmm11, 0

	movaps xmm11, [inv_25]
    ;cvttps2dq xmm11, xmm11

    mov eax, 255
    movd xmm5, eax
    

loop_pixels:
    xor r14, r14          ; Zerowanie licznika pikseli
    cmp rbx, r13          ; Jeśli licznik >= rozmiar, zakończ pętlę
    jge end_loop

    ; Wyczyszczenie akumulatorów
    pxor xmm1, xmm1       ; Akumulator R
    pxor xmm2, xmm2       ; Akumulator G
    pxor xmm3, xmm3       ; Akumulator B

    ; Oblicz płaski offset bieżącego piksela
    mov rdi, rbx          ; Aktualny indeks piksela
    imul rdi, 4           ; rdi *= 4 (ARGB = 4 bajty)
    lea rdi, [r12 + rdi]  ; Pełny adres bieżącego piksela

    ; Lista sąsiadów w 5x5 (bez rogów):
    ;           - (-2, -1), (-2, 0), (-2, 1)
    ; - (-1, -2), (-1, -1), (-1, 0), (-1, 1), (-1, 2)
    ; - ( 0, -2), ( 0, -1), (0, 0),  ( 0, 1), ( 0, 2)
    ; - ( 1, -2), ( 1, -1), ( 1, 0), ( 1, 1), ( 1, 2)
    ;           - ( 2, -1), ( 2, 0), ( 2, 1)

    ; Sąsiedzi według maski 5x5 bez rogów
    push -2
    push -1
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push -2
    push 0
    call PROCESS_NEIGHBOR
    pop rax   
    pop rax

    push -2
    push 1
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push -1
    push -2
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push -1
    push -1
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push -1
    push 0
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push -1
    push 1
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push -1
    push 2
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 0
    push -2
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 0
    push -1
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 0
    push 0
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 0
    push 1
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 0
    push 2
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 1
    push -2
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 1
    push -1
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 1
    push 0
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 1
    push 1
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 1
    push 2
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 2
    push -1
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 2
    push 0
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    push 2
    push 1
    call PROCESS_NEIGHBOR
    pop rax
    pop rax

    pslld xmm5, 24
    cvtsi2ss xmm11, r14
    rcpss xmm11, xmm11

    ; Przetwarzanie zakończone, normalizacja
    cvtdq2ps xmm1, xmm1   ; Konwertuj R na float
    mulps xmm1, xmm11    ; Normalizuj R
    ;psrad xmm1, 31
	cvtdq2ps xmm2, xmm2   ; Konwertuj G na float
    mulps xmm2, xmm11    ; Normalizuj G
    ;psrad xmm2, 31
	cvtdq2ps xmm3, xmm3   ; Konwertuj B na float
    mulps xmm3, xmm11    ; Normalizuj B
    ;psrad xmm3, 31

    cvtps2dq xmm1, xmm1   ; Konwertuj R z powrotem na int
	cvtps2dq xmm2, xmm2   ; Konwertuj G z powrotem na int
	cvtps2dq xmm3, xmm3   ; Konwertuj B z powrotem na int

    ; Połącz RGB
    pslld xmm1, 16        ; Przesuń R na swoje miejsce
    pslld xmm2, 8         ; Przesuń G na swoje miejsce
    por xmm1, xmm2
    por xmm1, xmm3
    por xmm1, xmm5

    psrld xmm5, 24

    ; Zapisz wynik
    movd dword ptr [r12 + rbx * 4], xmm1

skip_pixel:
    add rbx, 1            ; Przejdź do następnego piksela
    jmp loop_pixels

end_loop:
    pop rbx
    pop r13
    pop r12
    ret


CompressionFuncCircus ENDP

; Procedura do przetwarzania jednego sąsiada
PROCESS_NEIGHBOR proc horiz, vert
    ; Calculate row and column for the current pixel
    mov rax, rbx          ; rbx = current pixel index
    xor rdx, rdx          ; Clear remainder register
    div r8                ; rax = current row, rdx = current column
    mov rdi, rax          ; rdi = current row
    mov rsi, rdx          ; rsi = current column

    ; Add offsets to row and column
    add edi, vert         ; Adjust row by vertical offset
    add esi, horiz        ; Adjust column by horizontal offset

    ; Boundary checks
    cmp rdi, 0            ; Row >= 0
    jl skip
    cmp rdi, r9           ; Row < height
    jge skip
    cmp rsi, 0            ; Column >= 0
    jl skip
    cmp rsi, r8           ; Column < width
    jge skip

    ; Convert row/column back to flat index
    imul rdi, r8          ; rdi *= width
    add rdi, rsi          ; Flat index = row * width + column

    ; Load neighbor pixel
    imul rdi, 4           ; rdi *= 4 (ARGB = 4 bytes)
    lea rdi, [r12 + rdi]  ; Address of neighbor pixel
    movd xmm4, dword ptr [rdi]
    
    movdqa xmm7, xmm4
    pand xmm7, xmm5 ;B
    paddd xmm3, xmm7  

    psrld xmm4, 8
    movdqa xmm7, xmm4
    pand xmm7, xmm5 ;G
    paddd xmm2, xmm7

    psrld xmm4, 8
    movdqa xmm7, xmm4
    pand xmm7, xmm5 ;R
    paddd xmm1, xmm7
    
    inc r14
skip:
    ret
PROCESS_NEIGHBOR endp

END