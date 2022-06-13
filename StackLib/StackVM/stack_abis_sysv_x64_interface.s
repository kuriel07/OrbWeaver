.global	vm_call_func_x64
#PUBLIC vm_call_func_x64
#_STACK SEGMENT

#_STACK ENDS
#_DATA SEGMENT

#_DATA ENDS
#_TEXT SEGMENT

.extern vm_print_var
.extern vm_print_sss
.text
.macro vm_push_stack_x64 id, var
	#load args_type[cnt]
	mov 0x18(%rbp), %rbx 		#load %rbx with args_type pointer
	movzx (%rbx, %r11, 2), %bx 
	mov %rbx, %rax
	and $0x40, %rax 
	jz non_arr_push_&id&

	mov &var&, %rax
	mov 0x68(%rbp), %r11
	mov %rax, (%rsp, %r11, 8)
	#push %rax
	jmp argument_pushed_&id&

non_arr_push_&id&:

	#push non precision type
	mov %rbx, %rax
	and $0x0f, %rax

	cmp $0, %rax
	je push_var_&id&
	cmp $1, %rax
	je push_byte_&id&
	cmp $2, %rax
	je push_word_&id&
	cmp $4, %rax
	je push_dword_&id&
	cmp $8, %rax
	je push_qword_&id&

push_var_&id&:
	mov &var&, %rax
	mov 0x68(%rbp), %r11
	mov %rax, (%rsp, %r11, 8)
	#push %rax
	jmp argument_pushed_&id&

push_byte_&id&:
	mov (&var&), %rbx
	mov $0, %rax		#clear rax
	mov %bl, %al
	mov 0x68(%rbp), %r11
	mov %rax, (%rsp, %r11, 8)
	#push %rax
	jmp argument_pushed_&id&

push_word_&id&:
	mov (&var&), %rbx
	mov $0, %rax		#clear rax
	mov %bx, %ax
	mov 0x68(%rbp), %r11
	mov %rax, (%rsp, %r11, 8)
	#push %rax
	jmp argument_pushed_&id&

push_dword_&id&:
	mov (&var&), %rbx
	mov $0, %rax		#clear rax
	mov %ebx, %eax		#mov only lower dword
	mov 0x68(%rbp), %r11
	mov %rax, (%rsp, %r11, 8)
	#push %rax
	jmp argument_pushed_&id&

push_qword_&id&:
	#mov (&var&), %rdi
	#call vm_print_var
	mov (&var&), %rbx
	mov %rbx, %rax
	mov 0x68(%rbp), %r11
	mov %rax, (%rsp, %r11, 8)
	#push %rax
	jmp argument_pushed_&id&


argument_pushed_&id&:
	mov 0x68(%rbp), %rbx
	add $1, %rbx
	mov %rbx, 0x68(%rbp)
.endm

#vm_get_xmm PROC
vm_get_xmm:
	#mov eax, dword ptr [%rsp + 028h]
	#push %rbp # save previous frame pointer
	#.pushreg %rbp # encode unwind info
	#mov %rbp, %rsp # set new frame pointer
	#.setframe %rbp, 0 # encode frame pointer
	#.endprolog
	#int func, ret_type, args, num_args
	#sub %rbp, 0100h	##allocate local variable (100h)
	#pop %rax			##num_args
	#mov [%rbp+(8*0)], %rcx		#func
	#mov [%rbp+(8*8)], %rsp		# saved

	#calculate amount of data 16 byte boundary depending on number of arguments (odd or even)
	#sub %rsp, 100h
	#and %rsp, 0fffffffffffffff0h

	#mov %rax, [%rbp+(8*0)]
	#movdqu [%rbp+(8*0)], xmm0
	#mov %rax, [%rbp+(8*0)]

	#add %rsp, 100h
	#mov %rsp,[%rbp+(8*8)] 		# restore
	#mov %rsp, %rbp
	#pop %rbp
	ret
##vm_get_xmm ENDP

.macro vm_push_fp_x64 id, xmm, var

	#load args_type[cnt]
	mov 0x18(%rbp), %rbx 		#load %rbx with args_type pointer
	movzx (%rbx, %r11, 2), %bx  	#load precision type (r11 = counter)
	mov %rbx, %rax
	and $0x0f, %rax
	cmp $4, %rax			#check precision type (double/float)
	je push_float_&id&

push_double_&id&:
	mov &var&, %rbx
	mov (%rbx), %rax
	mov %rax, 0x38(%rbp)
	#TODO push argument to xmm0-xmm7, else to stack
	#movapd %xmm6,%xmm7
	#movapd %xmm5,%xmm6
	#movapd %xmm4,%xmm5
	#movapd %xmm3,%xmm4
	#movapd %xmm2,%xmm3
	#movapd %xmm1,%xmm2
	#movapd %xmm0,%xmm1
	movsd 0x38(%rbp), &xmm&
	jmp fp_pushed_&id&

push_float_&id&:
	mov &var&, %rbx
	mov (%rbx), %eax
	#convert float to double
	#mov $0, %rax
	#flds (%rbx, %rax,4)
	#fstl 0x38(%rbp, %rax, 8)
	mov %eax, 0x38(%rbp)
	#TODO push argument to xmm0-xmm7, else to stack
	#movapd %xmm6,%xmm7
	#movapd %xmm5,%xmm6
	#movapd %xmm4,%xmm5
	#movapd %xmm3,%xmm4
	#movapd %xmm2,%xmm3
	#movapd %xmm1,%xmm2
	#movapd %xmm0,%xmm1
	movss 0x38(%rbp), &xmm&
fp_pushed_&id&:
	#mov 0x38(%rbp), %rdi
	#call vm_print_var
.endm

.macro vm_push_int_x64 id, rx, var
	#load args_type[cnt]
	mov 0x18(%rbp), %rbx 		#load %rbx with args_type pointer
	movzx (%rbx, %r11, 2), %bx  	#, word ptr 
	
	mov %rbx, %rax 
	and $0x40, %rax 
	jz non_arr_push_&id&

	mov &var&, &rx&
	jmp int_pushed_&id&

non_arr_push_&id&:
	#push non pointer/array type
	mov %rbx, %rax
	and $0x0f, %rax

	cmp $0, %rax 
	je push_var_&id&
	cmp $1, %rax
	je push_byte_&id&
	cmp $2, %rax
	je push_word_&id&
	cmp $4, %rax
	je push_dword_&id&
	cmp $8, %rax
	je push_qword_&id&

push_var_&id&:
	mov &var&, &rx& 
	jmp int_pushed_&id&

push_byte_&id&:
	mov &var&, %rbx
	mov (%rbx), %bl
	mov %rbx, &rx&
	jmp int_pushed_&id&

push_word_&id&:
	mov &var&, %rbx
	mov (%rbx), %bx
	mov %rbx, &rx&
	jmp int_pushed_&id&

push_dword_&id&:
	mov &var&, %rbx
	mov (%rbx), %ebx
	mov %rbx, &rx&
	jmp int_pushed_&id&

push_qword_&id&:
	mov &var&, %rbx
	mov (%rbx), &rx&
	jmp int_pushed_&id&

int_pushed_&id&:
	#mov &rx&, %rdi
	#call vm_print_var
.endm

# vm_call_func_x64(func, ret_type, args, args_type, num_args, &dtemp)
#vm_call_func_x64 PROC FRAME
vm_call_func_x64:
	#enter 0
	push %rbp 		# save previous frame pointer
	mov %rsp, %rbp 	# set new frame pointer
	#int func, ret_type, args, num_args
	sub $0x100, %rbp 	##allocate local variable (100h)
	#store arguments 	
	mov %rdi, 0x00(%rbp) 		#func
	mov %rsi, 0x08(%rbp) 		#ret_type
	mov %rdx, 0x50(%rbp) 		#args
	mov %rcx, 0x18(%rbp)		#args_type
	mov %r8, 0x20(%rbp)		##store num_args
	mov %r9, 0x60(%rbp)		##store ptr dtemp
	mov %rsp, 0x40(%rbp)		#save current stack pointer to local var
	mov $0, %rax
	mov %rax, 0x28(%rbp)		##cnt
	mov %rax, 0x38(%rbp)		#reserved variable
	mov %rax, 0x48(%rbp)		#integer counter
	mov %rax, 0x58(%rbp)		#precision counter
	mov %rax, 0x68(%rbp)		#stack counter
	
	#calculate amount of data 16 byte boundary depending on number of arguments (odd or even)
	sub $0x200, %rsp
	#and $0x0fffffffffffffff0, %rsp 
	#sub $8, %rsp
	#TODO calculate the need for boundary based on arguments on stack
	jmp loop_push_argument
	mov 0x20(%rbp), %rax		#load num_args to rax
	#cmp $4, %rax 			
	#jle skip_rsp_sub		#no additional stack arguments?
	and $0x01, %rax			
	jz skip_rsp_sub
	#mov 0x20(%rbp), %rax
	#mov $8, %rbx
	#mul %rbx
	sub $8, %rsp
	skip_rsp_sub:

loop_push_argument:
	#for(i<num_args#i++) {
	mov 0x20(%rbp), %rax 		#num_args
	mov 0x28(%rbp), %r11 		#cnt
	add $2, %rax			#additional arguments (ctx, iface, ....
	cmp %rax, %r11
	je exit_push_argument		#no arguments left

	#check for precision type
	mov 0x18(%rbp), %rbx 		#load %rbx with args_type pointer
	movzx (%rbx, %r11, 2), %bx  	#load args_type
	mov %rbx, %rax
	and $0x10, %rax
	jnz push_precision_arg

	#push integer or pointer
	#vm_push_integer 0
	mov 0x28(%rbp), %r11 		#cnt
	mov 0x48(%rbp), %rbx		#integer counter
	mov %rbx, %rax

	cmp $0, %rax
	je push_int0
	cmp $1, %rax
	je push_int1
	cmp $2, %rax
	je push_int2
	cmp $3, %rax
	je push_int3
	cmp $4, %rax
	je push_int4
	cmp $5, %rax
	je push_int5

	#default push (integer counter > 6)
	#call vm_print_sss

	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10 
	vm_push_stack_x64 8, %r10
	jmp increment_int_counter

push_int0:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10 
	vm_push_int_x64 0, %rdi, %r10
	jmp increment_int_counter

push_int1:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_int_x64 1, %rsi, %r10
	jmp increment_int_counter

push_int2:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_int_x64 2, %rdx, %r10
	jmp increment_int_counter

push_int3:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_int_x64 3, %rcx, %r10
	jmp increment_int_counter

push_int4:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_int_x64 4, %r8, %r10
	jmp increment_int_counter

push_int5:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_int_x64 5, %r9, %r10
	jmp increment_int_counter

increment_int_counter:
	#increment integer counter
	mov 0x48(%rbp), %rax
	add $1, %rax
	mov %rax, 0x48(%rbp) 

	jmp increment_counter
	
	#push precision arguments
push_precision_arg:
	
	#increment integer counter
	mov 0x28(%rbp), %r11 		#cnt
	mov 0x58(%rbp), %rbx		#precision counter
	
	cmp $0, %rbx
	je push_fp0
	cmp $1, %rbx
	je push_fp1
	cmp $2, %rbx
	je push_fp2
	cmp $3, %rbx
	je push_fp3
	cmp $4, %rbx
	je push_fp4
	cmp $5, %rbx
	je push_fp5
	cmp $6, %rbx
	je push_fp6
	cmp $7, %rbx
	je push_fp7

	#default push (precision counter > 8)
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10 
	vm_push_stack_x64 16, %r10
	jmp increment_fp_counter

push_fp0:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_fp_x64 0, %xmm0, %r10
	jmp increment_fp_counter
push_fp1:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_fp_x64 1, %xmm1, %r10
	jmp increment_fp_counter
push_fp2:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_fp_x64 2, %xmm2, %r10
	jmp increment_fp_counter
push_fp3:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_fp_x64 3, %xmm3, %r10
	jmp increment_fp_counter
push_fp4:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_fp_x64 4, %xmm4, %r10
	jmp increment_fp_counter
push_fp5:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_fp_x64 5, %xmm5, %r10
	jmp increment_fp_counter
push_fp6:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_fp_x64 6, %xmm6, %r10
	jmp increment_fp_counter
push_fp7:
	mov 0x50(%rbp), %rbx 
	mov (%rbx,%r11,8), %r10
	vm_push_fp_x64 7, %xmm7, %r10
	jmp increment_fp_counter

increment_fp_counter:
	#increment precision counter
	mov 0x58(%rbp), %rax
	add $1, %rax
	mov %rax, 0x58(%rbp) 
	
	
increment_counter:
	#increment counter, jump back to loop
	mov 0x28(%rbp), %r11
	add $1, %r11
	mov %r11, 0x28(%rbp)
	jmp loop_push_argument
	#} end for
	.byte 00,00,00,00,00,00,00,00
exit_push_argument:
	
	mov 0x00(%rbp), %rax
	call %rax

	mov 0x60(%rbp), %rbx 
	movsd %xmm0, (%rbx)
	mov 0x40(%rbp), %rsp		# restore
	pop %rbp
	#leave
	ret

	#func()#		call target function
	mov 0x00(%rbp), %rax
	#allocate shadow space (microsoft calling convention)
	#push %rax
	#push %rax
	#push %rax
	#push %rax
	#sub $0x40, %rsp
	#sub $8, %rbp

	call *%rax

	#add $8, %rbp
	#add $0x40, %rsp
	#pop %rbx
	#pop %rbx
	#pop %rbx
	#pop %rbx
	#nop
	mov 0x60(%rbp), %rbx 
	movsd %xmm0, (%rbx)

	mov 0x40(%rbp), %rsp		# restore
	pop %rbp
	ret
#vm_call_func_x64 ENDP


#_TEXT ENDS
#END
