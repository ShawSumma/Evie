;analyze
section .text

extern _GetStdHandle@4
extern _WriteConsoleA@20
extern _ReadConsoleA@20

global sys_print
sys_print:

; esp+16 =  len
; esp+12 = buf
; esp+8 = return
; esp+4 = 0
; esp = written

push -11 ; STD_OUTPUT_HANDLE
call _GetStdHandle@4

push 0 ; lpReserved
push written ; lpNumberOfCharsWritten
push dword [esp+16] ; nNumberOfCharsToWrite 
push dword [esp+16] ; lpBuffer
push eax ; hConsoleOutput 
call _WriteConsoleA@20

ret

; ebp+8: length
; ebp+4: buffer
global sys_read
sys_read:

push -10 ; STD_INPUT_HANDLE
call _GetStdHandle@4

; lpNumberOfCharsRead
sub esp, 4
mov ebx, esp

push 0 ; pInputControl
push ebx ; lpNumberOfCharsRead
push dword [esp+20] ; nNumberOfCharsToRead
push dword [esp+20] ; lpBuffer
push eax ; hConsoleInput
call _ReadConsoleA@20

; lpNumberOfCharsRead
mov eax, [esp]
add esp, 4
ret

;test the normal laber analyzer
global new
	new:
	
global _Z3newi
	_Z3newi:

;int ptr banana(int, apple ptr)
;_E stands for Evie
;P stands for ptr
;number stands for the length of the class name
;first type is the returning type
;second is the func name
;rest are the parameters
_EP3int6banana3intP5apple:

section .data
written dd 0