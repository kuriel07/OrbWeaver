
PUBLIC vm_call_func_x64
_STACK SEGMENT

_STACK ENDS
_DATA SEGMENT

_DATA ENDS
_TEXT SEGMENT

vm_push_stack_x64 MACRO id, var
	;load args_type[cnt]
	mov rbx, [rbp+(8*3)]		;load rbx with args_type pointer
	movzx rbx, word ptr [rbx + (r11*2)]
	mov rax, rbx
	and rbx, 040h
	jz non_arr_push_&id&

	mov rax, &var&
	push rax
	jmp argument_pushed_&id&

non_arr_push_&id&:
	mov rbx, rax
	and rbx, 010h
	jne push_precision_&id&

	;push non precision type
	mov rbx, rax
	and rbx, 00fh

	cmp rbx, 0
	je push_var_&id&
	cmp rbx, 1
	je push_byte_&id&
	cmp rbx, 2
	je push_word_&id&
	cmp rbx, 4
	je push_dword_&id&
	cmp rbx, 8
	je push_qword_&id&

push_var_&id&:
	mov rax, [&var&]
	push rax
	jmp argument_pushed_&id&

push_byte_&id&:
	mov rbx, &var&
	movzx rax,  byte ptr [rbx]
	push rax
	jmp argument_pushed_&id&

push_word_&id&:
	mov rbx, &var&
	movzx rax, word ptr [rbx]
	push rax
	jmp argument_pushed_&id&

push_dword_&id&:
	mov rbx, &var&
	mov eax, dword ptr [rbx]
	;mov [rsp], eax
	;sub rsp, 4
	push rax
	jmp argument_pushed_&id&

push_qword_&id&:
	mov rbx, &var&
	mov rax, [rbx]
	push rax
	jmp argument_pushed_&id&

push_precision_&id&:
	;push precison type
	mov rbx, rax
	and rbx, 00fh

	cmp rbx, 4
	je push_float_&id&

push_double_&id&:
	mov rbx, &var&
	mov rax, [rbx]
	push rax
	jmp argument_pushed_&id&

push_float_&id&:
	mov rbx, &var&
	mov eax, dword ptr [rbx]
	;mov [rsp], eax
	;sub rsp, 4
	push rax

argument_pushed_&id&:
ENDM

vm_get_xmm PROC
	;mov eax, dword ptr [rsp + 028h]
	;push rbp ; save previous frame pointer
	;.pushreg rbp ; encode unwind info
	;mov rbp, rsp ; set new frame pointer
	;.setframe rbp, 0 ; encode frame pointer
	;.endprolog
	;int func, ret_type, args, num_args
	;sub rbp, 0100h	;;allocate local variable (100h)
	;pop rax			;;num_args
	;mov [rbp+(8*0)], rcx		;func
	;mov [rbp+(8*8)], rsp		; saved

	;calculate amount of data 16 byte boundary depending on number of arguments (odd or even)
	;sub rsp, 100h
	;and rsp, 0fffffffffffffff0h

	;mov rax, [rbp+(8*0)]
	;movdqu [rbp+(8*0)], xmm0
	;mov rax, [rbp+(8*0)]

	;add rsp, 100h
	;mov rsp,[rbp+(8*8)] 		; restore
	;mov rsp, rbp
	;pop rbp
	ret
vm_get_xmm ENDP

vm_push_arg_x64 MACRO id, rx, xmm, var
	;load args_type[cnt]
	mov rbx, [rbp+(8*3)]		;load rbx with args_type pointer
	movzx rbx, word ptr [rbx + (r11*2)]
	mov rax, rbx
	and rbx, 040h
	jz non_arr_push_&id&

	mov &rx&, &var&
	;push &rx&
	jmp argument_pushed_&id&

non_arr_push_&id&:
	mov rbx, rax
	and rbx, 010h
	jne push_precision_&id&

	;push non precision type
	mov rbx, rax
	and rbx, 00fh

	cmp rbx, 0
	je push_var_&id&
	cmp rbx, 1
	je push_byte_&id&
	cmp rbx, 2
	je push_word_&id&
	cmp rbx, 4
	je push_dword_&id&
	cmp rbx, 8
	je push_qword_&id&

push_var_&id&:
	mov &rx&, [&var&]
	jmp argument_pushed_&id&

push_byte_&id&:
	mov rbx, &var&
	movzx rax,  byte ptr [rbx]
	mov &rx&, rax
	jmp argument_pushed_&id&

push_word_&id&:
	mov rbx, &var&
	movzx rax, word ptr [rbx]
	mov &rx&, rax
	jmp argument_pushed_&id&

push_dword_&id&:
	mov rbx, &var&
	mov eax, dword ptr [rbx]
	mov &rx&, rax
	jmp argument_pushed_&id&

push_qword_&id&:
	mov rbx, &var&
	mov &rx&, [rbx]
	jmp argument_pushed_&id&

push_precision_&id&:
	;push precison type
	mov rbx, rax
	and rbx, 00fh

	cmp rbx, 4
	je push_float_&id&

push_double_&id&:
	mov rbx, &var&
	mov rax, [rbx]
	mov [rbp + (8*7)], rax
	movdqu &xmm&, [rbp + (8*7)]
	;push &rx&
	jmp argument_pushed_&id&

push_float_&id&:
	mov rbx, &var&
	mov eax, dword ptr [rbx]
	mov [rbp + (8*7)], rax
	movdqu &xmm&, [rbp + (8*7)]
	;movsxd &xmm&, [rbp + (8*7)]
	;push &rx&

argument_pushed_&id&:
ENDM

; vm_call_func_x64(func, ret_type, args, args_type, num_args, &dtemp)
vm_call_func_x64 PROC FRAME
	mov eax, dword ptr[rsp + 028h]
	mov rbx, [rsp + 030h]
	push rbp ; save previous frame pointer
	.pushreg rbp ; encode unwind info
	mov rbp, rsp ; set new frame pointer
	.setframe rbp, 0 ; encode frame pointer
	.endprolog
	;int func, ret_type, args, num_args
	sub rbp, 0100h	;;allocate local variable (100h)
	;pop rax			;;num_args
	mov [rbp+(8*4)], rax		;;store num_args
	mov [rbp+(8*6)], rbx		;;store ptr dtemp
	mov [rbp+(8*0)], rcx		;func
	mov [rbp+(8*1)], rdx		;ret_type
	mov [rbp+(8*10)], r8		;args
	mov [rbp+(8*3)], r9			;args_type
	mov rax,0
	mov [rbp+(8*5)], rax		;;cnt
	mov [rbp+(8*8)], rsp		; saved
	mov [rbp+(8*7)], rax		;reserved variable

	;calculate amount of data 16 byte boundary depending on number of arguments (odd or even)
	sub rsp, 200h
	and rsp, 0fffffffffffffff0h
	mov rax, [rbp+(8*4)]
	cmp rax, 4
	jle skip_rsp_sub
	and rax, 01h
	jz skip_rsp_sub
	sub rsp, 8
	skip_rsp_sub:

loop_push_argument:
	;for(i<num_args;i++) {
	mov rax, [rbp+(8*4)]		;num_args
	mov r11, [rbp+(8*5)]		;cnt
	add rax, 2
	cmp r11, rax
	je exit_push_argument

	cmp r11, 0
	je push_arg0
	cmp r11, 1
	je push_arg1
	cmp r11, 2
	je push_arg2
	cmp r11, 3
	je push_arg3

	;default push
	mov rbx, [rbp+(8*10)]
	mov r10, [rbx+(8*r11)]
	vm_push_stack_x64 8, r10
	jmp increment_counter

push_arg0:
	mov rbx, [rbp + (8*10)]
	mov r10, [rbx + (8*r11)]
	vm_push_arg_x64 0, rcx, xmm0, r10
	jmp increment_counter

push_arg1:
	mov rbx, [rbp + (8*10)]
	mov r10, [rbx + (8*r11)]
	vm_push_arg_x64 1, rdx, xmm1, r10
	jmp increment_counter

push_arg2:
	mov rbx, [rbp + (8*10)]
	mov r10, [rbx + (8*r11)]
	vm_push_arg_x64 2, r8, xmm2, r10
	jmp increment_counter

push_arg3:
	mov rbx, [rbp + (8*10)]
	mov r10, [rbx + (8*r11)]
	vm_push_arg_x64 3, r9, xmm3, r10
	jmp increment_counter

increment_counter:
	;increment counter, jump back to loop
	mov r11, [rbp+(8*5)]
	add r11, 1
	mov [rbp+(8*5)], r11 
	jmp loop_push_argument
	;} end for
exit_push_argument:

	;func();		call target function
	mov rax, [rbp+(8*0)]
	;allocate shadow space
	;push rax
	;push rax
	;push rax
	;push rax
	sub rsp, 4*8
	call rax
	add rsp, 4*8
	;pop rbx
	;pop rbx
	;pop rbx
	;pop rbx
	mov rbx, [rbp+(8*6)]
	movsd mmword ptr[rbx], xmm0

	add rsp, 200h
	mov rsp,[rbp+(8*8)] 		; restore
	;mov rsp, rbp
	pop rbp
	ret
vm_call_func_x64 ENDP


_TEXT ENDS
END