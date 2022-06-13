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

vm_object * vm_add_immutable_object(VM_DEF_ARG, vm_object * obj) {
	vm_object * iterator = NULL;
	if (VM_ARG->object_list == NULL) VM_ARG->object_list = obj;
	else {
		iterator = VM_ARG->object_list;
		while (iterator->next != NULL) iterator = iterator->next;
		iterator->next = obj;
	}
	return obj;
}

LIB_API vm_object * vm_load_bool(VM_DEF_ARG, uchar value) {
	vm_object * obj = NULL;
	if(value) {
		obj = _RECAST(vm_object *, vm_create_object(VM_ARG, 4, _RECAST(uchar *, "true")));		//create new object (clone object)
	} else {
		obj = _RECAST(vm_object *, vm_create_object(VM_ARG, 5, _RECAST(uchar *, "false")));		//create new object (clone object)
	}
	return obj;
}

LIB_API uint8 vm_object_get_type(vm_object * obj) {
	return (obj->mgc_refcount & VM_MAGIC_MASK);
}

LIB_API uint16 vm_object_get_text(VM_DEF_ARG, vm_object * obj, uint8 * text) {
	uint16 len = 0;
	switch(vm_object_get_type(obj)) {
		case VM_MAGIC:
			len = obj->len;
			memcpy(text, obj->bytes, len);
			break;
		case VM_EXT_MAGIC:
			len = ((vm_extension *)obj->bytes)->apis->text(VM_ARG, obj, text);
			break;
	}
	return len;
}

LIB_API uint32 vm_array_get_count(VM_DEF_ARG, uint8 * bytes) {
	vm_object * obj;
	if (bytes == NULL) return 0;
	//printf("bytes : %d\n", bytes);
	int r_offset = (sizeof(vm_object) - sizeof(uint8*));
	obj = (vm_object *)(void *)(bytes - r_offset);
	return obj->len / (obj->type & 0x0F);
}

LIB_API vm_object * vm_create_interface(VM_DEF_ARG, pk_class * base, uint8 * ptr, void (* release)(void *, void *)) {
	vm_interface iface ;
	vm_object * ret = VM_NULL_OBJECT;
	iface.base = base;
	iface.ptr = NULL;
	if(ptr != NULL) iface.ptr = ptr;
	if(base == NULL) return ret;
	ret = vm_create_object(VM_ARG, sizeof(vm_interface), (uchar *)&iface);
	ret->type = VM_OBJ_TYPE_INTERFACE;
	ret->release = release;
	return ret;
}

LIB_API vm_object * vm_create_interface_enumerator(VM_DEF_ARG, pk_class * base, uint8 * ptr, void * elem_base, int index, size_t elem_size, int count, void (* release)(void *, void *)) {
	vm_interface_enumerator iface ;
	vm_object * ret = VM_NULL_OBJECT;
	iface.base.base = base;
	iface.base.ptr = NULL;
	if(ptr != NULL) iface.base.ptr = ptr;
	if(base == NULL) return ret;
	iface.elem_base = elem_base;
	iface.index = index;
	iface.elem_size = elem_size;
	iface.count = count;
	ret = vm_create_object(VM_ARG, sizeof(vm_interface_enumerator), (uchar *)&iface);
	ret->type = VM_OBJ_TYPE_INTERFACE;
	ret->release = release;
	return ret;
}

LIB_API vm_object * vm_create_object(VM_DEF_ARG, uint32 length, void * bytes) {
	vm_object * newobj;
	//if (length > 32678) return vm_create_immutable_object(VM_ARG, length, bytes);
	newobj = (vm_object *)m_alloc(vm_get_heap(), sizeof(vm_object) + length);
	newobj->mgc_refcount = (VM_MAGIC | 1);
	newobj->type = VM_OBJ_TYPE_ANY;
	newobj->release = NULL;
	newobj->lock = 0;
	newobj->len = length;
	newobj->next = NULL;
	if(bytes != NULL) memcpy(newobj->bytes, bytes, length);
	return newobj;
}

LIB_API vm_object * vm_create_immutable_object(VM_DEF_ARG, uint32 length, void * bytes) {
	vm_object * newobj = (vm_object *)malloc(sizeof(vm_object) + length);
	newobj->mgc_refcount = (VM_MAGIC | 1);
	newobj->type = VM_OBJ_TYPE_ANY;
	newobj->release = NULL;
	newobj->lock = 0;
	newobj->len = length;
	newobj->next = NULL;
	if (bytes != NULL) memcpy(newobj->bytes, bytes, length);
	vm_add_immutable_object(VM_ARG, newobj);		//add to list of immutable object
	return newobj;
}

LIB_API vm_object * vm_create_arg(VM_DEF_ARG, uint32 length, void * bytes) {
	vm_object * newobj = vm_create_object(VM_ARG, length, bytes);
	newobj->mgc_refcount = (VM_OBJ_MAGIC | 1);
	return newobj;
}

LIB_API vm_object * vm_clone_object(VM_DEF_ARG, vm_object * obj) {
	vm_object * newobj;
	if (obj == NULL) return NULL;
	newobj = vm_create_object(VM_ARG, obj->len, obj->bytes);
	newobj->mgc_refcount = (obj->mgc_refcount & 0xF0) | 1;
	newobj->type = obj->type;
	newobj->release = obj->release;
	return newobj;
}

LIB_API vm_object * vm_create_extension(VM_DEF_ARG, uint8 tag, vm_custom_opcode * apis, uint16 length, uchar * bytes) {
	vm_object * newobj = vm_create_object(VM_ARG, sizeof(vm_extension) + length, NULL);
	newobj->mgc_refcount = (VM_EXT_MAGIC | 1);
	newobj->type = VM_OBJ_TYPE_ANY;
	newobj->release = NULL;
	newobj->lock = 0;
	((vm_extension *)(newobj->bytes))->tag = tag;
	((vm_extension *)(newobj->bytes))->apis = apis;
	if(bytes != NULL) memcpy(((vm_extension *)(newobj->bytes))->payload, bytes, length);
	else memset(((vm_extension *)(newobj->bytes))->payload, 0, length);
	return newobj;
}

LIB_API vm_object * vm_create_reference(VM_DEF_ARG, void * ptr) {
	return vm_create_operand(VM_ARG, VM_OBJ_TYPE_REF | sizeof(void *), ptr, 0);
}

LIB_API vm_object * vm_create_operand(VM_DEF_ARG, uint16 type, void * ptr, uint32 size) {
	vm_object * newobj = vm_create_object(VM_ARG, size, NULL);
	newobj->mgc_refcount = (VM_OPR_MAGIC | 1);
	newobj->type = type;
	if(size != 0) {
		//copy data as array
		newobj->len = size;
		memcpy(newobj->bytes, ptr, size);
		//((vm_operand *)newobj->bytes)->ptr = ((vm_operand *)newobj->bytes)->bytes;
	} else {
		//only copy pointer value
		newobj->type |= VM_OBJ_TYPE_REF;
		newobj->len = sizeof(void *);
		memcpy(newobj->bytes, &ptr, sizeof(void *));
	}
	return newobj;
}

LIB_API void vm_release_object(VM_DEF_ARG, vm_object * obj) {
	if(obj == NULL) return;
	//if(obj->mgc_refcount == 0) {
	m_free(vm_get_heap(), obj);
	//}
	//vm_object_unref(obj);
}

LIB_API pk_object * vm_register_interface(VM_DEF_ARG, const char * name, pk_object * base) {
	pk_object * p;
	p = (pk_object *)pk_select_class(vm_get_info(), (uchar *)name);
	if(p == NULL) {
		p = (pk_object *)pk_create_interface((uchar *)name, (uchar *)"", base);
	}
	p = (pk_object *)pk_add_object(&vm_get_info(), p);
	return p;
}

LIB_API pk_object * vm_select_interface(VM_DEF_ARG, const char * name) {
	pk_object * p;
	p = (pk_object *)pk_select_class(vm_get_info(), (uchar *)name);
	return p;
}

LIB_API pk_object * vm_interface_add_method(VM_DEF_ARG, pk_object * parent, const char * name, uint8 ret_type, uint8 num_args, void (* callback)(), ...) {
	va_list vl;
	pk_object * obj;
	pk_method * pkm;
	int i;
	char fname[256];
	if(parent == NULL) return (pk_object *)parent;
	sprintf(fname, "%s?%d", name, num_args);
	if((pkm = pk_select_method((pk_class *)parent, (uchar *)fname)) == NULL) {
		//printf("register method %s to %s\n", name, fname);
		obj = (pk_object *)pk_register_method((pk_class *)parent, (uchar *)fname, num_args, 0, ret_type, 0, callback); 
		if(obj != NULL) {
			//printf("va_start %d\n", num_args);
			va_start(vl, callback);
			for(i=0;i<num_args;i++) {
				uint8 vtype = va_arg(vl, int);
				///printf("va_arg %d\n", num_args);
				pk_set_arg((pk_method *)obj, i, vtype);
			}
			//printf("va_end %d\n", num_args);
			va_end(vl);
		}
	}
}
