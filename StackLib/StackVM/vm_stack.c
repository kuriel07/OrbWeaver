#include "../defs.h"
#include "../config.h"
#include "vm_stack.h"
#include "vm_framework.h"
#include "vm_stream.h"
#include "midgard.h"
#include "../Stack/pkg_linker.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef __cplusplus_cli
#pragma managed
#include "..\stdafx.h"
#using <mscorlib.dll>
#include "..\StackLib.h"

using namespace System;
using namespace StackLib;
using namespace System::Runtime::InteropServices;
#endif

void vm_memcpy(void * dst, void * src, size_t sz) {
	WORD i;
	if (dst == src) return;
	if (sz == 0) return;
	if (dst > src) {
		for (i = sz;i>0;) {
			i--;
			((uint8 *)dst)[i] = ((uint8 *)src)[i];
		}
	}
	else {
		memcpy(dst, src, sz);
	}
}

int vm_memcmp(void * dst, void * src, size_t sz) {
	return memcmp(dst, src, sz);
}

void vm_memset(void * dst, int set, size_t sz) {
	memset(dst, set, sz);
}

size_t vm_strlen(const char * str) {
	return strlen(str);
}

void vm_object_ref(vm_object * obj) {
	if ((obj->mgc_refcount & 0x0F) != 0xF) {
		obj->mgc_refcount++;
		obj->lock++;
	}
}

void vm_object_unref(vm_object * obj) {
	if((obj->mgc_refcount & 0x0F) != 0) obj->mgc_refcount--;
	if(obj->lock != 0) obj->lock--;
	//printf("unref %lx\n", obj);
}


#if defined(__cplusplus_cli) || defined(STACK_HOST)
LIB_API void vm_init(VM_DEF_ARG, pk_object * root, uint32 heap_size)
{
	//memset(_vm_stacks, 0, sizeof(_vm_stacks));
	memset(&vm_get_context(), 0, sizeof(vm_instance));
	vm_set_heap(m_heap_create(heap_size));
	m_init_alloc(vm_get_heap());
	vm_get_context().var_root = 0;
	vm_get_context().vars = &vm_get_context().var_root;
	vm_set_pc(0);
	vm_set_sp(VM_MAX_STACK_SIZE -1);		//stack pointer
	vm_set_bp(0);		//base pointer
	vm_set_info((pk_object *)root);
}

LIB_API void vm_close(VM_DEF_ARG)
{
	//printf("close virtual machine, free heap\n");
	free(vm_get_heap());
}
#endif

//#define VM_CODEDEBUG		1
#ifdef STANDALONE_VIRTUAL_MACHINE
FILE * _ilfile = NULL;
#endif
extern const uchar * _ins_name[];

#if VM_CODEDEBUG
static uint32 _current_offset = 0;
static void vm_debug(uint32 offset, uchar * str, ...) {
	if(offset >= _current_offset) {
		//if(_asmfile != NULL) {
			//fprintf(_asmfile, "%08x:%s\n", offset, str);
		printf("%08x:%s\n", offset, str);
		//_current_offset = offset;
		//}
	}
}

static void vm_debug_var(VM_DEF_ARG, int opcode, vm_object * var) {
	uchar dbgbuf[256];
	uchar buffer[192];
	int len;
	memset(dbgbuf, 0, sizeof(dbgbuf));
	switch(vm_object_get_type(var)) {
		case VM_MAGIC:
			//interface, instance
			switch(var->type) {
				case VM_OBJ_TYPE_ANY:
					sprintf(dbgbuf, "\t\t%s ", _ins_name[opcode]);
					if(var->len > 64) {	
						memcpy(dbgbuf + strlen(dbgbuf), var->bytes, 64);
						strcat(dbgbuf, "...");
					} else {
						memcpy(dbgbuf + strlen(dbgbuf), var->bytes, var->len);
					}
					break;
				default:
				case VM_OBJ_TYPE_INSTANCE:
				case VM_OBJ_TYPE_INTERFACE:
					sprintf(dbgbuf, "\t\t%s 0x%lx", _ins_name[opcode], var);
					break;
			}
			break;
		case VM_EXT_MAGIC:
			len = ((vm_extension *)var->bytes)->apis->text(VM_ARG, var, buffer);
			sprintf(dbgbuf, "\t\t%s ", _ins_name[opcode]);
			if(len > 64) {	
				memcpy(dbgbuf + strlen(dbgbuf), buffer, 64);
				strcat(dbgbuf, "...");
			} else {
				memcpy(dbgbuf + strlen(dbgbuf), buffer, len);
			}
			break;
		default:
			//extension, operand or object/array
			sprintf(dbgbuf, "\t\t%s 0x%lx", _ins_name[opcode], var);
			break;
	}
	vm_debug(vm_get_pc(), dbgbuf);
}
#endif

static void vm_iptr_release(void * ctx, void * ptr) {
	//printf("free memory : %ld\n", ptr);
	if (ptr != NULL) {
		void * iface = ((void **)ptr)[0];
		if (iface == NULL) return;
		//free(iface->ptr);
		free( iface);
	}
}
/*
static uint16 _sp = VM_MAX_STACK_SIZE -1;		//stack pointer
static uint16 _bp = 0;		//base pointer
static uint16 _pc = 0;
void * _base_address = NULL;
static void * _vm_stacks[VM_MAX_STACK_SIZE];
static uchar _vm_state;
uint8 g_recursive_exec = FALSE;		//set recursive_exec flag to false
vm_context _vm_file;
BYTE _vm_current_api = 0;
sys_context g_sysc;
vm_object * g_pVaRetval;
*/

#define VM_CMP_EQ			0x10
#define VM_CMP_GT			0x20
#define VM_CMP_LT			0x40

#pragma unmanaged

#ifdef STANDALONE_VIRTUAL_MACHINE
void vm_init(VM_DEF_ARG, uchar * exepath, uchar * inpath) {		//original input file path as parameter
	uchar asmpath[512];
	uchar ilpath[512];
	int size;
	uint16 i = strlen(exepath);
	m_init_alloc(vm_get_heap());
	//_ilfile = fopen(inpath, "rb");
	do { i--; }
	while(exepath[i] != '\\' && i != 0);
	//exepath[i] = 0;
	//printf(exepath);
	if(i == 0) {
		lk_import_directory(".\\framework");
	} else {
		exepath[i] = 0;
		sprintf(exepath, "%s\\framework", exepath);
		lk_import_directory(exepath);
	}
	vm_set_package(lk_decode_file(inpath, &size));
	lk_dump_classes();
	//memset(_vm_stacks, 0, sizeof(_vm_stacks));
	memset(&vm_get_context(), 0, sizeof(vm_instance));
	vm_get_context().var_root = 0;
	vm_get_context().vars = &vm_get_context().var_root;
	vm_set_pc(0);
	vm_set_sp(VM_MAX_STACK_SIZE -1);		//stack pointer
	vm_set_bp(0);		//base pointer
}

#ifndef STANDALONE_INTERPRETER
uint32 vm_fetch(VM_DEF_ARG, uint32 offset, uchar * buffer, uint32 size) {
	uint32 readed = 0;
	
	memcpy(buffer, _RECAST(uchar *, vm_get_package()) + offset, size);
	return size;
}
#endif

void vm_close(VM_DEF_ARG) {
	if(_ilfile != NULL) {
		fclose(_ilfile);
	}
}
#endif

vm_object * vm_push_stack_arc(VM_DEF_ARG, vm_object * obj) {
	vm_push_stack(obj);
	if(obj != NULL) {
		obj->mgc_refcount++;
	}
	return obj;
}

vm_object * vm_pop_stack_arc(VM_DEF_ARG) {
	vm_object * obj = NULL;
	obj = _RECAST(vm_object *, vm_pop_stack());
	if(ctx->sp >= VM_MAX_STACK_SIZE) {
		vm_invoke_exception(VM_ARG, VX_STACK_OVERFLOW);
		getchar();
		return NULL;
	}
	if(obj != NULL) {
		obj->mgc_refcount--;
	}
	vm_set_stack(NULL);
	return obj;
}

void vm_push_argument(VM_DEF_ARG, int size, uint8 * buffer) {
	vm_object * obj;
	obj = vm_create_object(VM_ARG, size, buffer);  
	vm_push_stack(obj);
}

void vm_push_argument_object(VM_DEF_ARG, vm_object * obj) {
	vm_push_stack(obj);
	obj->mgc_refcount++;
}

uint8 vm_pop_result(VM_DEF_ARG, uint8 max_len, uint8 * buffer) {
   	vm_object * obj = (vm_object *)vm_pop_stack();
	uint8 size;
	if(obj->len == 0) return 0;
	size = (obj->len < max_len)?obj->len:max_len;
	obj->mgc_refcount--;
	memcpy(buffer, obj->bytes, size);
	return size;
}

vm_variable * vm_variable_new(vm_variable ** root, uint8 type, uint16 length, uint8 * bytes) {
	vm_variable * var;
	vm_variable * iterator = root[0];
	//if(root != _var_root) return NULL;		//only current context could create variable, private accessor
	var = (vm_variable *)malloc(sizeof(vm_variable) + length);
	if(var != NULL) {
		var->mgc = type;
		var->len = length;
		memcpy(var->bytes, bytes, length);
		var->next = NULL;
		if(root[0] == NULL) {
			root[0] = var;
		} else {
			while(iterator->next != NULL) {
				iterator = iterator->next;
			}
			iterator->next = var;
		}
	}
	return var;
}

void vm_variable_release(vm_variable ** root, vm_variable * var) {
	vm_variable * iterator = root[0];
	//if(root != _var_root) return;			//only current context could release variable, private accessor
	if(iterator == var) {
		root[0] = iterator->next;
	} else {
		while(iterator != NULL) {
			if(iterator->next == var) {
				iterator->next = var->next;
				break;
			}
			iterator = iterator->next;
		}
	}
	free(var);
}

void vm_variable_clear(vm_variable ** root) {
	vm_variable * iterator = root[0];
	vm_variable * var;
	while(iterator != NULL) {
		var = iterator;
		iterator = iterator->next;
		free(var);
	}
	root = NULL;
}

//added 2018.01.10
void vm_exec_function(VM_DEF_ARG, vm_function * func) {
	//vm_context vctx;
	uint8 hbuf[4];
	uint8 tag, hlen;
	uint16 header_size = 0;
	uint16 codestart = 0;
	//save current vm_context base stack, PC should already incremented
	//vm_memcpy(&vctx.handle, &_vm_file, sizeof(vf_handle));
	//vctx.offset = _pc;
	//_vm_stacks[_bp++] = _RECAST(void *, _base_address);				//save current base address
	//_vm_stacks[_bp++] = _RECAST(void *, _pc);				//save PC+3, address of next instruction
	vm_push_base(vm_get_package());
	vm_push_base(_RECAST(void *, vm_get_pc()));		
	//_vm_stacks[_bp++] = vm_create_object(sizeof(vm_context), &vctx);
	//set new vm_context based on func argument
	//vm_memcpy(&_vm_file, &func->base.handle, sizeof(vf_handle));
	//only for terminal (because each bytecodes file are stored with their respectual header codes)
#if 1			
	//hlen = vf_pop_handle(&_vm_file, 0, &tag, &header_size);
	//codestart = header_size + hlen;										//total header length
#endif
	//_pc = func->offset + codestart;
	vm_set_package(func->base);
	vm_set_pc(func->offset + codestart);
	//g_recursive_exec = TRUE;
	vm_set_rexec(TRUE);
}

#ifndef __cplusplus_cli
void * vm_function_get_instance(vm_function * func) {
	if(func != NULL) return func->instance;
	return NULL;
}

void * vm_load_callback(VM_DEF_ARG, vm_function * func) {
	vm_function * alloc = (vm_function *)malloc(sizeof(vm_function));
	if(alloc != NULL) {
		memcpy(alloc, func, sizeof(vm_function));
		alloc->base = func->base;
		alloc->instance = VM_ARG;
	}
	return alloc;
}

void vm_release_callback(VM_DEF_ARG, vm_function * func) {
	if(func != NULL) free(func);
}

void vm_exec_callback(VM_DEF_ARG, vm_function * func, int num_args, ...) {
	vm_instance vcit;
	va_list vlist;
	int i;
	vm_object * arg, * varg;
	vm_object * args[16];
	int arg_count = 0;
	vm_init(&vcit, (pk_object *)vm_get_info(), 20 * 1024);
	vcit.stream = vm_get_stream();
	vm_exec_function(&vcit, func);
	printf("vm_exec_function %lld==%lld==%lld\n", vm_get_package(), func->base, vcit.base_address);
	va_start(vlist, num_args);
	for (i = 0;i < num_args && i<func->arg_count && i<14;i++) {
		arg = va_arg(vlist, vm_object *);
		varg = (vm_object *)m_alloc(vcit.heap, sizeof(vm_object) + arg->len);
		memcpy(varg, arg, sizeof(vm_object) + arg->len);
		args[arg_count++] = varg;
		vm_release_object(VM_ARG, arg);
	}
	for (;i < func->arg_count && i<14;i++) args[arg_count++] = vm_create_object(&vcit, 0, NULL);
	for (i = 0;i < func->arg_count;i++) vm_push_argument_object(&vcit, args[i]);
	va_end(vlist);
	printf("vm_decode\n");
	vm_decode(&vcit, 0, 0);
	vm_close(&vcit);
	//vm_close(VM_ARG);
}
#endif

uint8 vm_init_exec(VM_DEF_ARG, char * classname, char * method) {
	vm_object * obj;
	//vm_object * cla = vm_create_object(VM_ARG, strlen(_RECAST(const char *, classname)), _RECAST(uchar *, classname));
	vm_object * cla = vm_load_class_sta(VM_ARG, (uchar *)classname);
	vm_object * mtd = vm_create_object(VM_ARG, strlen(_RECAST(const char *, method)), _RECAST(uchar *, method));
	//printf("create object success\n");
	obj = _RECAST(vm_object *, vm_load_method(VM_ARG, cla, mtd));
	if(obj == NULL) { return -1; }
	vm_set_package(((pk_object *)obj)->codebase);
	printf("codebase : %ld\n", ((pk_object *)obj)->codebase);
	vm_set_pc(((pk_method *)obj)->offset);
	//printf("offset : %d\n", ((pk_method *)obj)->offset);
	//vm_set_state(VM_STATE_INIT);
	for(int i=0;i<10;i++) {
	   printf("%02x ", (unsigned char)((char *)(((pk_object *)obj)->codebase))[i]);
	}
	printf("\n");
	vm_release_object(VM_ARG, cla);
	vm_release_object(VM_ARG, mtd);
	vm_release_object(VM_ARG, obj);
	return 0;
}

vm_object * vm_load_class_sta(VM_DEF_ARG, uchar * classname) {
	//pk_class * pkc = NULL;
	vm_object * obj = VM_NULL_OBJECT;
	vm_interface iface;
	iface.ptr = NULL;
#ifdef STACK_HOST
	extern pk_class * stack_port_load_class(VM_DEF_ARG, char * name);
	iface.base = stack_port_load_class(VM_ARG, (char *)classname);
#else
	iface.base = lk_select_class(vm_get_info(), classname);
#endif
	if(iface.base != NULL) {
		if(((pk_object *)iface.base)->tag == PK_TAG_CLASS) {
			obj = vm_create_object(VM_ARG, sizeof(vm_interface), (uchar *)&iface);
		} else {
			obj = vm_create_object(VM_ARG, sizeof(vm_interface), (uchar *)&iface);
		}
	}
	return obj;							//invalid class
}

vm_object * vm_load_class(VM_DEF_ARG, vm_object * clsobj) _REENTRANT_ {		
	vm_object * obj = VM_NULL_OBJECT;
	uchar * buffer;
	//implemented 2016.05.15
	buffer = _RECAST(uchar *, m_alloc(vm_get_heap(), clsobj->len + 1));
	memset(buffer, 0, clsobj->len + 1);
	memcpy(buffer, clsobj->bytes, clsobj->len);
	obj = vm_load_class_sta(VM_ARG, buffer);
	m_free(vm_get_heap(), buffer);
	return obj;
}

pk_method * vm_load_method(VM_DEF_ARG, vm_object * clsobj, vm_object * mthobj) {
	uchar * buffer;
	pk_class * pkc = NULL;
	pk_method * pkm = NULL;
	if(clsobj->len == 0) return NULL;
	if(mthobj->len == 0) return NULL;
	pkc = ((pk_class **)clsobj->bytes)[1];
	//printf("class loaded");
	buffer = _RECAST(uchar *, m_alloc(vm_get_heap(), mthobj->len + 1));
	memset(buffer, 0, mthobj->len + 1);
	memcpy(buffer, mthobj->bytes, mthobj->len);
	//printf("method : %s\n", buffer);
	pkm = lk_select_method(pkc, buffer);
	m_free(vm_get_heap(), buffer);
	//if(pkm != NULL) printf("method loaded");
	return pkm;
}

#ifdef __cplusplus_cli
vm_object * vm_interface_exec_sta(VM_DEF_ARG, uint8 num_args, vm_object * iface, uint8 * method_name) {
	static vm_object * null_obj = (vm_object *)VM_NULL_OBJECT;
	vm_object * ret = null_obj;
	return ret;
}

vm_object * vm_interface_exec(VM_DEF_ARG, uint8 num_args, vm_object * iface, vm_object * method) {
	static vm_object * null_obj = (vm_object *)VM_NULL_OBJECT;
	vm_object * ret = null_obj;
	return ret;
}

#else

#if defined(WIN32) && defined(_MSC_VER)
//only for CL32
#include "dlfcn.h"

vm_object * vm_interface_exec_sta(VM_DEF_ARG, uint8 num_args, vm_object * iface, uint8 * method_name) {
	//static vm_object * null_obj = (vm_object *)VM_NULL_OBJECT;
	//va_list vl;
	vm_object * ret = NULL;
	pk_method * pkm;
	vm_object * args[16];
	vm_object * arg;
	pk_object * pkc;
	void * ptemp;
	void * temp;
	void (* func)(void)  = NULL;
	int64 wtemp;
	if(iface->len == 0) return ret;
	pkc = ((pk_object **)iface->bytes)[1];
	if(pkc->tag != PK_TAG_INTERFACE) return ret;
	pkm = pk_select_method((pk_class *)pkc, method_name);
	if(pkm != NULL) {
		if(pkm->callback != NULL) {
			func = pkm->callback;
		} else {
			func = (void (*)(void))dlsym(((pk_object *)pkc)->codebase, (const char *)method_name);
		}
	}
	if(pkm != NULL && pkm->numargs == num_args && func != NULL) {		//method available and total arguments matched
		//typecasting argument if any
		//va_start(vl, method_name);
		for(int i=0;i<num_args;i++) {
			//args[i] = va_arg(vl, vm_object *);
			args[i] = vm_pop_stack_arc(VM_ARG);
			if(args[i]->type != pkm->param_types[(num_args-1) - i]) { //auto-cast variable
				if((args[i]->type & 0x7F) == pkm->param_types[(num_args-1) - i]) 
					continue;
				//if((pkm->param_types[(num_args-1) - i] & 0x7F) == VM_STRING) 
				//	continue;		//skip autocast if requested param type is ref
				arg = args[i];
				args[i] = va_data_cast(VM_ARG, arg, pkm->param_types[(num_args-1) - i]);
				vm_release_object(VM_ARG, arg);
			}
		}
		//va_end(vl);
		//start pushing argument onto stack
		int _esp, _wsp;
		__asm { 
			mov _esp, esp 
			and esp, 0fffffff0h
		}
		for(int i=0;i<num_args;i++) {
			if(pkm->param_types[(num_args-1) - i] & VM_OBJ_TYPE_ARR) {
				temp = args[i]->bytes;
				__asm
			   {
				  mov eax, temp
				  push eax
			   }
			} else {
				switch(pkm->param_types[(num_args-1) - i] & 0x1F) {
					case VM_VAR:
						temp = args[i]->bytes;
						__asm
					   {
						  mov ebx, temp
						  xor eax, eax
						  mov eax, [ebx]
						  push eax
					   }
						break;
					case VM_INT8:			//1 bytes
						temp = args[i]->bytes;
						__asm
					   {
						  mov ebx, temp
						  xor eax, eax
						  mov al, [ebx]
						  push eax
					   }
						break;
					case VM_INT16:			//2 bytes
						temp = args[i]->bytes;
						__asm
					   {
						  mov ebx, temp
						  xor eax, eax
						  mov ax, [ebx]
						  push eax
					   }
						break;
					case VM_INT32:			//4 bytes
					case VM_FLOAT:			//4 bytes
						temp = args[i]->bytes;
						//printf("%f\n", *(float *)temp);
						__asm
					   {
						  mov ebx, temp
						  xor eax, eax
						  mov eax, [ebx]
						  push eax
					   }
						break;
#if defined(__aarch64__) || defined(__x64__) || defined(__x86_64__)
					case VM_INT64:			//8 bytes
#endif
					case VM_DOUBLE:
						temp = args[i]->bytes;
						//printf("%f\n", *(double *)temp);
						__asm
						{
						  mov ebx, temp
						  xor eax, eax
						  mov eax, [ebx +4]
						  push eax
						  mov eax, [ebx ]
						  push eax
						}
						break;
					case VM_STRING:
						temp = args[i]->bytes;
						__asm
					   {
						  mov eax, temp
						  push eax
					   }
						break;
					default: break;
				}
			}
		}
		//push current instance as argument (second argument)
		temp = ((pk_object **)iface->bytes)[0];
		__asm
	   {
		  mov eax, temp
		  push eax
	   }
		//push VM_ARG as last argument (first argument)
		temp = VM_ARG;
		__asm
	   {
		  mov eax, temp
		  push eax
	   }
		temp = &wtemp;
		func();
		__asm
		{
			mov ptemp, eax
			mov ebx, temp
			mov [ebx], eax
			mov [ebx + 4], edx
		}
		__asm { 
			mov esp , _esp 
		}
		//create vm_operand
		if(pkm->ret_type == VM_VAR) {
			ret = (vm_object *)ptemp;
		} else {
			ret = vm_create_operand(VM_ARG, pkm->ret_type, temp, pkm->ret_type & 0x0F);
			if((ret->type & VM_AUTO_POINTER) == VM_AUTO_POINTER && ret->len == sizeof(void *)) {
				ret->release = vm_iptr_release;
			}
		}
		//force release argument stack
		//printf("executed successfully\n");
		for(int i=0;i<num_args;i++) {
			vm_release_object(VM_ARG, args[i] );
		}
	}
	return ret;
}
vm_object * vm_interface_exec(VM_DEF_ARG, uint8 num_args, vm_object * iface, vm_object * method) {
	vm_object * ret;
	//va_list vl;
	uint8 buffer[256];
	//vm_object * args[20];
	if(method->len == 0) return ret;
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, method->bytes, method->len);
	for(int i =0;i<method->len;i++) if(buffer[i] == '?') buffer[i] = 0;
	//printf("method name : %s\n", buffer);
	//for(int i=0; i<num_args; i++) {
	//	args[i] = vm_pop_stack_arc(VM_ARG);
		//va_arg(vl, vm_object *) = vm_pop_stack_arc(VM_ARG);
	//}
	ret =  vm_interface_exec_sta(VM_ARG, num_args, iface, buffer);
	if(ret == NULL) {
		ret = vm_create_object(VM_ARG, 0, NULL);
	}
	return ret;
}
#endif
#endif

vm_object * vm_load_constant(VM_DEF_ARG, uint32 offset) {
	vm_object * obj = NULL;
	uint8 llen;
	uint8 lbuf[4];
	uint16 len = 0;
	uchar * buffer = NULL;
	vm_fetch(VM_ARG, offset, lbuf, 4);
	if(lbuf[0] < 128) { len = lbuf[0]; llen = 1; }
	else {
		switch(lbuf[0] & 0x0F) {
			case 0x01: len = lbuf[1]; llen = 2; break;
			case 0x02: len = (lbuf[1] << 8) | lbuf[2]; llen = 3; break;
			default: break;
		}
	}
	if(len > VA_OBJECT_MAX_SIZE) {
		vm_invoke_exception(VM_ARG, VX_OUT_OF_BOUNDS);
		return VM_NULL_OBJECT;
	}
	buffer = _RECAST(uchar *, m_alloc(vm_get_heap(), len + 1));
	buffer[len] = 0;
	vm_fetch(VM_ARG, offset + llen, buffer, len);		//obj = jumptable
	//printf("load constant %s\n", buffer);
	//printf("constant : %s\n", buffer);
	obj = _RECAST(vm_object *, vm_create_object(VM_ARG, len , buffer));
	obj->mgc_refcount--;
	m_free(vm_get_heap(), buffer);
	return obj;
}

uint16 vm_object_get_length(VM_DEF_ARG, vm_object * obj) {
	uint16 len = 0;
	uint8 buffer[64];
	switch(vm_object_get_type(obj)) {
		case VM_MAGIC:
			len = obj->len;
			break;
		case VM_EXT_MAGIC:
			len = ((vm_extension *)obj->bytes)->apis->text(VM_ARG, obj, buffer);
			break;
	}
	return len;
}

static void vm_interface_release(void * ctx, void * ptr) {
	if (ptr != NULL) {
		vm_interface * iface = ((vm_interface *)ptr);
		if (iface == NULL) return;
		if (iface->ptr == NULL) return;
		//free(iface->ptr);
		//delete iface->ptr;
	}
}

void vm_remove_immutable_object(VM_DEF_ARG, vm_object * obj) {
	vm_object * iterator = NULL;
	vm_object * prev_iterator = NULL;
	iterator = VM_ARG->object_list;
	while(iterator != NULL) {
		if (iterator == obj) {
			if (prev_iterator == NULL) VM_ARG->object_list = obj->next;
			else prev_iterator = obj->next;
			break;
		}
		prev_iterator = iterator;
		iterator = iterator->next;
	}
}

void vm_gc_immutable_object(VM_DEF_ARG) {
	vm_object * iterator = NULL;
	vm_object * prev_iterator = NULL;
	vm_object * candidate = NULL;
	iterator = VM_ARG->object_list;
	while (iterator != NULL) {
		if ((iterator->mgc_refcount & VM_MAGIC_MASK_ZERO) == VM_MAGIC) {
			if (prev_iterator == NULL) VM_ARG->object_list = iterator->next;
			else prev_iterator = iterator->next;
			candidate = iterator;
		}
		prev_iterator = iterator;
		iterator = iterator->next;
		if (candidate != NULL) {
			//printf("freed immutable object %d bytes\n", candidate->len);
			free(candidate);
			candidate = NULL;
		}
	}
}

uint8 vm_ext_get_tag(vm_object * obj) {
	if(vm_object_get_type(obj) != VM_EXT_MAGIC) return ASN_TAG_OCTSTRING;
	return ((vm_extension *)(obj->bytes))->tag;
}

vm_object * vm_size_object(VM_DEF_ARG, vm_object * op1) {
	uchar objlen = op1->len;
	uchar buffer[5];
	sprintf(_RECAST(char *, buffer), "%i", objlen);
	return vm_create_object(VM_ARG, strlen(_RECAST(const char *, buffer)), buffer);
}

uint8 vm_is_numeric(vm_object * obj) {
	if (obj->type != VM_OBJ_TYPE_ANY) return TRUE;
	return vm_is_numeric_s(obj->bytes, obj->len);
}

uint8 vm_is_numeric_s(uint8 * buffer, int len) { 
	uint8 n;
	uint8 dot = 0;
	if(len == 0) return FALSE;
	if(len > 64) return FALSE;
	while(len != 0 ) {
		n = buffer[--len];
		if((n == '-') && (len == 0)) break;
		if(n == '.' && dot == 0) { dot++; break; }
		if(n == 0) continue;
		if(n > 0x39 || n < 0x30) { return FALSE; }
	} 
	return TRUE;
}

uint8 vm_is_precision(uint8 * buffer, int len) {
	uint8 n;
	uint8 dot = 0;
	if (len == 0) return FALSE;
	if (len > 64) return FALSE;
	while (len != 0) {
		n = buffer[--len];
		if ((n == '-') && (len == 0)) break;
		if (n == '.' && dot == 0) { dot++; break; }
		if (n > 0x39 || n < 0x30) { return FALSE; }
	}
	if (dot == 1) return TRUE;
	return FALSE;
}

vm_object * vm_split_object(VM_DEF_ARG, vm_object * op2, vm_object * op1, vm_object * target) {
	uchar offset, len;
	vm_object * obj;
	uchar * opd1, * opd2;
	opd1 = _RECAST(uchar *, m_alloc(vm_get_heap(), op1->len + 1));
	opd2 = _RECAST(uchar *, m_alloc(vm_get_heap(), op2->len + 1));
	memcpy(opd1, op1->bytes, op1->len); opd1[op1->len] = 0;		//null terminated string
	memcpy(opd2, op2->bytes, op2->len); opd2[op2->len] = 0;		//null terminated string
	if(vm_is_numeric_s(op1->bytes, op1->len) == FALSE) { offset = 0; } else { offset = atoi(_RECAST(const char *, opd1)); }			//
	if(vm_is_numeric_s(op2->bytes, op2->len) == FALSE) { len = target->len; } else { len = atoi(_RECAST(const char *, opd2)); }
	if(len > (target->len - offset)) len = (target->len - offset);
	m_free(vm_get_heap(), opd1);
	m_free(vm_get_heap(), opd2);//
	obj = vm_create_object(VM_ARG, len, target->bytes + offset);
	//printf("new size : %d, address : %08x\n", m_size_chunk(obj), obj);
	//memcpy(obj->bytes, target->bytes + offset, len);
	//printf("address : %08x\n", obj);
	return obj;
}

vm_object * vm_operation_object(VM_DEF_ARG, uchar opcode, vm_object * op2, vm_object * op1) {
	int32 value1, value2;
	uint8 valbuf1[64];
	uint8 valbuf2[64];
	uint8 vb1len;
	uint8 vb2len;
	uint16 vb2offset;
	uint16 len;
	uchar * buffer = NULL;
	uchar * obj;
	vm_object * opr1;
	vm_object * opr2;
	double temp_d, d1, d2;
	int64 temp_w, w1, w2;
	int32 current_sp;
	int use_precision = FALSE;
	vm_extension * obj_ext;
	vm_object * (*func)(VM_DEF_ARG, vm_object *, vm_object *) = NULL;
	memset(valbuf1, 0, sizeof(valbuf1));
	memset(valbuf2, 0, sizeof(valbuf2));
	//check for array/object
	if (vm_object_get_type(op1) == VM_OBJ_MAGIC) {
		//array/object operation
		obj = (uint8 *)va_arg_operation(VM_ARG, opcode, op1, op2);
	}
	//check for operand
	else if( (opcode == INS_ADD && vm_object_get_type(op1) == VM_OPR_MAGIC && vm_object_get_type(op2) == VM_OPR_MAGIC) || 
		 (opcode != INS_ADD && (vm_object_get_type(op1) == VM_OPR_MAGIC || vm_object_get_type(op2) == VM_OPR_MAGIC))
		) {
		if(vm_object_get_type(op1) == VM_OPR_MAGIC) {
			memcpy(valbuf1, op1->bytes, op1->len);
			switch(op1->type & 0x3F) {
				case VM_INT8: d1 = w1 = *(int8 *)valbuf1; break;
				case VM_INT16: d1 = w1 = *(int16 *)valbuf1; break;
				case VM_INT32: d1 = w1 = *(int32 *)valbuf1; break;
#if defined(__aarch64__) || defined(__x64__)
				case VM_INT64: d1 = w1 = *(int64 *)valbuf1; break;
#endif
				case VM_FLOAT: w1 = d1 = *(float *)valbuf1; use_precision = TRUE; break;
				case VM_DOUBLE: w1 = d1 = *(double *)valbuf1; use_precision = TRUE; break;
			}
		} else { 
			opr1 = va_data_cast(VM_ARG, op1, VM_INT64);
			memcpy(valbuf2, opr1->bytes, opr1->len);
			vm_release_object(VM_ARG, opr1);
			w1 = d1 = *(int64 *)valbuf1;
		}
		if(vm_object_get_type(op2) == VM_OPR_MAGIC) { 
			memcpy(valbuf2, op2->bytes, op2->len);
			switch(op2->type & 0x3F) {
				case VM_INT8: d2 = w2 = *(int8 *)valbuf2; break;
				case VM_INT16: d2 = w2 = *(int16 *)valbuf2; break;
				case VM_INT32: d2 = w2 = *(int32 *)valbuf2; break;
#if defined(__aarch64__) || defined(__x64__)
				case VM_INT64: d2 = w2 = *(int64 *)valbuf2; break;
#endif
				case VM_FLOAT: w2 = d2 = *(float *)valbuf2; use_precision = TRUE; break;
				case VM_DOUBLE: w2 = d2 = *(double *)valbuf2; use_precision = TRUE; break;
			}
		} else {
			if (use_precision) {
				opr2 = va_data_cast(VM_ARG, op2, VM_DOUBLE);
				memcpy(valbuf2, opr2->bytes, opr2->len);
				vm_release_object(VM_ARG, opr2);
				w2 = d2 = *(double *)valbuf2;
			} else {
				opr2 = va_data_cast(VM_ARG, op2, VM_INT64);
				memcpy(valbuf2, opr2->bytes, opr2->len);
				vm_release_object(VM_ARG, opr2);
				w2 = d2 = *(int64 *)valbuf2;
			}
		}
		switch(opcode) {
			case INS_ADD: 
				if(use_precision) 
					temp_d = d1 + d2; 
				else
					temp_w = w1 + w2;
					break;
			case INS_SUB: 
				if(use_precision) 
					temp_d = d1 - d2; 
				else
					temp_w = w1 - w2;
					break;
			case INS_MUL: 
				if(use_precision) 
					temp_d = d1 * d2; 
				else
					temp_w = w1 * w2;
					break;
			case INS_DIV: 
				if(use_precision) 
					temp_d = d1 / d2; 
				else
					temp_w = w1 / w2;
					break;
			case INS_MOD: temp_w = w1 % w2; use_precision = FALSE; break;
			case INS_AND: temp_w = w1 & w2; use_precision = FALSE; break;
			case INS_OR: temp_w = w1 | w2; use_precision = FALSE; break;
			case INS_XOR: temp_w = w1 ^ w2; use_precision = FALSE; break;
			case INS_SHL: temp_w = w1 << w2; use_precision = FALSE; break;
			case INS_SHR: temp_w = w1 >> w2; use_precision = FALSE; break;
			case INS_NOT: temp_w = !w1; use_precision = FALSE; break;
			default: break;
		}
		if(use_precision) {
			obj = _RECAST(uchar *, vm_create_operand(VM_ARG, VM_DOUBLE, (uint8 *)&temp_d, sizeof(double)));
		} else {
			obj = _RECAST(uchar *, vm_create_operand(VM_ARG, VM_INT64, (uint8 *)&temp_w, sizeof(int64)));
		}
	}
	//check for extension
	else if(vm_object_get_type(op1) == VM_EXT_MAGIC || vm_object_get_type(op2) == VM_EXT_MAGIC) {
		if (vm_object_get_type(op2) == VM_EXT_MAGIC) obj_ext = (vm_extension *)op2->bytes;
		if (vm_object_get_type(op1) == VM_EXT_MAGIC) obj_ext = (vm_extension *)op1->bytes;		//op1 would be priority
		switch(opcode) {
			case INS_ADD: func = obj_ext->apis->_add; break;
			case INS_SUB: func = obj_ext->apis->_sub; break;
			case INS_MUL: func = obj_ext->apis->_mul; break;
			case INS_DIV: func = obj_ext->apis->_div; break;
			case INS_MOD: func = obj_ext->apis->_mod; break;
			case INS_AND: func = obj_ext->apis->_and; break;
			case INS_OR: func = obj_ext->apis->_or; break;
			case INS_XOR: func = obj_ext->apis->_xor; break;
			case INS_SHL: func = obj_ext->apis->_shl; break;
			case INS_SHR: func = obj_ext->apis->_shr; break;
			case INS_NOT: obj = (uchar *)obj_ext->apis->_not(VM_ARG, op1); break;
			default: break;
		}
		if (func != NULL) {
			obj = (uchar *)func(VM_ARG, op1, op2);
		}
	}
	//default operation auto type
	else {
		switch(op1->type) {
			case VM_OBJ_TYPE_INSTANCE:
				break;
			case VM_OBJ_TYPE_INTERFACE:
				//expand pointer to data type and push to stack
				//vm_push_stack_arc(VM_ARG, vm_create_object(VM_ARG, sizeof(void *), (uchar *)&((vm_interface *)op1->bytes)->ptr));
				current_sp = vm_get_sp();
				vm_push_stack_arc(VM_ARG, op2);
				switch(opcode) {
					//arith operation
					case INS_ADD: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "+")); break;
					case INS_SUB: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "-")); break;
					case INS_MUL: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "*")); break;
					case INS_DIV: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "/")); break;
					case INS_MOD: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "%")); break;
					//logical operation
					case INS_AND: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "&")); break;
					case INS_OR: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "|")); break;
					case INS_XOR: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "^")); break;
					case INS_SHL: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "<<")); break;
					case INS_SHR: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, ">>")); break;
					case INS_NOT: 
						op2 = vm_pop_stack_arc(VM_ARG);
						obj = (uchar *)vm_interface_exec_sta(VM_ARG, 0, op1, _RECAST(uint8 *, "!")); 
						break;
					default: break;
				}
				//restore to saved sp
				while(vm_get_sp() > current_sp) {
					op2 = vm_pop_stack_arc(VM_ARG);
				}
				if(obj == NULL) {
					//op2 = vm_pop_stack_arc(VM_ARG);
					//op2 = vm_pop_stack_arc(VM_ARG);
					//no method available, return empty object
					obj = (uchar *)vm_create_object(VM_ARG, 0, NULL);
				}
				break;
			
			default:
				if(vm_is_numeric_s(op1->bytes, op1->len) == TRUE && vm_is_numeric_s(op2->bytes, op2->len) == TRUE) {		//number
					buffer = _RECAST(uchar *, m_alloc(vm_get_heap(), 12));
					memcpy(buffer, op1->bytes, op1->len);
					buffer[op1->len] = 0;
					value1 = atoi(_RECAST(const char *, buffer));
					memcpy(buffer, op2->bytes, op2->len);
					buffer[op2->len] = 0;
					value2 = atoi(_RECAST(const char *, buffer));
					switch(opcode) {
						case INS_ADD: value1 = value1 + value2; break;
						case INS_SUB: value1 = value1 - value2; break;
						case INS_MUL: value1 = value1 * value2; break;
						case INS_DIV: 
							if(value2 == 0) {
								vm_invoke_exception(VM_ARG, VX_DIV_BY_ZERO);
								break;
							}
							value1 = value1 / value2; 
							break;
						case INS_MOD: value1 = value1 % value2; break;

						//logical operation
						case INS_AND: value1 = value1 & value2; break;
						case INS_OR: value1 = value1 | value2; break;
						case INS_XOR: value1 = value1 ^ value2; break;
						case INS_SHL: value1 = value1 << value2; break;
						case INS_SHR: value1 = value1 >> value2; break;
						case INS_NOT: value1 = !value1; break;
						default: break;
					}
					sprintf(_RECAST(char *, buffer), "%d", value1);
					obj = _RECAST(uchar *, vm_create_object(VM_ARG, strlen(_RECAST(const char *, buffer)), buffer));
					m_free(vm_get_heap(), buffer);
				}
				//default string operation
				else {		//string
					switch(opcode) {
						case INS_ADD:
							len = 0;
							vb1len = 0;
							vb2len = 0;
							if(vm_object_get_type(op1) == VM_EXT_MAGIC) {
								vb1len = ((vm_extension *)op1->bytes)->apis->text(VM_ARG, op1, valbuf1);
								len += vb1len;
							} else if(vm_object_get_type(op1) == VM_OPR_MAGIC) {
								opr1 = va_data_cast(VM_ARG, op1, 0);
								memcpy(valbuf1, opr1->bytes, opr1->len);
								vb1len = opr1->len;
								//printf("valbuf1 : %s(%d)\n", valbuf1, vb1len);
								vm_release_object(VM_ARG, opr1);
								len += vb1len;
							} else { len += op1->len; }
							vb2offset = len;
							if(vm_object_get_type(op2) == VM_EXT_MAGIC) {
								vb2len = ((vm_extension *)op2->bytes)->apis->text(VM_ARG, op2, valbuf2);
								len += vb2len;
							} else if(vm_object_get_type(op2) == VM_OPR_MAGIC) {
								opr2 = va_data_cast(VM_ARG, op2, 0);
								memcpy(valbuf2, opr2->bytes, opr2->len);
								vb2len = opr2->len;
								//printf("valbuf2 : %s(%d)\n", valbuf2, vb2len);
								vm_release_object(VM_ARG, opr2);
								len += vb2len;
							} else { len += op2->len; }
							obj = _RECAST(uchar *, vm_create_object(VM_ARG, len, buffer));
							if(vb1len == 0) memcpy(((vm_object *)obj)->bytes, op1->bytes, op1->len); 
							else memcpy(((vm_object *)obj)->bytes, valbuf1, vb1len); 
							if(vb2len == 0) memcpy(((vm_object *)obj)->bytes + vb2offset, op2->bytes, op2->len); 
							else memcpy(((vm_object *)obj)->bytes + vb2offset, valbuf2, vb2len); 
							break;
						default:
							obj = _RECAST(uchar *, vm_create_object(VM_ARG, 3, _RECAST(uchar *, "NaN")));
							break;
					}
				}
				break;
		}
	}
	return _RECAST(vm_object *, obj);
}

//deprecated APIs
uchar vm_cmp_object(VM_DEF_ARG, vm_object * op2, vm_object * op1) {
	int value1, value2;
	double d1 = 0, d2 = -1;
	int w1 = 0, w2 = ~w1;
	uchar * buffer = NULL;
	uchar * obj; 
	uchar opd1[12];
	uchar opd2[12];
	uchar use_precision = FALSE;
	vm_object * opr1, * opr2;
	int ptype = VM_INT64;
	//use auto-casting for operand object
	if (vm_object_get_type(op1) == VM_OPR_MAGIC || vm_object_get_type(op2) == VM_OPR_MAGIC) {
		if (vm_object_get_type(op1) == VM_OPR_MAGIC) {
			switch (op1->type & 0x3F) {
			case VM_INT8: d1 = w1 = *(int8 *)op1->bytes; break;
			case VM_INT16: d1 = w1 = *(int16 *)op1->bytes; break;
			case VM_INT32: d1 = w1 = *(int32 *)op1->bytes; break;
#if defined(__aarch64__) || defined(__x64__)
			case VM_INT64: d1 = w1 = *(int64 *)op1->bytes; break;
#endif
			case VM_FLOAT: w1 = d1 = *(float *)op1->bytes; use_precision = TRUE; break;
			case VM_DOUBLE: w1 = d1 = *(double *)op1->bytes; use_precision = TRUE; break;
			}
		} else {
			if (op1->len != 0) {
				opr1 = va_data_cast(VM_ARG, op1, VM_INT64);
				d1 = *(double *)opr1->bytes;
				w1 = *(int64 *)opr1->bytes;
				vm_release_object(VM_ARG, opr1);
			}
		}
		if (vm_object_get_type(op2) == VM_OPR_MAGIC) {
			//memcpy(valbuf2, op2->bytes, op2->len);
			switch (op2->type & 0x3F) {
			case VM_INT8: d2 = w2 = *(int8 *)op2->bytes; break;
			case VM_INT16: d2 = w2 = *(int16 *)op2->bytes; break;
			case VM_INT32: d2 = w2 = *(int32 *)op2->bytes; break;
#if defined(__aarch64__) || defined(__x64__)
			case VM_INT64: d2 = w2 = *(int64 *)op2->bytes; break;
#endif
			case VM_FLOAT: w2 = d2 = *(float *)op2->bytes; use_precision = TRUE; break;
			case VM_DOUBLE: w2 = d2 = *(double *)op2->bytes; use_precision = TRUE; break;
			}
		} else {
			w2 = d2 = ~w1;
			if (use_precision) ptype = VM_DOUBLE;
			if (op2->len != 0) {
				opr2 = va_data_cast(VM_ARG, op2, ptype);
				d2 = *(double *)opr2->bytes;
				w2 = *(int64 *)opr2->bytes;
				vm_release_object(VM_ARG, opr2);
			}
		}
		//use precision if necessary when one of the operand is precision type
		if (use_precision) {
			d1 = (d1 - d2);
			if (d1 == 0) value1 = 0;
			else if (d1 < 0) value1 = -1;
			else value1 = 1;
		}
		else value1 = w1 - w2;
	}
	else {
		//check for numeric
		if (vm_is_numeric_s(op1->bytes, op1->len) == TRUE && vm_is_numeric_s(op2->bytes, op2->len) == TRUE) {		//number

			memcpy(opd1, op1->bytes, op1->len); opd1[op1->len] = 0;
			memcpy(opd2, op2->bytes, op2->len); opd2[op2->len] = 0;
			value1 = atoi(_RECAST(const char *, opd1));
			value2 = atoi(_RECAST(const char *, opd2));
			//printf("%d - %d\n", value1, value2);
			value1 = value1 - value2;
		}
		//use string compare
		else {		//string
			value1 = memcmp(op1->bytes, op2->bytes, ((op1->len > op2->len) ? op1->len : op2->len));
		}
	}
	if(value1 == 0) {
		return VM_CMP_EQ;
	} else if(value1 < 0) {
		return VM_CMP_LT;
	} else {
		return VM_CMP_GT;
	}
}

vm_object * vm_compare_object(VM_DEF_ARG, uint8 opcode, vm_object * op2, vm_object * op1) {
	int32 value1, value2;
	uint8 valbuf1[64];
	uint8 valbuf2[64];
	uint8 vb1len;
	uint8 vb2len;
	uint16 vb2offset;
	uint16 len;
	uchar * buffer = NULL;
	uchar * obj = NULL;
	vm_object * opr1;
	vm_object * opr2;
	double temp_d, d1, d2;
	int64 temp_w, w1, w2;
	int32 current_sp;
	uchar psw;
	int use_precision = FALSE;
	vm_extension * obj_ext;
	vm_object * (*func)(VM_DEF_ARG, vm_object *, vm_object *) = NULL;
	memset(valbuf1, 0, sizeof(valbuf1));
	memset(valbuf2, 0, sizeof(valbuf2));
	//check for array/object
	if (vm_object_get_type(op1) == VM_OBJ_MAGIC) {
		//array/object compare
		obj = (uint8 *)va_arg_operation(VM_ARG, opcode, op1, op2);
	}
	//check for operand
	else if (op1->len != 0 && op2->len != 0 && (vm_object_get_type(op1) == VM_OPR_MAGIC || vm_object_get_type(op2) == VM_OPR_MAGIC)) {
		if (vm_object_get_type(op1) == VM_OPR_MAGIC) {
			memcpy(valbuf1, op1->bytes, op1->len);
			switch (op1->type & 0x3F) {
				case VM_INT8: d1 = w1 = *(int8 *)valbuf1; break;
				case VM_INT16: d1 = w1 = *(int16 *)valbuf1; break;
				case VM_INT32: d1 = w1 = *(int32 *)valbuf1; break;
#if defined(__aarch64__) || defined(__x64__)
				case VM_INT64: d1 = w1 = *(int64 *)valbuf1; break;
#endif
				case VM_FLOAT: w1 = d1 = *(float *)valbuf1; use_precision = TRUE; break;
				case VM_DOUBLE: w1 = d1 = *(double *)valbuf1; use_precision = TRUE; break;
			}
		}
		else {
			opr1 = va_data_cast(VM_ARG, op1, VM_INT64);
			memcpy(valbuf2, opr1->bytes, opr1->len);
			vm_release_object(VM_ARG, opr1);
			w1 = d1 = *(int64 *)valbuf1;
		}
		if (vm_object_get_type(op2) == VM_OPR_MAGIC) {
			memcpy(valbuf2, op2->bytes, op2->len);
			switch (op2->type & 0x3F) {
				case VM_INT8: d2 = w2 = *(int8 *)valbuf2; break;
				case VM_INT16: d2 = w2 = *(int16 *)valbuf2; break;
				case VM_INT32: d2 = w2 = *(int32 *)valbuf2; break;
#if defined(__aarch64__) || defined(__x64__)
				case VM_INT64: d2 = w2 = *(int64 *)valbuf2; break;
#endif
				case VM_FLOAT: w2 = d2 = *(float *)valbuf2; use_precision = TRUE; break;
				case VM_DOUBLE: w2 = d2 = *(double *)valbuf2; use_precision = TRUE; break;
			}
		}
		else {
			if (use_precision) {
				opr2 = va_data_cast(VM_ARG, op2, VM_DOUBLE);
				memcpy(valbuf2, opr2->bytes, opr2->len);
				vm_release_object(VM_ARG, opr2);
				w2 = d2 = *(double *)valbuf2;
			}
			else {
				opr2 = va_data_cast(VM_ARG, op2, VM_INT64);
				memcpy(valbuf2, opr2->bytes, opr2->len);
				vm_release_object(VM_ARG, opr2);
				w2 = d2 = *(int64 *)valbuf2;
			}
		}
		if (use_precision) {
			temp_d = d1 - d2;
			if (temp_d == 0) {
				psw = VM_CMP_EQ;
			} else if (temp_d < 0) {
				psw = VM_CMP_LT;
			} else {
				psw = VM_CMP_GT;
			}
		}
		else {
			temp_w = w1 - w2;
			if (temp_w == 0) {
				psw = VM_CMP_EQ;
			} else if (temp_w < 0) {
				psw = VM_CMP_LT;
			} else {
				psw = VM_CMP_GT;
			}
		}
		switch (opcode) {
			//compare operation
			case INS_CREQ: obj = (uchar *)vm_load_bool(VM_ARG, (psw & VM_CMP_EQ)); break;
			case INS_CRNE: obj = (uchar *)vm_load_bool(VM_ARG, (psw & VM_CMP_EQ) == 0); break;
			case INS_CRGT: obj = (uchar *)vm_load_bool(VM_ARG, (psw == VM_CMP_GT)); break;
			case INS_CRLT: obj = (uchar *)vm_load_bool(VM_ARG, (psw == VM_CMP_LT)); break;
			case INS_CRGTEQ: obj = (uchar *)vm_load_bool(VM_ARG, (psw != VM_CMP_LT)); break;
			case INS_CRLTEQ: obj = (uchar *)vm_load_bool(VM_ARG, (psw != VM_CMP_GT)); break;
			default: obj = NULL; break;
		}
	}
	//check for extension
	else if (vm_object_get_type(op1) == VM_EXT_MAGIC || vm_object_get_type(op2) == VM_EXT_MAGIC) {
		if (vm_object_get_type(op2) == VM_EXT_MAGIC) obj_ext = (vm_extension *)op2->bytes;
		if (vm_object_get_type(op1) == VM_EXT_MAGIC) obj_ext = (vm_extension *)op1->bytes;		//op1 would be priority
		func = obj_ext->apis->_sub;
		if (func != NULL) {
			obj = (uchar *)func(VM_ARG, op1, op2);
		}
	}
	//default operation auto type
	else {
		switch (op1->type) {
		case VM_OBJ_TYPE_INSTANCE:
			break;
#if 0
		case VM_OBJ_TYPE_INTERFACE:
			//expand pointer to data type and push to stack
			//vm_push_stack_arc(VM_ARG, vm_create_object(VM_ARG, sizeof(void *), (uchar *)&((vm_interface *)op1->bytes)->ptr));
			current_sp = vm_get_sp();
			vm_push_stack_arc(VM_ARG, op2);
			switch (opcode) {
				//compare operation
				case INS_CREQ: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "==")); break;
				case INS_CRNE: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "!=")); break;
				case INS_CRLT: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "<")); break;
				case INS_CRGT: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, ">")); break;
				case INS_CRLTEQ: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, "<=")); break;
				case INS_CRGTEQ: obj = (uchar *)vm_interface_exec_sta(VM_ARG, 1, op1, _RECAST(uint8 *, ">=")); break;
				default: break;
			}
			//restore to saved sp
			while (vm_get_sp() > current_sp) {
				op2 = vm_pop_stack_arc(VM_ARG);
			}
			//if (((vm_object *)obj)->len == 0) goto default_compare;
			break;
#endif
		default:
		default_compare:
			if (vm_is_numeric_s(op1->bytes, op1->len) == TRUE && vm_is_numeric_s(op2->bytes, op2->len) == TRUE) {		//number
				buffer = _RECAST(uchar *, m_alloc(vm_get_heap(), 12));
				memcpy(buffer, op1->bytes, op1->len);
				buffer[op1->len] = 0;
				value1 = atoi(_RECAST(const char *, buffer));
				memcpy(buffer, op2->bytes, op2->len);
				buffer[op2->len] = 0;
				value2 = atoi(_RECAST(const char *, buffer));
				value1 = value1 - value2;
				if (value1 == 0) {
					psw = VM_CMP_EQ;
				}
				else if (value1 < 0) {
					psw = VM_CMP_LT;
				}
				else {
					psw = VM_CMP_GT;
				}
				switch (opcode) {
					//compare operation
					case INS_CREQ: obj = (uchar *)vm_load_bool(VM_ARG, (psw & VM_CMP_EQ)); break;
					case INS_CRNE: obj = (uchar *)vm_load_bool(VM_ARG, (psw & VM_CMP_EQ) == 0); break;
					case INS_CRGT: obj = (uchar *)vm_load_bool(VM_ARG, (psw == VM_CMP_GT)); break;
					case INS_CRLT: obj = (uchar *)vm_load_bool(VM_ARG, (psw == VM_CMP_LT)); break;
					case INS_CRGTEQ: obj = (uchar *)vm_load_bool(VM_ARG, (psw != VM_CMP_LT)); break;
					case INS_CRLTEQ: obj = (uchar *)vm_load_bool(VM_ARG, (psw != VM_CMP_GT)); break;
					default: obj = NULL; break;
				}
				//sprintf(_RECAST(char *, buffer), "%i", value1);
				//obj = _RECAST(uchar *, vm_create_object(VM_ARG, strlen(_RECAST(const char *, buffer)), buffer));
				m_free(vm_get_heap(), buffer);
			}
			//default string compare
			else {		//string
				value1 = memcmp(op1->bytes, op2->bytes, ((op1->len > op2->len) ? op1->len : op2->len));
				if (value1 == 0) {
					psw = VM_CMP_EQ;
				}
				else if (value1 < 0) {
					psw = VM_CMP_LT;
				}
				else {
					psw = VM_CMP_GT;
				}
				switch (opcode) {
					//compare operation
					case INS_CREQ: obj = (uchar *)vm_load_bool(VM_ARG, (psw & VM_CMP_EQ)); break;
					case INS_CRNE: obj = (uchar *)vm_load_bool(VM_ARG, (psw & VM_CMP_EQ) == 0); break;
					case INS_CRGT: obj = (uchar *)vm_load_bool(VM_ARG, (psw == VM_CMP_GT)); break;
					case INS_CRLT: obj = (uchar *)vm_load_bool(VM_ARG, (psw == VM_CMP_LT)); break;
					case INS_CRGTEQ: obj = (uchar *)vm_load_bool(VM_ARG, (psw != VM_CMP_LT)); break;
					case INS_CRLTEQ: obj = (uchar *)vm_load_bool(VM_ARG, (psw != VM_CMP_GT)); break;
					default: obj = NULL; break;
				}
			}
			break;
		}
	}
	if (obj == NULL) {
		//no method available, return empty object
		obj = (uchar *)vm_create_object(VM_ARG, 0, NULL);
	}
	return _RECAST(vm_object *, obj);
}

#define VM_GC_DEBUG		0
void vm_garbage_collect(VM_DEF_ARG) _REENTRANT_ {									//reference counting garbage collector
	vm_object * candidate = NULL;
#if VM_GC_DEBUG
	uint16 a_heap, b_heap, i = 0;
#endif
	vm_object * nextchunk = NULL;
	vm_object * shifted = NULL;
	vm_object * iterator = (vm_object *) m_first_chunk(vm_get_heap());
	void * auto_ptr;
#if VM_GC_DEBUG
	b_heap = m_get_allocated_space();
#endif
	while(iterator != NULL) {
		if((iterator->mgc_refcount & VM_MAGIC_MASK_ZERO) == VM_MAGIC) {				//check vm_object reference counter == 0
			candidate = iterator;
		}
		nextchunk = (vm_object *)m_next_chunk(vm_get_heap(), iterator);
		if(nextchunk == NULL) break;
		if((nextchunk->mgc_refcount & VM_MAGIC_MASK_ZERO) == VM_MAGIC && nextchunk->lock == 0) { 		//check vm_object for shifting
			shifted = _RECAST(vm_object *, m_shift_next_chunk(vm_get_heap(), iterator));
			if(shifted != nextchunk) {
				vm_update_mutator(VM_ARG, nextchunk, shifted);
#if VM_GC_DEBUG
				//printf("shift chunk %08x to %08x\n", nextchunk, shifted);
#endif
				nextchunk = shifted;
			}
		}
		iterator = nextchunk;
		if(candidate != NULL) {
#if VM_GC_DEBUG 
			i++;
#endif
			if(candidate->release != NULL) {
				candidate->release(VM_ARG, candidate->bytes);
			}
			//printf("[GarbageCollector] : clear an object %d bytes\n", m_size_chunk(vm_get_heap(), candidate));
			memset(candidate, 0, m_size_chunk(vm_get_heap(), candidate));
			m_free(vm_get_heap(), candidate);
			candidate = NULL;
		}
	}
#if VM_GC_DEBUG
	a_heap = m_get_allocated_space();
	if(i != 0) {
		printf("freed up %i variables, heap : %d -> %d bytes\n", i, b_heap, a_heap);
	}
#endif
	vm_gc_immutable_object(VM_ARG);		//garbage collect immutable object
	//printf("allocated memory : %d\n", m_get_allocated_space(vm_get_heap()));
	//getchar();
}

#if 0
void vm_garbage_collect2(VM_DEF_ARG) {									//reference counting garbage collector
	vm_object * candidate = NULL;
#if VM_GC_DEBUG
	uint16 a_heap, b_heap, i = 0;
#endif
	vm_object * nextchunk = NULL;
	vm_object * shifted = NULL;
	vm_object * iterator = _RECAST(vm_object *, m_first_chunk(vm_get_heap()));
#if VM_GC_DEBUG
	b_heap = m_get_allocated_space();
#endif
	while(iterator != NULL) {
		if(iterator->mgc_refcount == VM_MAGIC) {				//check vm_object reference counter == 0
			candidate = iterator;
		}
		nextchunk = _RECAST(vm_object *, m_next_chunk(vm_get_heap(), iterator));
		if((iterator->mgc_refcount & VM_MAGIC_MASK) == VM_MAGIC && nextchunk->lock == 0) { 		//check vm_object for shifting
			shifted = _RECAST(vm_object *, m_shift_next_chunk(vm_get_heap(), iterator));
			if(shifted != nextchunk) {
				vm_update_mutator(VM_ARG, nextchunk, shifted);
#if VM_GC_DEBUG
				printf("shift chunk %08x to %08x\n", nextchunk, shifted);
#endif
				nextchunk = shifted;
			}
		}
		iterator = nextchunk;
		if(candidate != NULL) {
#if VM_GC_DEBUG 
			i++;
#endif
			//printf("[GarbageCollector] : clear an object %d bytes\n", m_size_chunk(vm_get_heap(), candidate));
			memset(candidate, 0, m_size_chunk(vm_get_heap(), candidate));
			m_free(vm_get_heap(), candidate);
			candidate = NULL;
		}
	}
#if VM_GC_DEBUG
	a_heap = m_get_allocated_space();
	if(i != 0) {
		printf("freed up %i variables, heap : %d -> %d bytes\n", i, b_heap, a_heap);
	}
#endif
	vm_gc_immutable_object(VM_ARG);		//garbage collect immutable object
	//printf("allocated memory : %d\n", m_get_allocated_space(vm_get_heap()));
}
#endif

void vm_update_mutator(VM_DEF_ARG, vm_object * old_addr, vm_object * new_addr) {									//reallign all objects on heap
	uint16 i = 0;
	for(i =0 ;i<VM_MAX_STACK_SIZE; i++) {
		if((vm_object *)vm_stack(i) == old_addr) vm_stack(i) = new_addr;
	}
}

vm_object * vm_get_argument(VM_DEF_ARG, uchar index) {
	vm_object * obj = NULL;
	uint16 base_arg = vm_get_sp() + vm_get_sysc().num_of_params;
	if(vm_get_sp() <= (base_arg - index)) {
		obj = _RECAST(vm_object *, vm_stack((base_arg - index)));
	}
	return obj;
}

uchar vm_get_argument_count(VM_DEF_ARG) {
	return vm_get_sysc().num_of_params;
}

#define VTXT_INSUFFICIENT_HEAP	 		"Insufficient Heap"
#define VTXT_STACK_OVERFLOW				"Stack Overflow"
#define VTXT_UNKNOWN_INSTRUCTION		"Unknown Instruction"
#define VTXT_UNIMPLEMENTED_APIS			"Unimplemented APIs"
#define VTXT_SYSTEM_EXCEPTION			"System Exception" 
#define VTXT_STACK_UNDERFLOW			"Stack Underflow"
#define VTXT_OUT_OF_BOUNDS				"Out of Bounds"	 
#define VTXT_UNRESOLVED_CLASS			"Unresolved Class"
#define VTXT_UNRESOLVED_METHOD			"Unresolved Method"
#define VTXT_ARGUMENT_MISMATCH			"Argument Mismatch"
#define VTXT_INVALID_CONTEXT			"Invalid Context"
#define VTXT_DIVIDE_BY_ZERO				"Divide by Zero"
#define VTXT_UNSUPPORTED_ARGUMENT		"Unsupported Argument"
#define VTXT_UNSUPPORTED_TYPE			"Unsupported Type"

static const char * _vm_exception_text[] = {	
	_RECAST(const char *, VTXT_SYSTEM_EXCEPTION),
	_RECAST(const char *, VTXT_UNIMPLEMENTED_APIS),
	_RECAST(const char *, VTXT_UNKNOWN_INSTRUCTION),
	_RECAST(const char *, VTXT_STACK_OVERFLOW),
  	_RECAST(const char *, VTXT_INSUFFICIENT_HEAP),
	_RECAST(const char *, VTXT_STACK_UNDERFLOW),
	_RECAST(const char *, VTXT_OUT_OF_BOUNDS),
	_RECAST(const char *, VTXT_UNRESOLVED_CLASS),
	_RECAST(const char *, VTXT_UNRESOLVED_METHOD),
	_RECAST(const char *, VTXT_ARGUMENT_MISMATCH),
	_RECAST(const char *, VTXT_INVALID_CONTEXT),
	_RECAST(const char *, VTXT_DIVIDE_BY_ZERO),
	_RECAST(const char *, VTXT_UNSUPPORTED_ARGUMENT),
	_RECAST(const char *, VTXT_UNSUPPORTED_TYPE),
};	

void vm_set_state(VM_DEF_ARG, uchar state) {
	printf("set state : %d\n", state);
 	ctx->vm_state = state;
}

vm_object * vm_syscall(VM_DEF_ARG, uint16 api_id) _REENTRANT_ {
	//BYTE i;
	vm_api_entry * iterator = (vm_api_entry *)&g_vaRegisteredApis;
	vm_set_cur_api(api_id);
	//printf("call api id : %d\n", api_id);
	while(iterator->entry != NULL) {
	 	if(iterator->id == api_id) {
			vm_set_retval((vm_object *)vm_create_object(VM_ARG, 0, NULL));
		 	iterator->entry(VM_ARG);
			if(vm_get_retval() == NULL) {
				vm_set_retval((vm_object *)vm_create_object(VM_ARG, 0, NULL));
			}
			return vm_get_retval();
		}
		iterator++;
	} 
#ifdef VM_EXT_APIS
	iterator = (vm_api_entry *)&g_vaRegisteredExtApis;
	while(iterator->entry != NULL) {
	 	if(iterator->id == api_id) {
			vm_set_retval((vm_object *)vm_create_object(VM_ARG, 0, NULL));
		 	iterator->entry(VM_ARG);
			if(vm_get_retval() == NULL) {
				vm_set_retval((vm_object *)vm_create_object(VM_ARG, 0, NULL));
			}
			return vm_get_retval();
		}
		iterator++;
	} 
#endif
	vm_invoke_exception(VM_ARG, VX_UNIMPLEMENTED_APIS);
}

void vm_invoke_exception(VM_DEF_ARG, uchar excp) { 
	uchar len;
	uchar i = 0, j;
	//i = tkPrintf("cd", STK_CMD_DISPLAY_TEXT, 0x81, STK_DEV_DISPLAY);
	//len = strlen(_vm_exception_text[excp]);
	//i += tkPushBufferB(i, STK_TAG_TEXT_STRING, len, _vm_exception_text[excp]);
	//tkDispatchCommandW(NULL, i); 
	printf("Exception : %s\r\n", _vm_exception_text[excp]);
	//_vm_state = VM_STATE_EXCEPTION;
	vm_set_state(VM_ARG, VM_STATE_EXCEPTION);
	vm_set_exception(excp);
} 

#pragma managed

#ifdef __cplusplus_cli
int StackVirtualMachine::Decode(VM_DEF_ARG) {
	return this->Decode(VM_ARG, vm_get_pc());
}

void StackVirtualMachine::InvokeException(VM_DEF_ARG, int code) { 
	uchar len;
	uchar i = 0, j;
	StackExceptionEventArgs^ excArgs = gcnew StackExceptionEventArgs(vm_get_pc());
	excArgs->Message = gcnew String(_RECAST(const char *, _vm_exception_text[code]));
	this->ExceptionCallback(this, excArgs);
	//_vm_state = VM_STATE_EXCEPTION;
	vm_set_state(VM_ARG, VM_STATE_EXCEPTION);
} 

#endif

#ifdef __cplusplus_cli
int StackVirtualMachine::Decode(VM_DEF_ARG, int offset) {
	StackApiEventArgs^ apiArgs;
	StackExceptionEventArgs^ evtArgs;
	StackMethodEventArgs^ mtdArgs;
	StackFetchEventArgs^ fchArgs;
	pin_ptr<System::Byte> ptr;
#else
void vm_decode(VM_DEF_ARG, uint32 offset, uchar num_args, ...) {
	va_list argptr;
#endif
	#if VM_CODEDEBUG
	uchar dbgbuf[2048];
	uchar varname[256];
	#endif
	uchar ibuf[4];
	uchar opcode;
	uchar psw = 0;
	uint16 index = 0;
	vm_object * obj;
	vm_object * op1, * op2;
	uint8 enable_gc = TRUE;
	void * new_base_addr = vm_get_package();
	//vm_set_pc(offset);
#ifndef __cplusplus_cli
	va_start(argptr, num_args);						//start variadic arguments
	for(index=0; index< num_args; index++) {
		obj = va_arg(argptr, vm_object *);
		vm_push_stack(obj);							//push to argument stack (VM function call)
	}
	va_end(argptr);              					//end of variadic arguments
#endif						
	vm_push_base(_RECAST(vm_object *, 0x0000));				//push 0xFFFF maximum PC address to base pointer for ret
	vm_push_base(_RECAST(vm_object *, 0xFFFF));				//push 0xFFFF maximum PC address to base pointer for ret
	printf("vm state = %d\n", vm_get_state() );
#ifdef __cplusplus_cli
	if(vm_get_state() == VM_STATE_RUN) {
#else
	if(vm_get_state() != VM_STATE_EXCEPTION) vm_set_state(VM_ARG, VM_STATE_RUN);
	while(vm_get_state() == VM_STATE_RUN) {   
#endif
		//printf("fetch\n");
		vm_fetch(VM_ARG, vm_get_pc(), ibuf, 4);
		opcode = ibuf[0];		
		enable_gc = TRUE;
		vm_set_rexec(FALSE);	//set recursive_exec flag to false
		//printf("fetch ok %02x\n", opcode);
		switch(opcode) {
			case INS_LBL:					//create new label
				break;
			case INS_OBJPUSH:
				index = ibuf[1];
				index += 1;
				obj = _RECAST(vm_object *, vm_get_base(index));
				if(obj != NULL) {
					obj->mgc_refcount ++;
				}
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				vm_debug_var(VM_ARG, opcode, obj);
				#endif
				vm_add_pc(2);
				break;
			case INS_OBJSTORE:
				index = ibuf[1];
				index += 1;
				obj = _RECAST(vm_object *, vm_stack(vm_get_sp() + 1));
				goto vmd_store_variable;
			case INS_OBJPOP:
				index = ibuf[1];
				index += 1;
				obj = vm_pop_stack_arc(VM_ARG);
				vmd_store_variable:
				if(vm_get_base(index) != obj) {
					op1 = _RECAST(vm_object *,  vm_get_base(index));
					if(op1 != NULL) {
						op1->mgc_refcount--;
					}
					vm_set_base(index, obj);
					if(obj != NULL) {
						obj->mgc_refcount++;
					}
				}
				#if VM_CODEDEBUG
				vm_debug_var(VM_ARG, opcode, obj);
				#endif
				vm_add_pc(2);
				break;
			case INS_SWITCH:
				obj = vm_pop_stack_arc(VM_ARG);
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if IS_REL_JUMP_ADDRESS
				op1 = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else	
				op1 = vm_load_constant(VM_ARG, index);
				#endif
				//jumptable
				for(psw =2; psw < op1->len; psw += 4) {
					index = end_swap16(*((uint16 * )(op1->bytes + psw)));
					#if IS_REL_JUMP_ADDRESS
					op2 = vm_load_constant(VM_ARG, index + vm_get_pc());
					#else
					op2 = vm_load_constant(VM_ARG, index);
					#endif
					//printf("%s\n", op2->bytes);
					if(vm_cmp_object(VM_ARG, obj, op2) == VM_CMP_EQ) {			//object match found
						index = end_swap16(*((uint16 * )(op1->bytes + psw + 2)));
						vm_release_object(VM_ARG, op2);							//release constant object
						goto start_jump;
					}
					vm_release_object(VM_ARG, op2);								//release constant object
				}	
				//default index
				index = end_swap16(*((uint16 * )(op1->bytes)));			//default jump offset
				start_jump:
				vm_release_object(VM_ARG, op1);									//release jump table
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %i", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				//printf("next pc %08x\n", index + _pc);
				#if IS_REL_JUMP_ADDRESS
				vm_set_pc(index + vm_get_pc());
				#else
				vm_set_pc(index);
				#endif
				break;
			case INS_OBJDEL:
				obj = vm_pop_stack_arc(VM_ARG);
				vm_release_object(VM_ARG, obj);
				vm_add_pc(1);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				break;
			case INS_OBJSUB:
				op2 = vm_pop_stack_arc(VM_ARG);
				op1 = vm_pop_stack_arc(VM_ARG);
				obj = vm_pop_stack_arc(VM_ARG);
				obj = vm_split_object(VM_ARG, op2, op1, obj);
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;
			case INS_OBJSZ:
				op1 = vm_pop_stack_arc(VM_ARG);
				obj = vm_size_object(VM_ARG, op1);
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;
			case INS_OBJDUP:
				op1 = _RECAST(vm_object *, vm_stack(vm_get_sp() + 1));
				//obj = vm_create_object(VM_ARG, op1->len, op1->bytes);		//create new object (clone object)
				vm_push_stack_arc(VM_ARG, op1);
				#if VM_CODEDEBUG
				vm_debug_var(VM_ARG, opcode, obj);
				#endif
				vm_add_pc(1);
				break;
			
			case INS_NOT:
				op2 = vm_pop_stack_arc(VM_ARG);
				op1 = op2;
				obj = vm_operation_object(VM_ARG, opcode, op2, op1);
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;
			case INS_ADD:
			case INS_MUL:
			case INS_DIV:
			case INS_SUB:
			case INS_AND:
			case INS_OR:
			case INS_XOR:
			case INS_SHL:
			case INS_SHR:
				op2 = vm_pop_stack_arc(VM_ARG);
				op1 = vm_pop_stack_arc(VM_ARG);
				obj = vm_operation_object(VM_ARG, opcode, op2, op1);
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;
			case INS_SYSCALL0:
			case INS_SYSCALL1:
			case INS_SYSCALL2:
			case INS_SYSCALL3:
			case INS_SYSCALL4:
			case INS_SYSCALL5:
			case INS_SYSCALL6:
			case INS_SYSCALL7:
			case INS_SYSCALL8:
			case INS_SYSCALL9:
			case INS_SYSCALL10:
			case INS_SYSCALL11:
			case INS_SYSCALL12:
			case INS_SYSCALL13:
			case INS_SYSCALL14:
			case INS_SYSCALL15:
				vm_get_sysc().num_of_params = (opcode - INS_SYSCALL0);		//initialize syscall context
				
#if __cplusplus_cli
				apiArgs = gcnew StackApiEventArgs(vm_get_pc());
				apiArgs->Arguments = gcnew array<array<Byte>^>(vm_get_sysc().num_of_params);
				//pop argument stack
				for(index = 0;index < vm_get_sysc().num_of_params;index ++) {
					vm_object * varg = vm_pop_stack_arc(VM_ARG);
					array<System::Byte>^ temp = gcnew array<System::Byte>(varg->len);
					ptr = &temp[0];
					memcpy(_RECAST(uchar *, ptr), varg->bytes, varg->len);
					apiArgs->Arguments[index] = temp;
				}
				
				this->ApiCallback(this, apiArgs);
				if(apiArgs->ReturnedValue != nullptr) {
					pin_ptr<System::Byte> rptr = &apiArgs->ReturnedValue[0];
					obj = vm_create_object(VM_ARG, apiArgs->ReturnedValue->Length, rptr);
				} else {
					obj = NULL;
				}
#else
				if(ibuf[1] < 254) {		  	//basic API
					vm_add_pc(2);
					obj = vm_syscall(VM_ARG, ibuf[1]);
					enable_gc = FALSE;
					//pop argument stack
					if(vm_get_rexec() == FALSE) {
						for(index = 0;index < vm_get_sysc().num_of_params;index ++) {
							vm_pop_stack_arc(VM_ARG);
						}
						if(vm_get_state() == VM_STATE_RUN) {
							vm_push_stack(obj);
						}
					}
				} else {					//extended API  
					vm_add_pc(4);
					index = ((uint16)ibuf[2] * 256) + ibuf[3];
					obj = vm_syscall(VM_ARG, index);
					enable_gc = FALSE;
					//pop argument stack
					if(vm_get_rexec() == FALSE) {
						for(index = 0;index < vm_get_sysc().num_of_params;index ++) {
							vm_pop_stack_arc(VM_ARG);
						}
						if(vm_get_state() == VM_STATE_RUN) {
							vm_push_stack(obj);
						}
					}
				}
#endif
				
				//_vm_stacks[_sp--] = obj;
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %d", _ins_name[opcode], ibuf[1]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				//_pc += 2;
				break;
			//F2O support argument count and vm_context (added 2018.01.10)
			case INS_F2O_0:			//96	//-> function to object
			case INS_F2O_1:			//97	//-> function to object
			case INS_F2O_2:			//98	//-> function to object
			case INS_F2O_3:			//99	//-> function to object
			case INS_F2O_4:			//100	//-> function to object
			case INS_F2O_5:			//101	//-> function to object
			case INS_F2O_6:			//102	//-> function to object
			case INS_F2O_7:			//103	//-> function to object
			case INS_F2O_8:			//104	//-> function to object
			case INS_F2O_9:			//105	//-> function to object
			case INS_F2O_10:			//106	//-> function to object
			case INS_F2O_11:			//107	//-> function to object
			case INS_F2O_12:			//108	//-> function to object
			case INS_F2O_13:			//109	//-> function to object
			case INS_F2O_14:			//110	//-> function to object
			case INS_F2O_15:			//111	//-> function to object
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				obj = vm_create_object(VM_ARG, sizeof(vm_function), NULL);				//copy current execution handle (vm_context)
				//obj->mgc_refcount |= VM_OBJ_MAGIC;
				((vm_function *)obj->bytes)->base = vm_get_package();
				((vm_function *)obj->bytes)->instance = VM_ARG;
				((vm_function *)obj->bytes)->arg_count = (opcode - INS_F2O_0);		//set argument count
				((vm_function *)obj->bytes)->offset = index;									//set function offset
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %08x", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(3);
				break;
			case INS_SCTX:
				//increment base pointer
				index = ibuf[1];
				for(psw = 0; psw < index; psw ++) {
					vm_push_base(vm_create_object(VM_ARG, 0, NULL));
				}
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %d", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(2);
				break;
			case INS_RCTX:
				//_bp -= ibuf[1];			//increment base pointer
				index = ibuf[1];
				for(psw = 0; psw < index; psw ++) {
					obj = _RECAST(vm_object *, vm_pop_base());
					vm_stack(vm_get_bp()) = vm_create_object(VM_ARG, 0, NULL);
					if(obj != NULL) obj->mgc_refcount --;
				}
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %d", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(2);
				break;
			case INS_RET:
				index = _RECAST(uint16, vm_pop_base());
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_set_pc(index);
				vm_set_package(vm_pop_base());
				new_base_addr = vm_get_package();
				break;
			case INS_EXTCALL0:
			case INS_EXTCALL1:
			case INS_EXTCALL2:
			case INS_EXTCALL3:
			case INS_EXTCALL4:
			case INS_EXTCALL5:
			case INS_EXTCALL6:
			case INS_EXTCALL7:
			case INS_EXTCALL8:
			case INS_EXTCALL9:
			case INS_EXTCALL10:
			case INS_EXTCALL11:
			case INS_EXTCALL12:
			case INS_EXTCALL13:
			case INS_EXTCALL14:
			case INS_EXTCALL15:
				index = end_swap16(*((uint16 * )(ibuf + 1)));		//offset of method name
				op1 = vm_pop_stack_arc(VM_ARG);		//class instance
				#if IS_REL_JUMP_ADDRESS
				obj = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else
				obj = vm_load_constant(VM_ARG, index);
				#endif			
#if __cplusplus_cli
				mtdArgs = gcnew StackMethodEventArgs();
				mtdArgs->ClassName = gcnew String(_RECAST(const char *, op1->bytes), 0, op1->len);
				mtdArgs->MethodName = gcnew String(_RECAST(const char *, obj->bytes), 0, obj->len);
				
				this->LoadMethodCallback(this, mtdArgs);
				if(mtdArgs->MethodBase == nullptr) { 
					this->InvokeException(VM_ARG, VX_UNIMPLEMENTED_APIS);
					vm_set_pc(0xFFFF);
					break; 
				}
				ptr = &mtdArgs->MethodBase[0];
				index = mtdArgs->MethodOffset;
				index -= vm_get_pc();
				*((uint16 * )(ibuf + 1)) = end_swap16(index);
				new_base_addr = _RECAST(void *, ptr);
				
#else
				index = sizeof(vm_interface);
				if (op1->len != index) {
					printf("invalid interface size : %d : %d\n", op1->len, sizeof(vm_interface));
					vm_invoke_exception(VM_ARG, VX_UNRESOLVED_CLASS);
					vm_set_pc(0xFFFF); break;
				}
				if(((pk_object *)((vm_interface *)op1->bytes)->base)->tag == PK_TAG_INTERFACE) {
					//vm_set_retval(NULL);
					#if VM_CODEDEBUG
					sprintf(dbgbuf, "\t\t%s %08x", _ins_name[opcode], index);
					vm_debug(vm_get_pc(), dbgbuf);
					#endif
					obj = vm_interface_exec(VM_ARG, opcode & 0x0F, op1, obj);
					vm_push_stack(obj);
					vm_add_pc(3);
					break;
				}
				obj = (vm_object *)vm_load_method(VM_ARG, op1, obj);
				if(obj == NULL) { vm_set_pc(0xFFFF); break; }
				index = ((pk_method *)obj)->offset;
				index -= vm_get_pc();
				*((uint16 * )(ibuf + 1)) = end_swap16(index);
				new_base_addr = ((pk_object *)obj)->codebase;
#endif
				//printf("index : %d\n", ((pk_method *)obj)->offset);
				//_pc = 0;
			case INS_CALL:
				//printf("call function\n");
				vm_push_base(_RECAST(void *, vm_get_package()));					//save current base address
				vm_push_base(_RECAST(void *, (vm_get_pc() + 3)));				//save PC+3, address of next instruction
			case INS_JMP:									//jump to specified label
				vm_set_package(new_base_addr);
				//printf("base %d\n", _base_address);
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %08x", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				#if IS_REL_JUMP_ADDRESS
				vm_set_pc(index + vm_get_pc());
				#else
				vm_set_pc(index);
				#endif
				//printf("PC: %d\n", _pc);
				break;
			case INS_OBJNEW: 
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if IS_REL_JUMP_ADDRESS
				op1 = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else
				op1 = vm_load_constant(VM_ARG, index);
				#endif 	
				obj = vm_load_class(VM_ARG, op1); 
				if(obj->len == 0) { vm_invoke_exception(VM_ARG, VX_UNRESOLVED_CLASS); vm_set_pc(0xFFFF); break; }
				obj->mgc_refcount++;
				vm_push_stack(obj);	
				#if VM_CODEDEBUG
				vm_debug_var(VM_ARG, opcode, obj);
				#endif			
				vm_add_pc(3);
				vm_release_object(VM_ARG, op1);		//no need for op1
				break;
			case INS_OBJCONST:
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if IS_REL_JUMP_ADDRESS
				obj = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else
				obj = vm_load_constant(VM_ARG, index);
				#endif
				obj->mgc_refcount++;	
				vm_push_stack(obj);	
				#if VM_CODEDEBUG
				vm_debug_var(VM_ARG, opcode, obj);
				#endif		
				vm_add_pc(3);
				break;
			case INS_JFALSE:
				op1 = vm_pop_stack_arc(VM_ARG);
				#if IS_REL_JUMP_ADDRESS
				index = 3;
				#else
				index = (vm_get_pc() + 3);
				#endif
				if(memcmp(op1->bytes, "false", op1->len) == 0) {
					index = end_swap16(*((uint16 * )(ibuf + 1)));
				}
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %08x", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				#if IS_REL_JUMP_ADDRESS
				vm_set_pc(index + vm_get_pc());
				#else
				vm_set_pc(index);
				#endif
				break;
			case INS_CREQ:		//64	//-> jump if equal (relative to pc)
			case INS_CRNE:			//65	//-> jump not equal (relative to pc)
			case INS_CRGT:			//66	//-> jump greater than (relative to pc)
			case INS_CRLT:			//67	//-> jump less than (relative to pc)
			case INS_CRGTEQ:			//68	//-> jump greater than (relative to pc)
			case INS_CRLTEQ:			//69	//-> jump less than (relative to pc)
				op2 = vm_pop_stack_arc(VM_ARG);
				op1 = vm_pop_stack_arc(VM_ARG);
				obj = vm_compare_object(VM_ARG, opcode, op2, op1);
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %08x", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;

			default: /* printf("%08x\n", _pc); */			//unknown instruction exception
				//_pc += 1;
				//printf("%08x %02x\n", _pc, opcode); 
#ifdef __cplusplus_cli
				this->InvokeException(VM_ARG, VX_UNKNOWN_INSTRUCTION);
#else
				vm_invoke_exception(VM_ARG, VX_UNKNOWN_INSTRUCTION);
#endif
				break;
		}
		if(vm_get_bp() >= vm_get_sp()) {		//stack overflow exception
#ifdef __cplusplus_cli
				this->InvokeException(VM_ARG, VX_STACK_OVERFLOW);
#else
		  	vm_invoke_exception(VM_ARG, VX_STACK_OVERFLOW);
#endif
		}
		//printf("sp : %d, bp : %d\n", _sp, _bp);
		//#try-catch mechanism (tcblock)
		if (vm_get_state() == VM_STATE_EXCEPTION) {
			vm_tcblock * tc = vm_pop_tcblock(VM_ARG);
			if (tc != NULL) {
				//vm_garbage_collect(VM_ARG);
				//memcpy(VM_ARG, tc->snapshot, sizeof(vm_instance));		//restore snapshot
				while (vm_get_sp() != tc->snapshot->sp) {
					vm_pop_stack_arc(VM_ARG);
				}
				vm_set_pc(tc->_catch);									//jump to catch handler
				for (index = 0;index < tc->num_args;index++) {			//push catch arguments
					//vm_push_argument(VM_ARG, 0, NULL);
					switch (index) {
						case 0: 
							obj = vm_create_object(VM_ARG, strlen(_vm_exception_text[vm_get_exception()]), (char *)_vm_exception_text[vm_get_exception()]);
							vm_push_argument_object(VM_ARG, obj); 
							break;
						default: 
							obj = vm_create_object(VM_ARG, 0, NULL);
							vm_push_argument_object(VM_ARG, obj);
							break;
					}
				}
				vm_set_state(VM_ARG, VM_STATE_RUN);						//handle exception as normal execution
				free(tc->snapshot);
				free(tc);
			}
		}
		else {
			vm_locate_tcblock(VM_ARG);
		}
		//printf("garbage collect\n");
		vm_garbage_collect(VM_ARG);	
#if __cplusplus_cli
		fchArgs = gcnew StackFetchEventArgs(vm_get_pc());
		
		this->FetchCallback(this, fchArgs);
		switch(fchArgs->MachineState) {
			case StackMachineState::RUN: break;
			case StackMachineState::SUSPEND: vm_set_state(VM_ARG, VM_STATE_SUSPEND); break;
			case StackMachineState::EXCEPTION: 
				vm_set_state(VM_ARG, VM_STATE_EXCEPTION); 
				break;
			case StackMachineState::ABORT: vm_set_state(VM_ARG, VM_STATE_ABORT); break;
			default: break;
		}
#endif
		//getch();
		if(vm_get_pc() == 0xFFFF) {	  		//normally terminated
			printf("terminate\n");
			vm_set_state(VM_ARG, VM_STATE_INIT);
		}
	}
#ifdef __cplusplus_cli
	return vm_get_pc();
#endif
}

void va_sys_exec(VM_DEF_ARG) _REENTRANT_ {
	uint8 arglen, i;
	vm_function * func;
	uint16 base_arg = vm_get_sp() + vm_get_sysc().num_of_params;
	//OS_DEBUG_ENTRY(va_sys_exec);
	if(vm_get_argument_count(VM_ARG) == 0) {
		vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
		goto exit_sys_exec;
	}
	arglen = vm_get_argument_count(VM_ARG) - 1;
	((vm_object *)vm_get_argument(VM_ARG, 0))->mgc_refcount--;
	func = (vm_function *)(vm_get_argument(VM_ARG, 0)->bytes);
	//check function argument with syscall argument
	if(func->arg_count != arglen) {
		vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
		goto exit_sys_exec;
	}
	//shift arguments by 1
	for(i=1;i<vm_get_argument_count(VM_ARG);i++) {
		vm_stack(base_arg) = vm_stack(base_arg - 1)  ;
		base_arg--;
	}
	vm_get_sp() += 1;
	//execute function (recursive execution from syscall)
	vm_exec_function(VM_ARG, func);
	exit_sys_exec:
	//OS_DEBUG_EXIT();
	return;
}

#ifdef STANDALONE_INTERPRETER
void vm_init(VM_DEF_ARG, pk_object * root, uint32 heap_size) {		//original input file path as parameter
	//memset(_vm_stacks, 0, sizeof(_vm_stacks));
	lk_import_directory("\\packages");
	memset(&vm_get_context(), 0, sizeof(vm_instance));
	vm_set_heap(m_heap_create(heap_size));
	m_init_alloc(vm_get_heap());
	vm_get_context().var_root = 0;
	vm_get_context().vars = &vm_get_context().var_root;
	vm_set_pc(0);
	vm_set_sp(VM_MAX_STACK_SIZE -1);
	vm_set_bp(0);
	vm_set_info(root);
}

void vm_close(VM_DEF_ARG) {
	free(vm_get_heap());
}
#endif

#ifdef STANDALONE_VIRTUAL_MACHINE 
#include "build_vm.h"
void main( int argc, char *argv[] )
{ 
	extern FILE *yyin;
	int i;
	vm_instance vcit;
	//++argv; --argc;
	if(argc < 2) return;
	vm_init(&vcit, argv[0], argv[1]);
	vm_decode(&vcit, 0, 0);
	vm_close(&vcit);
	getch();
}
#endif

void vm_push_tcblock(VM_DEF_ARG, pk_tcblock * block) {
	vm_tcblock * tc = (vm_tcblock *)malloc(sizeof(vm_tcblock));
	tc->next = NULL;
	tc->_catch = block->cblock;
	tc->_end = block->cblock - 1;
	tc->_start = block->start;
	tc->num_args = block->num_args;
	tc->snapshot = (vm_instance *)malloc(sizeof(vm_instance));
	memcpy(tc->snapshot, VM_ARG, sizeof(vm_instance));
	tc->next = vm_get_tcs();
	vm_set_tcs(tc);
}

vm_tcblock * vm_pop_tcblock(VM_DEF_ARG) {
	vm_tcblock * ret;
	if (vm_get_tcs() == NULL) return NULL;
	ret = vm_get_tcs();
	vm_set_tcs(ret->next);
	ret->next = NULL;
	return ret;
}

void vm_locate_tcblock(VM_DEF_ARG) {
	pk_object * iterator = vm_get_info();
	vm_tcblock * tc;
	//load new tcblock if any
	while (iterator != NULL) {
		if (iterator->codebase == vm_get_package() && iterator->tag == PK_TAG_TCBLOCK) {
			if (vm_get_pc() == ((pk_tcblock *)iterator)->start) {
				vm_push_tcblock(VM_ARG, (pk_tcblock *)iterator);
			}
		}
		iterator = (pk_object *)iterator->next;
	}
	//remove current tcblock if any
	if (vm_get_tcs() != NULL && vm_get_tcs()->_end <= vm_get_pc()) {
		tc = vm_pop_tcblock(VM_ARG);
		free(tc->snapshot);
		free(tc);
	}
}
