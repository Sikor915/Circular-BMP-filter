;-------------------------------------------------------------------------
;.586
;INCLUDE C:\masm32\include\windows.inc 

.CODE
;-------------------------------------------------------------------------
; To jest przyk�adowa funkcja. 
;-------------------------------------------------------------------------

CompressionFunc PROC 		;parametry ECX EDX
add 	rcx, rdx
mov 	rax, rcx
jnc ET1
ror	rcx,1
mul 	rcx
ret
ET1:	
neg 	rax
ret
;DO SOMETHING FROM THE ALGORITHM!
;FUNCTION START:


CompressionFunc ENDP

CompressionFuncReal PROC
    mov al, cl      ; Przenie� warto�� piksela do al
    add al, 3       ; Dodaj 3 do warto�ci piksela
    mov cl, al
    ret
CompressionFuncReal ENDP

END 			;no entry point