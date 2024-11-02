;-------------------------------------------------------------------------
;.586
;INCLUDE C:\masm32\include\windows.inc 

.CODE

; V1 
; RCX - adres z informacja do pikseli
; RDX - rozmiar do przejscia
CompressionFuncReal PROC
    mov al, [rcx]      
    add al, 200
    mov [rcx], al
    ret

CompressionFuncReal ENDP


END 			;no entry point