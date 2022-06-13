#include "defs.h"
#include "config.h"
#include "vm_stack.h"
#include "vm_framework.h"
extern "C" {

#if defined(WIN64) && defined(_MSC_VER)
//only for CL32
#include "dlfcn.h"

	static void vm_iptr_release(void * ctx, void * ptr) {
		//printf("free memory : %ld\n", ptr);
		free(ptr);
	}
	extern int64 vm_call_func_x64(void(*func)(), int ret_type, void * args[], uint16 args_type[], int num_args, double * dbl);
	extern double vm_get_xmm();

	vm_object * vm_interface_exec_sta(VM_DEF_ARG, uint8 num_args, vm_object * iface, uint8 * method_name) {
		//static vm_object * null_obj = (vm_object *)VM_NULL_OBJECT;
		//va_list vl;
		vm_object * ret = NULL;
		pk_method * pkm;
		vm_object * args[16];
		vm_object * arg;
		pk_object * pkc;
		void * temp_args[16];
		uint16 args_type[16];
		int64 ptemp;
		void * temp;
		double dtemp;
		float ftemp;
		void(*func)(void) = NULL;
		int64 wtemp;
		char fname[256];
		if (iface->len == 0) return ret;
		pkc = ((pk_object **)iface->bytes)[1];
		if (pkc->tag != PK_TAG_INTERFACE) return ret;
		//try default method name (for static method)
		pkm = pk_select_method((pk_class *)pkc, method_name);		
		if (pkm == NULL) {
			//use overloaded method naming convention
			sprintf(fname, "%s?%d", method_name, num_args);
			pkm = pk_select_method((pk_class *)pkc, (uchar *)fname);
		}
		if (pkm != NULL) {
			if (pkm->callback != NULL) {
				func = pkm->callback;
			}
			else {
				func = (void(*)(void))dlsym(((pk_object *)pkc)->codebase, (const char *)method_name);
			}
		}
		if (pkm != NULL && pkm->numargs == num_args && func != NULL) {		//method available and total arguments matched
																			//typecasting argument if any
																			//va_start(vl, method_name);
			for (int i = 0;i < num_args;i++) {
				//args[i] = va_arg(vl, vm_object *);
				args[i] = (vm_object *)vm_pop_stack(VM_ARG);
				if (args[i]->type != pkm->param_types[(num_args - 1) - i]) { //auto-cast variable
					if ((args[i]->type & 0x7F) == pkm->param_types[(num_args - 1) - i])
						continue;
					arg = args[i];
					args[i] = va_data_cast(VM_ARG, arg, pkm->param_types[(num_args - 1) - i]);
					//vm_release_object(VM_ARG, arg);
					vm_object_unref(arg);
				}
			}
			int i;
			int offset;
			temp_args[0] = VM_ARG;
			args_type[0] = VM_OBJ_TYPE_ARR;
			temp_args[1] = ((pk_object **)iface->bytes)[0];
			args_type[1] = VM_OBJ_TYPE_ARR;
			for ( i = 0;i < num_args && i < 2;i++) {
				offset = i;
				temp_args[i + 2] = args[(num_args - 1) - offset]->bytes;
				args_type[i + 2] = pkm->param_types[offset];
			}
			//pushed as stack (reverse)
			for (i;i < num_args;i++) {
				offset = (num_args - (i - 2)) -1;
				temp_args[i + 2] = args[((num_args-1)- offset)]->bytes;
				args_type[i + 2] = pkm->param_types[offset];
			}

			ptemp = vm_call_func_x64(func, pkm->ret_type, temp_args, args_type, num_args, &dtemp);
			if (pkm->ret_type & VM_OBJ_TYPE_PRECISION) {
				ftemp = dtemp;
				temp = &dtemp;
				if (pkm->ret_type & 0x0F == sizeof(float)) temp = &ftemp;
			}
			else {
				temp = &ptemp;
			}
			//va_end(vl);
			//create vm_operand
			if (pkm->ret_type == VM_VAR) {
				ret = (vm_object *)ptemp;
			} else {
				ret = vm_create_operand(VM_ARG, pkm->ret_type, temp, pkm->ret_type & 0x0F);
				if ((ret->type & VM_AUTO_POINTER) == VM_AUTO_POINTER && ret->len == sizeof(void *)) {
					ret->release = vm_iptr_release;
				}
			}
			//force release argument stack
			//printf("executed successfully\n");
			for (int i = 0;i < num_args;i++) {
				//vm_release_object(VM_ARG, args[i]);
				vm_object_unref(args[i]);
			}
		}
		return ret;
	}

	vm_object * vm_interface_exec(VM_DEF_ARG, uint8 num_args, vm_object * iface, vm_object * method) {
		vm_object * ret;
		//va_list vl;
		uint8 buffer[256];
		//vm_object * args[20];
		if (method->len == 0) return ret;
		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, method->bytes, method->len);
		for (int i = 0;i < method->len;i++) if (buffer[i] == '?') buffer[i] = 0;

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
#endif

}