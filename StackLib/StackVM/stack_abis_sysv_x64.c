#include "defs.h"
#include "config.h"
#include "vm_stack.h"
#include "vm_framework.h"


#if (defined(WIN32) || defined(WIN64)) && defined(_MSC_VER)
extern "C" {
//only for CL32
#include "dlfcn.h"
#endif

#if defined(__GNUC__)
#include <dlfcn.h>
#endif

static void vm_iptr_release(void * ctx, void * ptr) {
	//printf("free memory : %ld\n", ptr);
	free(ptr);
}

void vm_print_var(void * v) {
	//float f = ((float *)v)[0];
	printf("var : %lf(%016lx)\n", v, v);
}

void vm_print_sss() {
	printf("break here\n");
}

void vm_interface_exec_dump(void * args[], uint16 args_type[], int num_args) {
	for(int i=0;i<(num_args + 2);i++) {
		if(args[i] == NULL) {
			printf("args[%i]@%016lx : NULL\n", i, args[i]);
			continue;
		}
		switch(args_type[i]) {
		    case VM_INT8:
			printf("args[%i]@%016lx : %ld(%i)\n", i, args[i], *(int8 *)args[i], args_type[i]);
			break;
		    case VM_INT16:
			printf("args[%i]@%016lx : %ld(%i)\n", i, args[i], *(int16 *)args[i], args_type[i]);
			break;
		    case VM_INT32:
			printf("args[%i]@%016lx : %ld(%i)\n", i, args[i], *(int32 *)args[i], args_type[i]);
			break;
		    case VM_INT64:
			printf("args[%i]@%016lx : %ld(%i)\n", i, args[i], *(int64 *)args[i], args_type[i]);
			break;
		    case VM_DOUBLE:
			printf("args[%i]@%016lx : %f(%i)\n", i, args[i], *(double *)args[i], args_type[i]);
			break;
		    case VM_FLOAT:
			printf("args[%i]@%016lx : %f(%08x:%i)\n", i, args[i], *(float *)args[i], *(int32 *)args[i], args_type[i]);
			break;
		    default: 
			if(args_type[i] == VM_STRING) {
				printf("args[%i] : %s\n", i, args[i]);
			} else {
				printf("args[%i]@%016lx : %ld(%08x:%i)\n", i, args[i], *(float *)args[i], *(int32 *)args[i], args_type[i]);
			}
			break;
			
		}
	}
} 

extern int64 vm_call_func_x64(void(*func)(), int ret_type, void * args[], uint16 args_type[], int num_args, double * dbl);
extern double vm_get_xmm();

vm_object * vm_interface_exec_sta(VM_DEF_ARG, uint8 num_args, vm_object * iface, uint8 * method_name) {
	//static vm_object * null_obj = (vm_object *)VM_NULL_OBJECT;
	//va_list vl;
	//void * stack_args[32];
	//uint16 * stack_args_type[32];
	double dtemp;
	vm_object * ret = NULL;
	pk_method * pkm;
	vm_object * args[16];
	vm_object * arg;
	pk_object * pkc;
	void * temp_args[16];
	uint16 args_type[16];
	//int stack_index = 0;
	int int_count = 0;
	int precision_count = 0;
	int arg_pushed_count = 0;
	int64 ptemp;
	void * temp;
	float ftemp;
	void(*func)(void) = NULL;
	int64 wtemp;
	char fname[256];
	int offset;
	uint16 type;
	int i;
	//printf("iface->len = %d\n", iface->len);
	if (iface->len == 0) return ret;
	pkc = ((pk_object **)iface->bytes)[1];

	//printf("pkc->tag %d == %d\n", pkc->tag, PK_TAG_INTERFACE);
	if (pkc->tag != PK_TAG_INTERFACE) return ret;
	//try default method name (for static method)
	pkm = pk_select_method((pk_class *)pkc, method_name);		
	if (pkm == NULL) {
		//use overloaded method naming convention
		sprintf(fname, "%s?%d", method_name, num_args);
		pkm = pk_select_method((pk_class *)pkc, (uchar *)fname);
	}
	//printf("pk_select_method %s -> %d\n", method_name, pkm);
	//printf("pkm check\n");
	//if(pkm == NULL) return ret;
	if (pkm != NULL) {
		if (pkm->callback != NULL) {
			//printf("pkm use callback\n");
			func = pkm->callback;
		}
		else {
			//printf("select function %s\n", method_name);
			func = (void(*)(void))dlsym(((pk_object *)pkc)->codebase, (const char *)method_name);
			//printf("Results after dlsym(): funcp = %p; dlerror = %s\n", (void *) func, dlerror());
		}
	}
	//if(pkm != NULL) printf("%d==%d %lld\n", pkm->numargs, num_args, func);
	//if(func == NULL) printf("entry point %s not found\n", method_name);
	if (pkm != NULL && pkm->numargs == num_args && func != NULL) {		//method available and total arguments matched
																			//typecasting argument if any
																			//va_start(vl, method_name);
		//printf("num arguments : %d\n", pkm->numargs);
		//for (i = 0;i < num_args;i++) {
		//}
		for (i = 0;i < num_args;i++) {
				//args[i] = va_arg(vl, vm_object *);
			args[i] = (vm_object *)vm_pop_stack();
			if (args[i]->type != pkm->param_types[(num_args - 1) - i]) { //auto-cast variable
				if ((args[i]->type & 0x7F) == pkm->param_types[(num_args - 1) - i]) {
					//printf("no cast %lx\n", args[i]);
					continue;
				}
				arg = args[i];
				//printf("cast to %x\n", pkm->param_types[(num_args - 1) - i]);
				args[i] = va_data_cast(VM_ARG, arg, pkm->param_types[(num_args - 1) - i]);
				vm_object_unref(arg);
			}
		}
		temp_args[0] = VM_ARG;
		args_type[0] = VM_OBJ_TYPE_ARR;
		temp_args[1] = ((pk_object **)iface->bytes)[0];
		args_type[1] = VM_OBJ_TYPE_ARR;
		//stack_args = (void *)malloc(sizeof(void *) *  num_args);
		//stack_args_type = (uint16 *)malloc(sizeof(uint16) *  num_args);
		for (i = 0;i < num_args ; i++) {
			offset = i;
			type = pkm->param_types[offset];
			if(type == VM_VAR) {
				temp_args[i + 2] = args[(num_args - 1) - offset]->bytes;
				temp_args[i + 2] = *((void **)temp_args[i + 2]);
			} else {
				temp_args[i + 2] = args[(num_args - 1) - offset]->bytes;
			}
			args_type[i + 2] = type;
			//printf("temp_args[%d] : %lx\n", (num_args - 1) - offset, args[(num_args - 1) - offset]->bytes);
			/*switch(type & VM_OBJ_TYPE_PRECISION) {
				case 0:		//non precision
					int_count ++;
					//if(int_count > 4) {		//sixth argument
					//	stack_args[stack_index] = args[(num_args - 1) - offset]->bytes;
					//	stack_args_type[stack_index] = type;
					//	stack_index++;
					//	break;
					//}
					temp_args[arg_pushed_count + 2] = args[(num_args - 1) - offset]->bytes;
					args_type[arg_pushed_count + 2] = type;
					arg_pushed_count++;
					break;
				default:
					precision_count++;
					//if(precision_count > 6) {		//sixth argument
					//	stack_args[stack_index] = args[(num_args - 1) - offset]->bytes;
					//	stack_args_type[stack_index] = type;
					//	stack_index++;
					//	break;
					//}
					temp_args[arg_pushed_count + 2] = args[(num_args - 1) - offset]->bytes;
					args_type[arg_pushed_count + 2] = type;
					arg_pushed_count++;
					break;
			}*/
			
		}
		//for (i = arg_pushed_count;i < num_args ; i++) {
			//printf("pushed argument : %d", stack_args[stack_index - 1]);
			//temp_args[arg_pushed_count + i + 2] = stack_args[stack_index - 1];
			//args_type[arg_pushed_count + i + 2] = stack_args_type[stack_index - 1];
			//stack_index--;
		//}
		//printf("pushed argument : %d, stack args : %d\n", arg_pushed_count, stack_index);	
		//pushed as stack (reverse)
		//for (i;i < num_args;i++) {
		//	offset = (num_args - (i - 4)) -1;
		//	temp_args[i + 2] = args[((num_args-1)- offset)]->bytes;
		//	args_type[i + 2] = pkm->param_types[offset];
		//}

		//printf("vm_call_func_x64 %d %lx\n", pkm->ret_type, pkm);
		//vm_interface_exec_dump(temp_args, args_type, num_args);
		//vm_interface_exec_dump(stack_args, stack_args_type, stack_index);
		ptemp = vm_call_func_x64(func, pkm->ret_type, temp_args, args_type, num_args, &dtemp);
		//ptemp = 0;
		//ptemp = VM_NULL_OBJECT;
		//printf("end vm_call_func_x64 %d\n", (int)ptemp);
		//printf("returning var %d %lx\n", pkm->ret_type, pkm);
		if (pkm->ret_type & VM_OBJ_TYPE_PRECISION) {
			ftemp = dtemp;
			temp = &dtemp;
			if (pkm->ret_type & 0x0F == sizeof(float)) temp = &ftemp;
		}
		else {
			temp = &ptemp;
		}

		if (pkm->ret_type == VM_VAR) {
			//printf("direct return\n");
			ret = (vm_object *)ptemp;
		} else {
			//printf("create encapsulated operand %d %lx\n", *(int *)temp, pkm->ret_type);
			ret = vm_create_operand(VM_ARG, pkm->ret_type, temp, pkm->ret_type & 0x0F);
			if ((ret->type & VM_AUTO_POINTER) == VM_AUTO_POINTER && ret->len == sizeof(void *)) {
				ret->release = vm_iptr_release;
			}
		}
		//force release argument stack
		//printf("executed successfully\n");
		for (i = 0;i < num_args;i++) {
			//vm_release_object(VM_ARG, args[i]);
			vm_object_unref(args[i]);
		}
	}
	//if(ret != NULL) printf("return %lx(%lx)\n", *(void **)ret->bytes, ret);
	return ret;
}

vm_object * vm_interface_exec(VM_DEF_ARG, uint8 num_args, vm_object * iface, vm_object * method) 
{
	vm_object * ret;
	//va_list vl;
	uint8 buffer[256];
	//vm_object * args[20];
	//printf("vm_interface_exec\n");
	if (method->len == 0) return ret;
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, method->bytes, method->len);
	for (int i = 0;i < method->len;i++) if (buffer[i] == '?') buffer[i] = 0;
	//printf("method name : %s\n", buffer);
	//for(int i=0; i<num_args; i++) {
	//	args[i] = vm_pop_stack_arc(VM_ARG);
	//va_arg(vl, vm_object *) = vm_pop_stack_arc(VM_ARG);
	//}
	ret = vm_interface_exec_sta(VM_ARG, num_args, iface, buffer);
	if (ret == NULL) {
		ret = vm_create_object(VM_ARG, 0, NULL);
	}
	return ret;
}

#if (defined(WIN32) || defined(WIN64)) && defined(_MSC_VER)
}
#endif
