#include "../defs.h"
#include "../config.h"
#ifdef STANDALONE_COMPILER
#include "dirent.h"
#endif
#include "pkg_encoder.h"
#include "pkg_linker.h"
#include <stdio.h>

pk_object * _pk_iroot = NULL;

void lk_set_root(pk_object * root) {
	_pk_iroot = root;
}

pk_object * lk_get_root() {
	return _pk_iroot;
}

pk_class * lk_select_class(pk_object * root, uchar * name) {
	return pk_select_class(root, name);
}

pk_interface * lk_select_interface(pk_object * root, uchar * name) {
	return (pk_interface * )pk_select_class(root, name);
}

pk_method * lk_select_method(pk_class * parent, uchar * name) {
	return pk_select_method(parent, name);
}

void lk_import_directory(uchar * path) {
#ifdef STANDALONE_COMPILER
	DIR * dfd;
	struct dirent *dp;
	uchar filename_qfd[256] ;
	int size;
	if ((dfd = opendir(_RECAST(const char *,path))) == NULL) return;
	while ((dp = readdir(dfd)) != NULL)
	{
		sprintf( _RECAST(char *, filename_qfd) , "%s\\%s", path, dp->d_name) ;
		lk_decode_file(filename_qfd, &size);
	}
#endif
}

pk_object * lk_decode_file(uchar * path, int * size) {
	uchar t;
	uint16 l = 0;
	pk_object * pkg = NULL;
	void  * codebase = NULL;
	uint16 offset = 0, sz = -1;
	FILE * file = fopen(_RECAST(const char *, path), "rb");
	if(file == NULL) return pkg;
	fseek(file, offset, SEEK_SET);
	offset += fread(&t, 1, 1, file);
	switch(t & 0x1f) {
		case ASN_TAG_SEQ:			//valid class sequence
			offset += fread(&t, 1, 1, file);		//l
 			if(t & 0x80) {
				offset += fread(&l, 1, 2, file);
				l = end_swap16(l);
			} else {
				l = t;
			}
			break;
		default: break;
	}
	offset += l;
	//printf("%d\n", offset);
	if(offset != 0) {
		codebase = lk_decode_data_seq(file, offset, size);
	}
	//printf("code base : %d\n", codebase);
	offset = 0;
	offset = lk_decode_header_seq(file, offset, codebase);
	return (pk_object * )codebase;
}

void * lk_decode_data_seq(FILE * file, uint16 offset, int * size) {
	uchar t;
	uint16 l = 0;
	uint16 e_offset = 0;
	uchar * codebase = NULL;
	fseek(file, offset, SEEK_SET);
	offset += fread(&t, 1, 1, file);
	size[0] = 0;
	switch(t & 0x1f) {
		case ASN_TAG_BMPSTRING:			//valid class sequence
			offset += fread(&t, 1, 1, file);		//l
 			if(t & 0x80) {
				offset += fread(&l, 1, 2, file);
				l = end_swap16(l);
			} else {
				l = t;
			}
			codebase = (uchar *)malloc(l);
			size[0] = l;
			fread(codebase, 1, l, file);
			//printf("size %d\n", l);
			//offset += csz;
			break;
		default:
			offset = 0;
			break;
	}
	return codebase;
}

uint16 lk_decode_header_seq(FILE * file, uint16 offset, void * codebase) {
	uchar t;
	uint16 l = 0;
	uint16 e_offset = 0;
	fseek(file, offset, SEEK_SET);
	offset += fread(&t, 1, 1, file);
	switch(t & 0x1f) {
		case ASN_TAG_SEQ:			//valid class sequence
			offset += fread(&t, 1, 1, file);		//l
 			if(t & 0x80) {
				offset += fread(&l, 1, 2, file);
			} else {
				l = t;
			}
			e_offset = offset + l;
			while(offset < e_offset) {
				offset = lk_decode_class_seq(file, offset, codebase);
			}
			//offset += csz;
			break;
		default:
			
			offset = 0;
			break;
	}
	return offset;
}

uint16 lk_decode_class_seq(FILE * file, uint16 offset, void * codebase) {
	uchar t;
	uint16 l = 0;
	uint16 csz = 0;
	fseek(file, offset, SEEK_SET);
	offset += fread(&t, 1, 1, file);
	switch(t & 0x1f) {
		case ASN_TAG_SEQ:			//valid class sequence
			offset += fread(&t, 1, 1, file);		//l
 			if(t & 0x80) {
				offset += fread(&l, 1, 2, file);
			} else {
				l = t;
			}
			offset = lk_decode_class_header(file, offset, l, codebase);
			break;
		case ASN_TAG_OCTSTRING:			//valid menu entry 
		case ASN_TAG_INTEGER:			//valid event entry
			offset += fread(&t, 1, 1, file);		//l
 			if(t & 0x80) {
				offset += fread(&l, 1, 2, file);
			} else {
				l = t;
			}
			offset = offset + l;
			break;
		default:
			offset = 0;
			break;
	}
	return offset;
}

uint16 lk_decode_class_header(FILE * file, uint16 offset, uint16 length, void * codebase) {
	uchar t;
	uint16 l = 0;
	uint16 csz = 0;
	uchar buffer[4096];
	uchar numargs = 0;
	uint16 m_offset = 0;
	uint8 call_type;
	uint8 ret_type;
	uchar args_buffer[16];
	pk_object * parent = NULL;
	pk_object * method_obj;
	uint16 e_offset = offset + length, d_offset = 0;
	fseek(file, offset, SEEK_SET);
	while(offset < e_offset) {
		offset += fread(&t, 1, 1, file);
		memset(args_buffer, 0, sizeof(args_buffer));
		memset(buffer, 0, sizeof(buffer));
		switch(t & 0x1f) {
			case ASN_TAG_CHSTRING:			//library name
				offset += fread(&t, 1, 1, file);		//l
	 			if(t & 0x80) {
					offset += fread(&l, 1, 2, file);
				} else {
					l = t;
				}
				offset += fread(buffer, 1, l, file);
				buffer[l] = 0;
				parent = (pk_object *)pk_install_interface(&_pk_iroot, (uchar *)"", buffer);
				parent->codebase = codebase;
				break;
			case ASN_TAG_IA5STRING:			//valid class sequence
				offset += fread(&t, 1, 1, file);		//l
	 			if(t & 0x80) {
					offset += fread(&l, 1, 2, file);
				} else {
					l = t;
				}
				offset += fread(buffer, 1, l, file);
				buffer[l] = 0;
				if(parent != NULL) { 
					sprintf((char *)((pk_class *)parent)->name, "%s", (char *)buffer);
				} else {
					parent = (pk_object *)pk_install_class(&_pk_iroot, buffer);
					parent->codebase = codebase;
				}
				break;
			case ASN_TAG_OCTSTRING:		//method
				offset += fread(&t, 1, 1, file);		//l
	 			if(t & 0x80) {
					offset += fread(&l, 1, 2, file);
				} else {
					l = t;
				}

				if(l <= 3) return offset;
				offset += fread(&m_offset, 1, 2, file);
				offset += fread(&numargs, 1, 1, file);
				offset += fread(buffer, 1, l - 3, file);
				buffer[l-3] = 0;
				printf("method : %s\n", buffer);
				method_obj = (pk_object *)pk_register_method((pk_class *)parent, buffer, numargs, 0, 0, m_offset, NULL);
				method_obj->codebase = codebase;
				break;
			case ASN_TAG_OBJDESC:		//interface method
				offset += fread(&t, 1, 1, file);		//l
	 			if(t & 0x80) {
					offset += fread(&l, 1, 2, file);
				} else {
					l = t;
				}
				if(l <= 3) return offset;
				offset += fread(&call_type, 1, 1, file);
				offset += fread(&ret_type, 1, 1, file);
				offset += fread(&numargs, 1, 1, file);
				offset += fread(args_buffer, 1, numargs, file);
				if(l <= (3 + numargs)) return offset;
				offset += fread(buffer, 1, l - (3 + numargs), file);
				buffer[l-3] = 0;
				printf("method : %s\n", buffer);
				method_obj = (pk_object *)pk_register_method((pk_class *)parent, buffer, numargs, call_type, ret_type, m_offset, NULL);
				method_obj->codebase = codebase;
				memcpy(((pk_method *)method_obj)->param_types, args_buffer, numargs);
				break;
			default:
				offset = 0;
				break;
		}
	}
	return offset;
}

uint16 lk_decode_method_header(pk_object * parent, FILE * file, uint16 offset, uint16 length, void * codebase) {
	uchar t;
	uint16 l = 0;
	uint16 csz = 0;
	uchar buffer[4096];
	uchar numargs = 0;
	uint8 call_type;
	uint8 ret_type;
	uchar args_buffer[16];
	uint16 m_offset = 0;
	uint16 e_offset = offset + length;
	pk_object * method_obj;
	memset(buffer, 0, sizeof(buffer));
	memset(args_buffer, 0, sizeof(args_buffer));
	fseek(file, offset, SEEK_SET);
	offset += fread(&t, 1, 1, file);
	if((t & 0x1f) == ASN_TAG_OCTSTRING) {
		offset += fread(&t, 1, 1, file);		//l
			if(t & 0x80) {
			offset += fread(&l, 1, 2, file);
		} else {
			l = t;
		}
		if(l <= 3) return offset;
		offset += fread(&m_offset, 1, 2, file);
		offset += fread(&numargs, 1, 1, file);
		offset += fread(buffer, 1, l, file);
	} else if((t & 0x1f) == ASN_TAG_OBJDESC) {
		offset += fread(&t, 1, 1, file);		//l
			if(t & 0x80) {
			offset += fread(&l, 1, 2, file);
		} else {
			l = t;
		}
		if(l <= 3) return offset;
		//offset += fread(&m_offset, 1, 2, file);
		offset += fread(&call_type, 1, 1, file);
		offset += fread(&ret_type, 1, 1, file);
		offset += fread(&numargs, 1, 1, file);
		offset += fread(args_buffer, 1, numargs, file);
		offset += fread(buffer, 1, l, file);
	}
	printf("register method %s, args : %d, offset : %d\n", buffer, numargs, m_offset);
	method_obj = (pk_object *)pk_register_method((pk_class *)parent, buffer, numargs, call_type, ret_type, m_offset, NULL);
	method_obj->codebase = codebase;
	memcpy(((pk_method *)method_obj)->param_types, args_buffer, numargs);
	return offset;
}

void lk_dump_classes() {
	pk_method * itr;
	pk_class * class_itr = (pk_class *)_pk_iroot;
	printf("===================== CLASSES DUMP =====================\n");
	printf("root\n");
	while(class_itr != NULL) 
	{
		if (((pk_object *)class_itr)->tag == PK_TAG_CLASS || ((pk_object *)class_itr)->tag == PK_TAG_METHOD || ((pk_object *)class_itr)->tag == PK_TAG_INTERFACE) {
			if (((pk_object *)class_itr)->next != NULL) {
				printf("    \xc3%s\n", class_itr->name);
				if (class_itr->properties != NULL) {
					itr = (pk_method *)class_itr->properties;
					while (itr != NULL) {
						if (((pk_object *)itr)->next != NULL) {
							printf("    \xb3    \xc3%s\n", itr->name);
						}
						else {
							printf("    \xb3    \xc0%s\n", itr->name);
						}
						itr = (pk_method *)((pk_object *)itr)->next;
					}
				}
			}
			else {
				printf("    \xc0%s\n", class_itr->name);
				if (class_itr->properties != NULL) {
					itr = (pk_method *)class_itr->properties;
					while (itr != NULL) {
						if (((pk_object *)itr)->next != NULL) {
							printf("         \xc3%s\n", itr->name);
						}
						else {
							printf("         \xc0%s\n", itr->name);
						}
						itr = (pk_method *)((pk_object *)itr)->next;
					}
				}
			}
		}
		class_itr = (pk_class *)((pk_object *)class_itr)->next;
	}
	printf("=====================   END DUMP   =====================\n");
}

void lk_clear_entries() {
	pk_method * itr;
	void * obj;
	pk_class * class_itr = (pk_class *)_pk_iroot;
	//printf("===================== CLASSES DUMP =====================\n");
	//printf("root\n");
	while(class_itr != NULL) 
	{
		if(((pk_object *)class_itr)->next != NULL) {
			//printf("    \xc3%s\n", class_itr->name);
			//fflush(0);
			if(class_itr->properties != NULL) {
				itr = (pk_method *)class_itr->properties;
				while(itr != NULL) {
					obj = itr;
					itr = (pk_method *)((pk_object *)itr)->next;
					free(obj);
				}
			}
		} else {
			//printf("    \xc0%s\n", class_itr->name);
			//fflush(0);
			if(class_itr->properties != NULL) {
				itr = (pk_method *)class_itr->properties;
				while(itr != NULL) {
					obj = itr;
					itr = (pk_method *)((pk_object *)itr)->next;
					free(obj);
				}
			}
		}
		obj = class_itr;
		class_itr = (pk_class *)((pk_object *)class_itr)->next;
		free(obj);
	}
	_pk_iroot = NULL;
	//printf("=====================   END DUMP   =====================\n");
}


