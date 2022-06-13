//#include "stdafx.h"


#pragma unmanaged
#include "vm_stack.c"
#include "vm_framework.c"
#include "vm_stream.c"


int vm_bytes_length(void * str) {
	vm_object * ptr = (vm_object *)((uint8 *)str - (sizeof(vm_object) - 1));
	if ((ptr->mgc_refcount & VM_MAGIC) == VM_MAGIC) {
		return ptr->len;
	}
	return 0;		//invalid object
}

static void va_file_read(VM_DEF_ARG) {
	int size = -1;
	FILE * f;
	vm_object * obj;
	vm_object * vfile = vm_get_argument(VM_ARG, 0);
	if (vm_get_argument_count(VM_ARG) > 1) {
		size = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	}
	f = (FILE *)((va_default_context *)vfile->bytes)->ctx;
	if (size == -1) {
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
	}
	//uint8 * bytes = (uint8 *)malloc(size);
	//size = fread(bytes, 1, size, (FILE *)((va_default_context *)vfile->bytes)->ctx);
	vm_set_retval((obj = vm_create_immutable_object(VM_ARG, size, NULL)));
	fread(obj->bytes, 1, size, (FILE *)((va_default_context *)vfile->bytes)->ctx);
	//free(bytes);
}

static void va_file_write(VM_DEF_ARG) {
	vm_object * vfile = vm_get_argument(VM_ARG, 0);
	vm_object * vbytes = vm_get_argument(VM_ARG, 1);
	fwrite(vbytes->bytes, 1, vbytes->len, (FILE *)((va_default_context *)vfile->bytes)->ctx);
}

static void va_file_seek(VM_DEF_ARG) {
	int mode = SEEK_SET;
	vm_object * vfile = vm_get_argument(VM_ARG, 0);
	int offset = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	fseek((FILE *)((va_default_context *)vfile->bytes)->ctx, 0, SEEK_END);
	int size = ftell((FILE *)((va_default_context *)vfile->bytes)->ctx);
	if (size < offset) {
		va_return_word(VM_ARG, size);
	} else {
		if (vm_get_argument_count(VM_ARG) >= 3) mode = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 2));
		offset = fseek((FILE *)((va_default_context *)vfile->bytes)->ctx, offset, mode);
		offset = ftell((FILE *)((va_default_context *)vfile->bytes)->ctx);
		va_return_word(VM_ARG, offset);
	}
}

static void va_file_close(VM_DEF_ARG) {
	vm_object * vfile = vm_get_argument(VM_ARG, 0);
	fclose((FILE *)((va_default_context *)vfile->bytes)->ctx);
}

void va_fopen(VM_DEF_ARG) {
	va_default_context defctx;
	vm_object * ret;
	vm_object * vpath = vm_get_argument(VM_ARG, 0);
	char * path = (char *)malloc(vpath->len + 1);
	memcpy(path, vpath->bytes, vpath->len);
	path[vpath->len] = 0;
	FILE * f = fopen(path, "r+");
	if (f != NULL) {
		defctx.ctx = f;
		defctx.read = va_file_read;
		defctx.seek = va_file_seek;
		defctx.write = va_file_write;
		defctx.close = va_file_close;
		ret = vm_create_object(VM_ARG, sizeof(va_default_context), (uint8 *)&defctx);
		vm_set_retval(ret);
	}
	free(path);
}

#ifndef __cplusplus_cli
#ifdef _MSC_VER
#include "dlfcn.c"
#endif
#endif