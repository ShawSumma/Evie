.intel_syntax noprefix
.global Start_Test
.global main
main:
call Start_Test
mov eax, 1
ret 
ret 


foo:
mov eax, 5
ret 
ret 


Start_Test:
lea rcx, qword ptr [foo ]
mov rcx, rcx
call rcx
mov eax, eax
ret 
ret 


