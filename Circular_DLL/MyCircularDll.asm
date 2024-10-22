;-------------------------------------------------------------------------
;.586
;INCLUDE C:\masm32\include\windows.inc 

.CODE

CompressionFuncReal PROC
    mov al, cl      
    add al, 200
    mov cl, al
    ret
CompressionFuncReal ENDP

END 			;no entry point