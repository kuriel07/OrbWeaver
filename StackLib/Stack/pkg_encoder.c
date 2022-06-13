#include "../defs.h"
#include "../config.h"
#include "pkg_encoder.h"
#include "scr_generator.h"
#include <stdio.h>

pk_object * _pk_root = NULL;
uchar _pkg_buffer[65536];
FILE * _pkfile = NULL;

void pk_set_root(pk_object * root) {
	_pk_root = root;
}

pk_object * pk_get_root() {
	return _pk_root;
}

pk_tcblock * pk_create_tcblock(int start, int cblock, uint8 num_args) {
	pk_tcblock * ptc = (pk_tcblock *)malloc(sizeof(pk_tcblock));
	((pk_object *)ptc)->tag = PK_TAG_TCBLOCK;
	((pk_object *)ptc)->next = NULL;
	((pk_object *)ptc)->codebase = NULL;
	ptc->start = start;
	ptc->cblock = cblock;
	ptc->num_args = num_args;
	return ptc;
}

pk_class * pk_install_class(pk_object ** root, uchar * name) {
	pk_class * pkc = pk_create_class(name);
	return (pk_class *)pk_add_object(root, (pk_object *)pkc);
}

pk_interface * pk_install_interface(pk_object ** root, uchar * name, uchar * libname) {
	pk_interface * pkc = pk_create_interface(name, libname, NULL);
	return (pk_interface *)pk_add_object(root, (pk_object *)pkc);
}

pk_tcblock * pk_install_tcblock(pk_object ** root, int start, int cblock, uint8 num_args) {
	pk_tcblock * ptc = pk_create_tcblock(start, cblock, num_args);
	return (pk_tcblock *)pk_add_object(root, (pk_object *)ptc);
}

void pk_merge_root(pk_object * root, pk_object * new_root) {
	pk_object * iterator = (pk_object *)root;
	if(iterator == NULL) return;
	while(iterator->next != NULL) {
		iterator = (pk_object *)iterator->next;
	}
	iterator->next = new_root;
}

pk_object * pk_get_last_root(pk_object * root) {
	pk_object * iterator = (pk_object *)root;
	if(iterator == NULL) return NULL;
	while(iterator->next != NULL) {
		iterator = (pk_object *)iterator->next;
	}
	return iterator;
}

pk_class * pk_register_class(uchar * name) {
	return pk_install_class(&_pk_root, name);
}

pk_interface * pk_register_interface(uchar * name, uchar * libname) {
	return pk_install_interface(&_pk_root, name, libname);
}

pk_tcblock * pk_register_tcblock(int start, int end, uint8 num_args) {
	return pk_install_tcblock(&_pk_root, start, end, num_args);
}

pk_method * pk_register_method_args(pk_class * parent, uchar * name, uchar numargs) {
	pk_method * pkm = pk_select_method(parent, name);
	if(pkm != NULL) {
		pkm->numargs = numargs;
	}
	return pkm;
}

pk_method * pk_register_menu(pk_class * parent, uchar * name, uchar * aliasname) {
	pk_method * pkm = pk_select_method(parent, name);
	if(pkm == NULL) return pkm;
	strcpy(_RECAST(char *, pkm->alias), _RECAST(const char *, aliasname));
#ifdef STANDALONE_COMPILER
	sc_install_menu(aliasname, pkm);
#endif
	return pkm;
}

pk_method * pk_register_event(pk_class * parent, uchar * name, uchar id) {
	pk_method * pkm = pk_select_method(parent, name);
	if(pkm == NULL) return pkm;
	pkm->event = id;
#ifdef STANDALONE_COMPILER
	sc_install_event(id, pkm);
#endif
	return pkm;
}

static uint32 pk_push_tlv(uchar t, uint16 l, uchar * v, uchar * buffer) {
	uint16 sz = 0;
	buffer[0] = t;
	if(l < 127) {
		buffer[1] = l;
		sz = 2;
	} else {
		buffer[1] = 0x82;
		buffer[2] = (uchar)(l >> 8);
		buffer[3] = (uchar)l;
		sz = 4;
	}
	memcpy(buffer + sz, v, l);
	return (l + sz);
}

uint32 pk_flush_root() {								//flush root with ASN.1 BER(X.690) Basic Encoding Rules
	uint32 sz = 0, csz = 0;
	csz = pk_flush_entries(_pk_root, _pkg_buffer + 6);
	csz += pk_flush_objects(_pk_root, _pkg_buffer + csz + 6);
	sz += pk_push_tlv(ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED, csz, _pkg_buffer + 6, _pkg_buffer);
	return sz;
}

uint32 pk_flush_entries(pk_object * object, uchar * buffer) {
	pk_object * iterator = object;
	uint32 sz = 0, csz = 0;
	uint16 offset = 0;
	uchar t_buffer[128];
	while(iterator != NULL) {
		switch(iterator->tag) {
			case PK_TAG_INTERFACE:
			case PK_TAG_CLASS:
				if(((pk_class *)iterator)->properties != NULL) {
					sz += pk_flush_entries((pk_object *)((pk_class *)iterator)->properties, buffer + sz);
				}
				//sz += pk_push_tlv(ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED, csz, buffer + sz + 2, buffer + sz);

				break;
			case PK_TAG_METHOD:

				offset = end_swap16(((pk_method *)iterator)->offset);
				csz = 0;
				if(strlen(_RECAST(const char *,((pk_method *)iterator)->alias)) != 0) {
					memcpy(t_buffer, &offset, 2);
					strcpy(_RECAST(char *, t_buffer) + 2, _RECAST(const char *, ((pk_method *)iterator)->alias));
					//printf("menu %s\n", ((pk_method *)iterator)->alias);
					sz += pk_push_tlv(ASN_TAG_OCTSTRING, strlen(_RECAST(const char *,((pk_method *)iterator)->alias)) + 2, t_buffer, buffer + sz + csz);
				}
				if(((pk_method *)iterator)->event != 0xFF) {
					memcpy(t_buffer, &offset, 2);
					t_buffer[2] = ((pk_method *)iterator)->event;
					sz += pk_push_tlv(ASN_TAG_INTEGER, 3, t_buffer, buffer + sz + csz);
				}
				break;
			case PK_TAG_PROPERTY: break;
		}	
		iterator = (pk_object *)iterator->next;
		//buffer += sz;
	}
	//sz += pk_push_tlv(ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED, csz, buffer + sz + 2, buffer + sz);
	return sz;
}

uint32 pk_flush_objects(pk_object * object, uchar * buffer) {
	pk_object * iterator = object;
	uint32 sz = 0, csz = 0, dsz = 0;
	uint16 offset = 0;
	uint32 offset32;
	uchar d_buffer[4096];
	uint8 tag;
	while(iterator != NULL) {
		csz = 0;
		switch(iterator->tag) {
			case PK_TAG_INTERFACE:
				csz += pk_push_tlv(ASN_TAG_CHSTRING, strlen(_RECAST(const char *, ((pk_interface *)iterator)->libname)), ((pk_interface *)iterator)->libname, buffer + sz + csz + 4);
			case PK_TAG_CLASS:
				csz += pk_push_tlv(ASN_TAG_IA5STRING, strlen(_RECAST(const char *, ((pk_class *)iterator)->name)), ((pk_class *)iterator)->name, buffer + sz + csz + 4);
				if(((pk_class *)iterator)->properties != NULL) {
					dsz = csz;
					csz += pk_flush_objects((pk_object *)((pk_class *)iterator)->properties, buffer + sz + csz + 2);
				}
				sz += pk_push_tlv(ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED, csz, buffer + sz + 4, buffer + sz);
				break;
			case PK_TAG_METHOD:
				dsz = 0;
				if(((pk_object *)((pk_method *)iterator)->parent)->tag == PK_TAG_INTERFACE) {
					dsz = ((pk_method *)iterator)->numargs;	
					//return type
					d_buffer[0] = ((pk_method *)iterator)->call_type;
					d_buffer[1] = ((pk_method *)iterator)->ret_type;
					d_buffer[2] = ((pk_method *)iterator)->numargs;
					//arguments type
					memcpy(d_buffer + 3, ((pk_method *)iterator)->param_types, ((pk_method *)iterator)->numargs);
					tag = ASN_TAG_OBJDESC;
				} else {
					offset = end_swap16(((pk_method *)iterator)->offset);
					memcpy(d_buffer, (uchar *)&offset, 2);
					d_buffer[2] = ((pk_method *)iterator)->numargs;
					tag = ASN_TAG_OCTSTRING;
				}				
				memcpy(d_buffer + dsz + 3, _RECAST(const char *, ((pk_method *)iterator)->name), strlen(_RECAST(const char *,((pk_method *)iterator)->name)));
				sz += pk_push_tlv(tag, strlen(_RECAST(const char *, ((pk_method *)iterator)->name)) + dsz + 3, d_buffer, buffer + sz + csz + 2);
				break;
			case PK_TAG_PROPERTY: break;
			case PK_TAG_TCBLOCK:
				offset32 = end_swap32(((pk_tcblock *)iterator)->start);
				memcpy(d_buffer, (uchar *)&offset32, 4);
				offset32 = end_swap32(((pk_tcblock *)iterator)->cblock);
				memcpy(d_buffer + 4, (uchar *)&offset32, 4);
				d_buffer[8] = 0;		//reserved byte
				d_buffer[9] = ((pk_tcblock *)iterator)->num_args;
				sz += pk_push_tlv(ASN_TAG_ENUM, 10, d_buffer, buffer + sz);
				break;
		}	
		iterator = (pk_object *)iterator->next;
		//buffer += sz;
	}
	return sz;
}

void pk_init(uchar * inpath) {
//#ifdef STANDALONE_COMPILER
	uchar pkpath[512];
	uchar index = 0;
	_pk_root = NULL;
	printf("output : %s\n", pkpath);
	if(inpath == NULL) return;
	strcpy(_RECAST(char *,pkpath), _RECAST(const char *, inpath));
	index = (uint32)strchr(_RECAST( char *, pkpath), '.') - (uint32)pkpath;
	pkpath[index] = 0;
	sprintf(_RECAST(char *,pkpath), "%s%s", pkpath, ".orb");
	printf("output : %s\n", pkpath);
	_pkfile = fopen(_RECAST(const char *, pkpath), "wb");
	if(_pkfile == NULL) return;
//#endif
}

void pk_file_flush(uchar * codes, uint32 csize) {
//#ifdef STANDALONE_COMPILER
	uint32 size = 0;
	if(_pkfile != NULL) {
		size = pk_flush_root();	
		//install codes for script generator
#ifdef STANDALONE_COMPILER
		sc_install_header(_pkg_buffer, size);			//install package header
		sc_install_codes(codes, csize);				//install raw codes
#endif
		size += pk_push_tlv(ASN_TAG_BMPSTRING, csize, codes, _pkg_buffer + size);
		fwrite(_pkg_buffer, size, 1, _pkfile);
		fclose(_pkfile);
	}
//#endif
}

void pk_set_codebase(pk_object * root, uint8 * codebase) {
	pk_method * itr;
	pk_class * class_itr;
	class_itr = (pk_class *)root;
#if IS_CODEDEBUG
	printf("===================== CLASSES DUMP =====================\n");
	printf("root\n");
#endif
	while(class_itr != NULL) 
	{
		if (((pk_object *)class_itr)->tag == PK_TAG_CLASS || ((pk_object *)class_itr)->tag == PK_TAG_METHOD || ((pk_object *)class_itr)->tag == PK_TAG_INTERFACE) {
			if (((pk_object *)class_itr)->next != NULL) {
#if IS_CODEDEBUG
				printf("    \xc3%s\n", class_itr->name);
#endif
				if (class_itr->properties != NULL) {
					itr = (pk_method *)class_itr->properties;
					while (itr != NULL) {
#if IS_CODEDEBUG
						if (((pk_object *)itr)->next != NULL) {
							printf("    \xb3    \xc3%s\n", itr->name);
						}
						else {
							printf("    \xb3    \xc0%s\n", itr->name);
						}
#endif
						((pk_object *)itr)->codebase = codebase;
						itr = (pk_method *)((pk_object *)itr)->next;
					}
				}
			}
			else {
#if IS_CODEDEBUG
				printf("    \xc0%s\n", class_itr->name);
#endif
				if (class_itr->properties != NULL) {
					itr = (pk_method *)class_itr->properties;
					while (itr != NULL) {
#if IS_CODEDEBUG
						if (((pk_object *)itr)->next != NULL) {
							printf("         \xc3%s\n", itr->name);
						}
						else {
							printf("         \xc0%s\n", itr->name);
						}
#endif
						((pk_object *)itr)->codebase = codebase;
						itr = (pk_method *)((pk_object *)itr)->next;
					}
				}
			}
		}
		((pk_object *)class_itr)->codebase = codebase;
		class_itr = (pk_class *)((pk_object *)class_itr)->next;
	}
#if IS_CODEDEBUG
	printf("=====================   END DUMP   =====================\n");
#endif
}

void pk_dispatch_tc_entries(pk_object ** root, pk_tcblock ** entries) {
	pk_object * iterator = root[0];
	pk_object * prev_iterator = NULL;
	pk_object * temp;
	while (iterator != NULL) {
		if (iterator->tag == PK_TAG_TCBLOCK) {
			if (prev_iterator == NULL) root[0] = (pk_object *)iterator->next;
			temp = iterator;
			iterator = (pk_object *)iterator->next;
			temp->next = NULL;
			pk_add_object((pk_object **)entries, temp);
		}
		else {
			prev_iterator = iterator;
			iterator = (pk_object *)iterator->next;
		}
	}
}
