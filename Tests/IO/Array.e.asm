.intel_syntax noprefix
.global _Z4mainv
.global _Z10Start_Testv
.section .text
_Z10Start_Testv:
sub rsp, 16
mov ecx, 10
mov r8d, ecx
mov r9, rdx
mov eax, 4
mul r8d
mov r8d, eax
mov rdx, r9
mov ecx, ecx
mov r8, rdx
mov eax, 4
mul ecx
mov ecx, eax
mov rdx, r8
mov ecx, ecx
call _V17internal_allocatex_rPh
mov qword ptr [rsp + 8 ], rax
jmp Return_Here_42
Return_Here_42:
mov rcx, qword ptr [rsp + 8 ]
mov qword ptr [rsp ], rcx
lea rcx, qword ptr [rsp ]
mov rcx, rcx
mov r8, qword ptr [rcx + 0 * 8 ]
lea r8, qword ptr [r8 + 1 * 4 ]
mov dword ptr [r8 ], 1
mov rcx, qword ptr [rcx + 0 * 8 ]
mov ecx, dword ptr [rcx + 1 * 4 ]
mov ecx, ecx
mov eax, ecx
add rsp, 16
ret 
add rsp, 16
ret 


_Z4mainv:
mov eax, 1
ret 
ret 


.section .data
