#include "../defs.h"
#include "../config.h"
#include "pkg_encoder.h"
#include "scr_generator.h"
#include <stdio.h>


pk_method * pk_create_method(uchar * name, uchar numargs, int offset) {
	pk_method * pkm = (pk_method *)malloc(sizeof(pk_method));
	((pk_object *)pkm)->tag = PK_TAG_METHOD;
	((pk_object *)pkm)->next = NULL;
	((pk_object *)pkm)->codebase = NULL;
	memset(pkm->name, 0, sizeof(pkm->name));
	strcpy(_RECAST(char *,pkm->name), _RECAST(const char *, name));
	pkm->parent = NULL;
	pkm->numargs = numargs;
	pkm->offset = offset;
	pkm->event = 0xFF;
	memset(pkm->alias, 0, sizeof(pkm->alias));
	return pkm;
}

pk_class * pk_select_class(pk_object * root, uchar * name) {
	pk_object * iterator = (pk_object *)root;
	if(iterator == NULL) return (pk_class *)iterator;
	while(iterator != NULL) {
		if(strcmp(_RECAST(const char *,((pk_class *)iterator)->name), _RECAST(const char *, name)) == 0) {
			return (pk_class *)iterator;
		}
		iterator = (pk_object *)iterator->next;
	}
	return (pk_class *)iterator;
}

pk_method * pk_select_method(pk_class * parent, uchar * name) {
	pk_object * iterator;
	if(parent == NULL) return NULL;
	if(((pk_object *)parent)->tag != PK_TAG_CLASS && ((pk_object *)parent)->tag != PK_TAG_INTERFACE) return NULL;
	iterator = (pk_object *)parent->properties;
	if(iterator == NULL) return (pk_method *)iterator;
	while(iterator != NULL) {
		if(strcmp(_RECAST(const char *,((pk_method *)iterator)->name), _RECAST(const char *, name)) == 0) {
			return (pk_method *)iterator;
		}
		iterator = (pk_object *)iterator->next;
	}
	//search for base class method
	if(((pk_class *)parent)->parent != NULL) {
		printf("try selecting base parent %s\n", ((pk_class *)((pk_class *)parent)->parent)->name);
		return pk_select_method(_RECAST(pk_class *, ((pk_class *)parent)->parent), name);
	}
	//not found
	return (pk_method *)NULL;
}

pk_object * pk_add_object(pk_object ** root, pk_object * obj) {
	pk_object * iterator = (pk_object *)*root;
	//if(iterator == NULL) return NULL;
	if(*root == NULL) *root = (pk_object *)obj;
	else {
		while(iterator->next != NULL) {
			iterator = (pk_object *)iterator->next;
		}
		iterator->next = obj;
	}
	return obj;
}

LIB_API pk_class * pk_create_class(uchar * name) {
	pk_class * pkc = (pk_class *)malloc(sizeof(pk_class));
	((pk_object *)pkc)->tag = PK_TAG_CLASS;
	((pk_object *)pkc)->next = NULL;
	((pk_object *)pkc)->codebase = NULL;
	memset(pkc->name, 0, sizeof(pkc->name));
	strcpy(_RECAST(char *,pkc->name), _RECAST(const char *, name));
	pkc->properties = NULL;
	return pkc;
}

LIB_API pk_interface * pk_create_interface(uchar * name, uchar * libname, pk_object * base) {
	pk_interface * pkc = (pk_interface *)malloc(sizeof(pk_interface));
	((pk_object *)pkc)->tag = PK_TAG_INTERFACE;
	((pk_object *)pkc)->next = NULL;
	((pk_object *)pkc)->codebase = NULL;
	memset(((pk_class *)pkc)->name, 0, sizeof(((pk_class *)pkc)->name));
	memset(pkc->libname, 0, sizeof(pkc->libname));
	strcpy(_RECAST(char *,((pk_class *)pkc)->name), _RECAST(const char *, name));
	strcpy(_RECAST(char *,pkc->libname), _RECAST(const char *, libname));
	((pk_class *)pkc)->parent = base;
	((pk_class *)pkc)->properties = NULL;
	return pkc;
}

LIB_API pk_method * pk_register_method(pk_class * parent, uchar * name, uchar numargs, uint8 call_type, uint8 ret_type, uint16 offset, void (* callback)(void)) {
	pk_object * iterator;
	pk_method * pkm;
	if(parent == NULL) return (pk_method *)parent;
	pkm = pk_create_method(name, numargs, offset);
	((pk_method *)pkm)->parent = parent;
	((pk_method *)pkm)->call_type = call_type;
	((pk_method *)pkm)->ret_type = ret_type;
	((pk_method *)pkm)->callback = callback;
	iterator = (pk_object *)parent->properties;
	if(iterator == NULL) parent->properties = pkm;
	else {
		while(iterator->next != NULL) {
			iterator = (pk_object *)iterator->next;
		}
		iterator->next = pkm;
	}
	return pkm;
}

LIB_API void pk_set_arg(pk_method * method, uint8 id, uint16 type) {
	if(method != NULL) {
		if(id > 15) return;
		method->param_types[id] = type;
	}
}

LIB_API void pk_set_callback(pk_method * method, void (* callback)()) {
	if(method != NULL) {
		method->callback = callback;
	}
}
