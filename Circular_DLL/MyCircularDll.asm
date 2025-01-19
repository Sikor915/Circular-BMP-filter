; Temat: Filtr dolnoprzepustowy kolowy
; Algorytm ten usrednia wartosci pikseli bazujac na macierzy sasiadow 5x5, tworzac efekt rozmycia.
; Ta implementacja pomija piksele na krawedziach obrazu, co powoduje powstanie "okna".
; Data wykonania: 2025-01-14
; Autor: Kacper Sikorski, INF/5 semestr
; --------------------------------------------------------------------------------------------------------------
; | Wersja: 1.0 - 2024.12.03                                                                                   |
; | W wersji 1.0 algorytm dziala dobrze gdy ilosc wybranych watkow jest wieksza od 16.                         |
; | Przekazuje wskaznik do pikseli, rozmiar vectora do przejsca i wymiary paska do przejscia.                  |
; | Wykorzystuje potrojna petle aby przejsc przez wszystkich sasiadow wszystkich pikseli                       |
; | i odpowiednio unikajac danych poza obrazem.                                                                |
; --------------------------------------------------------------------------------------------------------------
; | Wersja 2.0 - 2024.12.09                                                                                    |
; | W wersji 2.0 algorytm dziala poprawnie dla dowolnych obrazow .bmp i ilosci watkow.                         |
; | Pozbylem sie potrojnej petli, zastepujac ja wywolywaniem procedury odpowiedzialnej                         |
; | za sumowanie kanalow RGB sasiadow aktualnie przetwarzanego piksela.                                        |
; | W tej wersji, rejestry xmm sa jedynie wykorzystywane jako dodatkowe miejsce.                               |
; | Nie ma tutaj przetwarzania wektorowego.                                                                    |
; --------------------------------------------------------------------------------------------------------------
; | Wersja 3.0 - 2024.12.22                                                                                    |
; | W wersji 3.0 algorytm stosuje przetwarzanie wektorowe, poprzez prace na 4 pikselach naraz.                 |
; | Dodatkowo, w tej wersji zamiast przerabiac caly obraz, algorytm nie przetwarza pikseli na krawedziach.     |
; | Zamiast przekazywac wysokosc obrazu, przekazuje adres do ostatniego piksela do przetworzenia,              |
; | aby algorytm w odpowiednim miejscu sie skonczyl.                                                           |
; --------------------------------------------------------------------------------------------------------------
; | Wersja 3.1 - 2024.12.30                                                                                    |
; | W wersji 3.1 algorytm nie wykorzystuje konwersji na float oraz dzielenia, lecz wszystko odbywa sie         |
; | na intach. Dodatkowo zamienilem wywolywanie funkcji na makro, co przyspiesza dzialanie programu.           |
; --------------------------------------------------------------------------------------------------------------
; | Wersja 3.2 - 2025.01.04                                                                                    |
; | W wersji 3.2 zmienilem jeden z przekazywanych argumentow, by algorytm przetwarzal okreslona ilosc wierszy  |
; | a nie rozmiar vectora, co usuwa blad kiedy szerokosc obrazu jest wieksza od rozmiaru vectora do przejscia. |
; --------------------------------------------------------------------------------------------------------------
; | Wersja 3.3 - 2025.01.14                                                                                    |
; | W wersji 3.3 zamiast pracowac na jednej kopii obrazu, algorytm pobiera dane z jednego vectora i po         |
; | przetworzeniu zapisuje wynik do innego vectora. Dzieki temu wynik jego pracy powinien zawsze byc taki sam. |
; --------------------------------------------------------------------------------------------------------------
; | Wersja 3.3.1 - 2025.01.19                                                                                  |
; | W wersji 3.3.1 zrobilem szybka poprawke z dzialaniami na stosie                                            |
; --------------------------------------------------------------------------------------------------------------
;.586
;include \masm32\include\windows.inc
; V3.3.1
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
    push r12
    push r13
    push r14
    push rsi
    push rdi
    push rbx

    mov rsi, qword ptr [rsp + 88]
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
    pop rbx
    pop rdi
    pop rsi
    pop r14
    pop r13
    pop r12
    ret

CompressionFuncCircus ENDP

END
