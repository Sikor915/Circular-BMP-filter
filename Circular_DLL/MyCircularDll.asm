;.DATA
;inv_25 dd 0.04             ; Inverse of 25 for normalization

;.CODE
; V4
;CompressionFuncCircus PROC
    ; RCX -> RAX -> R12 - pointer to pixel data
    ; RDX -> R13        - segment size (number of pixels to process)
    ; R8                - width (image width)
    ; R9                - segmentHeight (height of the image segment)

    ;AKTUALNE BŁEDY:
    ;1. Nie działa przy braniu drugiego sąsiada z siatki. Dostaję błąd "Access violation reading location."
    ;   Trzeba poprawić wykrozystywanie width, bo z height nie korzystam więc mogę mieć 
    ;   i widthOG (pilnowanie końca wiersza) i paddedWidth (obliczanie adresu somsiada)
    ;2. Należy sprawdzić czy działa skipowanie paddingu przy końcu wiersza.
    ;3. Należy sprawdzić poprawność działania algorytmu.

;setup:
    ;push r12
    ;push r13
    ;push r14
    ;push rbx

    ;mov rax, r8
    ;shl rax, 1
    ;add rax, 10
    ;shl rax, 2
    ;add rcx, rax          ; Skip padding at the beginning of the segment

    ;mov r12, rcx          ; Pointer to pixel data
    ;mov r13, rdx          ; Segment size
    ;xor rbx, rbx          ; Pixel index counter
    ;movaps xmm11, [inv_25] ; Preload normalization factor
    ;mov r14, r8           ; Image width

    ;mov eax, 255          ; Mask for extracting color components
    ;movd xmm5, eax
    ;pshufd xmm5, xmm5, 0

;loop_pixels:
    ;cmp rbx, r13          ; Check if we've processed all pixels in the segment
    ;jge end_loop

    ; Load the 5x5 region for the current pixel
    ; Skip boundary checks due to padding, start directly at the valid area
    ;mov rdi, rbx
    ;imul rdi, 16          ; rdi = rbx * 4 pixels * 4 bytes/pixel
    ;lea rdi, [r12 + rdi]  ; Address of the first pixel in the batch

    ;pxor xmm1, xmm1       ; Clear accumulator for red
    ;pxor xmm2, xmm2       ; Clear accumulator for green
    ;pxor xmm3, xmm3       ; Clear accumulator for blue

    ; Loop through the 5x5 neighborhood
    ;mov rsi, -2
;filter_rows:
    ;mov rdx, -2
;filter_cols:
    ; Calculate the address of the neighbor pixel
    ;mov rax, rsi
    ;imul rax, r8          ; Row offset = row index * width
    ;add rax, rdx          ; Add column offset
    ;imul rax, 4           ; Scale by 4 bytes per pixel
    ;add rax, rdi          ; Final address of the neighbor

    ; Load neighbor pixel and extract RGB components
    ;movdqa xmm0, xmmword ptr [rax]
    ;movdqa xmm4, xmm0     ; Copy pixel for processing

    ; Blue channel
    ;pand xmm4, xmm5       ; Extract blue
    ;paddd xmm3, xmm4      ; Accumulate blue

    ; Green channel
    ;movdqa xmm4, xmm0
    ;pslld xmm5, 8
    ;pand xmm4, xmm5       ; Extract green
    ;paddd xmm2, xmm4      ; Accumulate green

    ; Red channel
    ;movdqa xmm4, xmm0
    ;pslld xmm5, 8
    ;pand xmm4, xmm5       ; Extract red
    ;paddd xmm1, xmm4      ; Accumulate red

    ;psrld xmm5, 16 	      ; Reset mask for next iteration
    
    ; Move to next neighbor column
    ;add rdx, 1
    ;cmp rdx, 3            ; Process columns -2 to +2
    ;jl filter_cols

    ; Move to next neighbor row
    ;add rsi, 1
    ;cmp rsi, 3            ; Process rows -2 to +2
    ;jl filter_rows

    ; Normalize accumulated values
    ;cvtdq2ps xmm1, xmm1   ; Convert red to float
    ;mulps xmm1, xmm11     ; Normalize red
    ;cvtps2dq xmm1, xmm1   ; Convert back to int

    ;cvtdq2ps xmm2, xmm2   ; Convert green to float
    ;mulps xmm2, xmm11     ; Normalize green
    ;cvtps2dq xmm2, xmm2   ; Convert back to int

    ;cvtdq2ps xmm3, xmm3   ; Convert blue to float
    ;mulps xmm3, xmm11     ; Normalize blue
    ;cvtps2dq xmm3, xmm3   ; Convert back to int

    ; Combine normalized color components
    ;pslld xmm1, 16        ; Shift red to its position
    ;pslld xmm2, 8         ; Shift green to its position
    ;por xmm1, xmm2        ; Combine red and green
    ;por xmm1, xmm3        ; Combine blue with red and green

    ; Write back the processed pixel
    ;movdqa xmmword ptr [rdi], xmm1

    ; Increment pixel index (4 pixels processed at once)
    ;add rbx, 4
    ; Compare rbx to image width, and add another 4 if it's bigger than the width
    ; Do the countdown (decrement width by 4) and check if it's bigger than 0
	;sub r14, 4
    ;jg loop_pixels
    ;add rbx, 4            ; Add 4 to the pixel index to skip padding
    ;mov r14, r8           ; Reset width for the next line
    ;jmp loop_pixels

;end_loop:
    ;pop rbx
    ;pop r14
    ;pop r13
    ;pop r12
    ;ret


;CompressionFuncCircus ENDP

;END


;-------------------------------------------------------------------------
;.586
;include \masm32\include\windows.inc
.DATA
inv_25 dd 0.04

.CODE

; V3
; RCX -> RAX -> R12 - rgbDataSegments[currentIdx] - pointer na info o pikselach
; RDX -> R13        - segmentSize                 - rozmiar vectora do przejścia
; R8                - width                       - szerokość paska przez który prrzechodzi wątek
; R9                - segmentHeight               - wysokość paska przez który przechodzi wątek
CompressionFuncCircus PROC

setup:
    push r12
    push r13
    push r14
    push rbx

    mov rax, r8
    sub rax, 4
    shl rax, 1
    add rax, 10
    shl rax, 2
    add rcx, rax          ; Skip padding at the beginning of the segment

    mov r12, rcx          
    mov r13, rdx          
    ; r8 = width (padded)
    ; r9 = segmentHeight

    xor rbx, rbx          ; rbx = 0 (licznik pętli)

	movaps xmm11, [inv_25]
    pshufd xmm11, xmm11, 0

    mov eax, 255
    movd xmm5, eax
    pshufd xmm5, xmm5, 0

    mov r14, r8
    sub r14, 4
    
 ; Można to zrobić tak aby przechodziło po 16 składowych naraz (słowa 16-bitowe, 4 piksele naraz), i tylko przy 
 ; końcu wiersza trochę na piechotę lecieć. Tak samo nie jest potrzebny float i wywoływanie
 ; procedury. Dalej zrobić to na intach, mnożenie przez odwrotność itd. itp. 
 ; Algorytm jest niezależny dla każdego komponentu RGB.
 ; DODAĆ WEKTOROWOŚĆ DO KURWY
 ; Nie wiem jak on chce bym np. to robił na krawędziach kiedy może zajść akcja, że np. jeden sąsiad
 ; dla jednego pikselu nie jest w obrazku (więc nie brany pod uwagę), ale ten sam offset dla piksela obok
 ; już da mi poprawnego sąsiada którego musiałbym przetworzyć. 
 ; 1. Mogę dodać czarne piksele na krawędziach by zawsze mieć 21 sąsiadów przetworzonych. Problem jak się ich pozbyć
 ; 2. Mogę skipować do momentu gdy nie będę miał pełnej maski dla każdego z pikseli aktualnie przetwarzanych.
 ;    Okropne według mnie, algorytm nie będzie wtedy w pełni kompletny i takie sprawdzanie też chujowo będzie wyglądać
 ; 3. Jeżeli jest poza skalą to w jakiś sposób tylko tą część zmaskować przez 0x00000000 (nie brać pod uwagę kanałów, ale sam piksel tak)

loop_pixels:
    cmp rbx, r13          
    jge end_loop

    ; Wyczyszczenie akumulatorów R, G, B
    pxor xmm1, xmm1
    pxor xmm2, xmm2
    pxor xmm3, xmm3

    lea rdi, [r12 + rbx * 4]  ; Pełny adres bieżącego piksela

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
    ;pop rax
    ;pop rax

    mov rax, -8
    push rax
    mov rax, 0
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, -8
    push rax
    mov rax, 4
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, -4
    push rax
    mov rax, -8
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, -4
    push rax
    mov rax, -4
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, -4
    push rax
    mov rax, 0
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, -4
    push rax
    mov rax, 4
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, -4
    push rax
    mov rax, 8
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 0
    push rax
    mov rax, -8
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 0
    push rax
    mov rax, -4
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 0
    push rax
    mov rax, 0
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 0
    push rax
    mov rax, 4
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 0
    push rax
    mov rax, 8
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 4
    push rax
    mov rax, -8
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 4
    push rax
    mov rax, -4
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 4
    push rax
    mov rax, 0
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 4
    push rax
    mov rax, 4
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 4
    push rax
    mov rax, 8
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 8
    push rax
    mov rax, -4
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 8
    push rax
    mov rax, 0
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    mov rax, 8
    push rax
    mov rax, 4
    push rax
    call PROCESS_NEIGHBOR
    ;pop rax
    ;pop rax

    pslld xmm5, 24

    ; Tu się psuje
    ;cvtps2dq xmm11, xmm11
    ; Przetwarzanie zakończone, normalizacja
    cvtdq2ps xmm1, xmm1   ; Konwertuj R na float
    mulps xmm1, xmm11    ; Normalizuj R

	cvtdq2ps xmm2, xmm2   ; Konwertuj G na float
    mulps xmm2, xmm11    ; Normalizuj G
    
	cvtdq2ps xmm3, xmm3   ; Konwertuj B na float
    mulps xmm3, xmm11    ; Normalizuj B
    

    cvtps2dq xmm1, xmm1   ; Konwertuj R z powrotem na int
	cvtps2dq xmm2, xmm2   ; Konwertuj G z powrotem na int
	cvtps2dq xmm3, xmm3   ; Konwertuj B z powrotem na int

    ; Połącz RGB
    pslld xmm1, 16
    pslld xmm2, 8 
    por xmm1, xmm2
    por xmm1, xmm3
    por xmm1, xmm5

    psrld xmm5, 24

    ; Zapisz wynik
    movdqu xmmword ptr [r12 + rbx * 4], xmm1

    ; Increment pixel index (4 pixels processed at once)
    add rbx, 4
    ; Compare rbx to image width, and add another 4 if it's bigger than the width
    ; Do the countdown (decrement width by 4) and check if it's bigger than 0
	sub r14, 4
    jg loop_pixels
    add rbx, 4            ; Add 4 to the pixel index to skip padding
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
PROCESS_NEIGHBOR proc ;horiz, vert
    
    pop rax ; Back address
    pop rcx ; Horizontal movement
    pop r15 ; Vertical movement
    push rax

    mov rax, r15
    imul rax, r8          ; rax = vert * padded_width
    add rax, rcx        ; rax = (vert * padded_width) + horiz

    ; Adjust the current pixel address
    lea rdi, [r12 + rbx * 4]  ; rdi = address of current pixel
    add rdi, rax          ; Add flat index adjustment to get neighbor address
    ;imul rdi, 4           ; Scale by 4 bytes per pixel

    ; WARUM IS THE EXCEPTION THROWN HERE
    ; Load the neighbor pixel
    movdqu xmm4, xmmword ptr [rdi]
    
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

    pxor xmm4, xmm4       ; Clear the register
    ret
PROCESS_NEIGHBOR endp

END
