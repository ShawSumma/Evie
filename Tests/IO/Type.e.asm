.intel_syntax noprefix
.global _Z4mainv
.global _Z10Start_Testv
.section .text		#.text
_Z10Start_Testv:
sub rsp, 24		#.STACK, 24
lea rcx, qword ptr [rsp + 0 ]		#x_REG0, .STACK_0
mov rcx, rcx		#x_TMP_1722785009248, x_REG0
add qword ptr [rcx + 0 ], 1		#x_TMP_1722785009248_0, 1
mov rcx, rcx		#this_57, x_TMP_1722785009248
mov r8, rcx		#this_57_TMP_1722784997152, this_57
add qword ptr [r8 + 0 ], 1		#this_57_TMP_1722784997152_0, 1
mov r8, r8		#this_56_57, this_57_TMP_1722784997152
mov r8, rcx		#this_56_57, this_57
mov dword ptr [r8 + 8 ], 4		#this_56_57_8, 4
Return_Here_79:
mov dword ptr [rcx + 12 ], 1		#this_57_12, 1
mov dword ptr [rcx + 16 ], 2		#this_57_16, 2
mov dword ptr [rcx + 20 ], 6		#this_57_20, 6
mov dword ptr [rcx + 8 ], 5		#this_57_8, 5
jmp Return_Here_78		#Return_Here_78
Return_Here_78:
lea rcx, qword ptr [rsp + 0 ]		#x_REG1, .STACK_0
mov rcx, rcx		#x_TMP_1722784990240, x_REG1
add qword ptr [rcx + 0 ], 1		#x_TMP_1722784990240_0, 1
mov rcx, rcx		#f_58, x_TMP_1722784990240
mov r8, rcx		#f_58_TMP_1722784991104, f_58
add qword ptr [r8 + 0 ], 1		#f_58_TMP_1722784991104_0, 1
mov r8, r8		#b_55_58, f_58_TMP_1722784991104
mov r8, rcx		#b_55_58, f_58
jmp Return_Here_81		#Return_Here_81
Return_Here_81:
mov ecx, dword ptr [r8 + 12 ]		#b_55_58_12_REG2, b_55_58_12
mov ecx, ecx		#Return_Value58, b_55_58_12_REG2
jmp Return_Here_80		#Return_Here_80
Return_Here_80:
mov eax, ecx		#Returning_REG3, Return_Value58
add rsp, 24		#.STACK, 24
ret 		#
add rsp, 24		#.STACK, 24
ret 		#


_Z4mainv:
sub rsp, 24		#.STACK, 24
lea rcx, qword ptr [rsp + 0 ]		#x_59_REG0, .STACK_0
mov rcx, rcx		#x_59_TMP_1722785029984, x_59_REG0
add qword ptr [rcx + 0 ], 1		#x_59_TMP_1722785029984_0, 1
mov rcx, rcx		#this_57_59, x_59_TMP_1722785029984
mov r8, rcx		#this_57_59_TMP_1722785042080, this_57_59
add qword ptr [r8 + 0 ], 1		#this_57_59_TMP_1722785042080_0, 1
mov r8, r8		#this_56_57_59, this_57_59_TMP_1722785042080
mov r8, rcx		#this_56_57_59, this_57_59
mov dword ptr [r8 + 8 ], 4		#this_56_57_59_8, 4
Return_Here_83:
mov dword ptr [rcx + 12 ], 1		#this_57_59_12, 1
mov dword ptr [rcx + 16 ], 2		#this_57_59_16, 2
mov dword ptr [rcx + 20 ], 6		#this_57_59_20, 6
mov dword ptr [rcx + 8 ], 5		#this_57_59_8, 5
jmp Return_Here_84		#Return_Here_84
Return_Here_84:
lea rcx, qword ptr [rsp + 0 ]		#x_59_REG1, .STACK_0
mov rcx, rcx		#x_59_TMP_1722785067136, x_59_REG1
add qword ptr [rcx + 0 ], 1		#x_59_TMP_1722785067136_0, 1
mov rcx, rcx		#f_58_59, x_59_TMP_1722785067136
mov r8, rcx		#f_58_59_TMP_1722785062816, f_58_59
add qword ptr [r8 + 0 ], 1		#f_58_59_TMP_1722785062816_0, 1
mov r8, r8		#b_55_58_59, f_58_59_TMP_1722785062816
mov r8, rcx		#b_55_58_59, f_58_59
jmp Return_Here_85		#Return_Here_85
Return_Here_85:
mov ecx, dword ptr [r8 + 12 ]		#b_55_58_59_12_REG2, b_55_58_59_12
mov ecx, ecx		#Return_Value58_59, b_55_58_59_12_REG2
jmp Return_Here_86		#Return_Here_86
Return_Here_86:
jmp Return_Here_82		#Return_Here_82
Return_Here_82:
mov eax, 1		#Returning_REG3, 1
add rsp, 24		#.STACK, 24
ret 		#
add rsp, 24		#.STACK, 24
ret 		#


_ZN6Banana10DestructorEP6Banana:
push rbx		#rbx
if_23:
cmp rcx, 0		#this, 0
je if_23_END		#if_23_END
mov r8, qword ptr [rcx + 0 ]		#this_0_REG0, this_0
sub r8, 1		#this_0_REG0, 1
cmp r8, 1		#this_0_REG0, 1
jge if_23_END		#if_23_END
mov r8, rcx		#this_TMP_1722785078368, this
add qword ptr [r8 + 0 ], 1		#this_TMP_1722785078368_0, 1
mov rbx, r8		#Address_54, this_TMP_1722785078368
mov rbx, rcx		#Address_54, this
mov rcx, rbx		#REG_Address_54_Parameter4833, Address_54
mov r8d, dword ptr [rbx + 20 ]		#Address_54_20_REG1, Address_54_20
mov edx, r8d		#REG_Address_54_20_REG1_Parameter31115, Address_54_20_REG1
call _V19internal_deallocatePhx
Return_Here_75:
mov rcx, rbx		#REG_Address_54_Parameter4639, Address_54
call _ZN6Banana10DestructorEP6Banana
if_23_END:
pop rbx		#rbx
ret 		#


.section .data		#.data
std_MAX_CONCOLE_BUFFER_LENGHT:
.long 4096		#4096
std_GENERIC_WRITE:
.long 1073741824		#1073741824
std_GENERIC_READ:
.quad 2147483648		#2147483648
std_FILE_SHARE_NONE:
.long 0		#0
std_FILE_SHARE_READ:
.long 1		#1
std_FILE_SHARE_WRITE:
.long 2		#2
std_FILE_SHARE_DELETE:
.long 4		#4
std_CREATE_NEW:
.long 1		#1
std_CREATE_ALWAYS:
.long 2		#2
std_OPEN_EXISTING:
.long 3		#3
std_OPEN_ALWAYS:
.long 4		#4
std_TRUNCATE_EXISTING:
.long 4		#4
std_FILE_ATTRIBUTE_NORMAL:
.long 128		#128
std_FILE_ATTRIBUTE_FOLDER:
.long 16		#16
std_MAXIMUM_PATH_LENGTH:
.long 260		#260
std_ERROR_INSUFFICIENT_BUFFER:
.long 122		#122
std_MINIMUM_PROCESS_FILENAME_LENGTH:
.long 50		#50
