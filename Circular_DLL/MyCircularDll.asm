;-------------------------------------------------------------------------
;.586
;include \masm32\include\windows.inc
; V3
; RCX -> RAX -> R12 - rgbDataSegments[currentIdx] - pointer to processed image data
; RDX -> R13        - rowsToProcess               - the number of rows to process
; R8                - width                       - the width of the image
; R9                - endPixelAddress             - pointer to the last pixel to be processed
; Stack -> RSI      - OGImageData[currentIdx]     - pointer to original image data
.CODE
CompressionFuncCircus PROC

ProcessNeigbor macro vert, horiz

    mov rcx, horiz
    mov rax, vert
    imul rax, r8          ; rax = vert * width
    add rax, rcx          ; rax = (vert * width) + horiz

    ; Adjust the current pixel address
    lea rdi, [RSI + rbx * 4] 
    add rdi, rax

    ; Load the neighbor pixel
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

    pxor xmm4, xmm4

endm

setup:
    mov rsi, qword ptr [rsp + 40]
    mov r12, rcx
    mov r13, rdx          

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
    ;cmp rbx, r13   ;compare with segmentSize
    ;jge end_loop

    cmp r13, 0
    jle end_procedure

    lea rdi, [RSI + rbx * 4]  ; Current pixel address
    cmp rdi, r9
    jge end_procedure

    ; Clear R, G, B accumulators
    pxor xmm1, xmm1
    pxor xmm2, xmm2
    pxor xmm3, xmm3

    ;            (-2, -1), (-2, 0), (-2, 1)
    ;  (-1, -2), (-1, -1), (-1, 0), (-1, 1), (-1, 2)
    ;  ( 0, -2), ( 0, -1), ( 0, 0), ( 0, 1), ( 0, 2)
    ;  ( 1, -2), ( 1, -1), ( 1, 0), ( 1, 1), ( 1, 2)
    ;            ( 2, -1), ( 2, 0), ( 2, 1)

    ; 5x5 Circular Filter (every index needs to be multiplied by 4)

    ProcessNeigbor -8, -4
    ProcessNeigbor -8, 0
    ProcessNeigbor -8, 4
    ProcessNeigbor -4, -8
    ProcessNeigbor -4, -4
    ProcessNeigbor -4, 0
    ProcessNeigbor -4, 4
    ProcessNeigbor -4, 8
    ProcessNeigbor 0, -8
    ProcessNeigbor 0, -4
    ProcessNeigbor 0, 0
    ProcessNeigbor 0, 4
    ProcessNeigbor 0, 8
    ProcessNeigbor 4, -8
    ProcessNeigbor 4, -4
    ProcessNeigbor 4, 0
    ProcessNeigbor 4, 4
    ProcessNeigbor 4, 8
    ProcessNeigbor 8, -4
    ProcessNeigbor 8, 0
    ProcessNeigbor 8, 4

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

    add rbx, 4
    sub r14, 4
    jg loop_pixels
    add rbx, 4            ; Add 4 to the pixel index to skip edges
    mov r14, r8           ; Reset width for the next line
    sub r14, 4
    sub r13, 1            ; Decrement the number of rows to process
    jmp loop_pixels

end_procedure:
    ret

CompressionFuncCircus ENDP

END
