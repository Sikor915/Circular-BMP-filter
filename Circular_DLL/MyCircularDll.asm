;-------------------------------------------------------------------------
;.586
;INCLUDE C:\masm32\include\windows.inc 

.CODE

; V1
CompressionFuncCircus PROC

    mov rax, rcx
    movq xmm0, r8
    mov r12, rdx

    pxor xmm1, xmm1
    pcmpeqb xmm1, xmm1
    psrld xmm1, 8

loopdaloop:
    movdqu xmm2, [rax]
    paddd xmm2, xmm0
    pand xmm2, xmm1
    movdqu [rax], xmm2

    add rax, 16
    sub r12, 4
    jnz loopdaloop

    ret

CompressionFuncCircus ENDP


END 			;no entry point