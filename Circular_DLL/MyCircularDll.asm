;-------------------------------------------------------------------------
;.586
;include \masm32\include\windows.inc

.CODE

; V2
; RCX -> RAX -> R12 - rgbDataSegments[currentIdx] - pointer na info o pikselach
; RDX -> R13        - segmentSize                 - rozmiar vectora do przejœcia
; R8  -> R14        - width                       - szerokoœæ paska przez który prrzechodzi w¹tek
; R9  -> R15        - segmentHeight               - wysokoœæ paska przez który przechodzi w¹tek
CompressionFuncCircus PROC
;setup:
;    mov rax, rcx           ; This moves the pointer to the data into rax
;    mov r12, rax           ; This saves the OG pointer
;    mov r13, rdx           ; This moves the segmentSize into r12
;    movq xmm3, r8          ; This moves the width into xmm3
;    movq xmm0, r9          ; This moves the segmentHeight into xmm0
;
;    pxor xmm1, xmm1        ; This is a setup for the mask, probably not needed
;    pcmpeqb xmm1, xmm1
;    pslld xmm1, 24
;
;    mov r14, 16777215
;    movd xmm4, r14
;    mov r14, 9
;    movd xmm5, r14
;
;loopdaloop:
;    movdqu xmm2, [rax]
;    pand xmm2, xmm4
;    psrld xmm2, 16
;    movaps xmm6, xmm2
;    paddd xmm2, xmm6
;    paddd xmm2, xmm6
;    paddd xmm2, xmm6
;    paddd xmm2, xmm6
;    paddd xmm2, xmm6
;    paddd xmm2, xmm6
;
;    cvtdq2ps xmm2, xmm2
;    cvtdq2ps xmm5, xmm5
;
;    divps xmm2, xmm5
;
;    cvtps2dq xmm2, xmm2
;
;    movdqu [rax], xmm2
;
;    add rax, 16
;    sub r13, 4
;    jnz loopdaloop
;
;    ret

 setup:
    push r12
    push r13
    push r14
    push r15
    push rbx

    mov r12, rcx          ; r12 = pixelData
    mov r13, rdx          ; r13 = size
    mov r14, r8           ; r14 = width (in pixels)
    mov r15, r9           ; r15 = height (in pixels)

    xor rbx, rbx          ; rbx = 0 (loop counter)

    ; Precompute offsets
    imul r8, r14, 4       ; r8 = row_offset = width * 4 (for one row)

    push rax

    mov eax, 16711680     ; Load the R mask (0xFF0000)
    movd xmm8, eax
    pshufd xmm8, xmm8, 0

    mov eax, 65280        ; Load the G mask (0x00FF00)
    movd xmm9, eax
    pshufd xmm9, xmm9, 0

    mov eax, 255          ; Load the B mask (0x0000FF)
    movd xmm10, eax
    pshufd xmm10, xmm10, 0

    pop rax

loop_pixels:
    cmp rbx, r13          ; If counter >= size, exit loop
    jge end_loop

    ; Calculate current pixel's row and column
    mov rax, rbx
    xor rdx, rdx          ; Clear remainder
    div r8                ; rax = row, rdx = column
    mov rdi, rax          ; rdi = row
    mov rsi, rdx          ; rsi = column

    ; Accumulators for R, G, B (as floats)
    pxor xmm1, xmm1       ; Red accumulator
    pxor xmm2, xmm2       ; Green accumulator
    pxor xmm3, xmm3       ; Blue accumulator
    xor r10, r10          ; Filter weight counter
    xor rcx, rcx

    ; Neighbor loop (-2 to +2 in both directions)
    mov rax, -2           ; Row offset (-2)
row_loop:
    mov rdx, -2           ; Column offset (-2)
col_loop:
    
    ; Calculate neighbor position
    lea r9, [rdi + rax]   ; neighborRow = row + offset
    lea r10, [rsi + rdx]  ; neighborCol = col + offset

    ; Boundary checks
    cmp r9, 0
    jl skip_pixel
    cmp r9, r15
    jge skip_pixel
    cmp r10, 0
    jl skip_pixel
    cmp r10, r8
    jge skip_pixel

    ; Calculate flat array offset for the neighbor
    mov r11, r9            ; Copy neighborRow to r11
    imul r11, r8           ; r11 = neighborRow * row_offset
    lea r11, [r11 + r10 * 4] ; + neighborCol * 4
    add r11, r12           ; Add base pixelData pointer

    ; Load neighbor pixel
    movd xmm4, dword ptr [r11]       ; Load one pixel into xmm4

    ; Extract R, G, B
    movaps xmm5, xmm4      ; Copy pixel data to xmm5
    pand xmm5, xmm8        ; Mask R
    psrld xmm5, 16         ; Shift R to lower bits
    cvtdq2ps xmm5, xmm5    ; Convert R to float
    addps xmm1, xmm5       ; Add to R accumulator

    movaps xmm5, xmm4      ; Copy pixel data to xmm5
    pand xmm5, xmm9        ; Mask G
    psrld xmm5, 8          ; Shift G to lower bits
    cvtdq2ps xmm5, xmm5    ; Convert G to float
    addps xmm2, xmm5       ; Add to G accumulator

    movaps xmm5, xmm4      ; Copy pixel data to xmm5
    pand xmm5, xmm10       ; Mask B
    cvtdq2ps xmm5, xmm5    ; Convert B to float
    addps xmm3, xmm5       ; Add to B accumulator

    inc rcx                ; Increment filter weight

skip_pixel:
    add rdx, 1             ; Next column
    cmp rdx, 3
    jl col_loop

    add rax, 1             ; Next row
    cmp rax, 3
    jl row_loop

    ; Normalize RGB accumulators
    movd xmm5, rcx         ; Move filter weight into xmm5
    cvtdq2ps xmm5, xmm5    ; Convert to float
    divps xmm1, xmm5       ; Normalize R
    divps xmm2, xmm5       ; Normalize G
    divps xmm3, xmm5       ; Normalize B

	cvtps2dq xmm1, xmm1    ; Convert R to int
	cvtps2dq xmm2, xmm2    ; Convert G to int
	cvtps2dq xmm3, xmm3    ; Convert B to int

    ; Combine normalized RGB and write back
    pslld xmm1, 16         ; Align R
    pslld xmm2, 8          ; Align G
    por xmm1, xmm2
    por xmm1, xmm3         ; Combine RGB
    movd dword ptr [r12 + rbx * 4], xmm1 ; Write to newPixelData

    add rbx, 1             ; Process next pixel
    jmp loop_pixels

end_loop:
    pop rbx
    pop r15
    pop r14
    pop r13
    pop r12
    ret

CompressionFuncCircus ENDP


END