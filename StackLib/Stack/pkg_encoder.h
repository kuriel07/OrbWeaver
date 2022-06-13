#ifndef _PKG_ENCODER__H
#include "../defs.h"
#include <stdio.h>

//use ASN.1 tag values, compatibility purpose
#define ASN_TAG_CONSTRUCTED		0x20

#define ASN_TAG_RSV				0	//reserved
#define ASN_TAG_BOOL			1	//BOOLEAN
#define ASN_TAG_INTEGER			2	//INTEGER
#define ASN_TAG_BSTRING			3	//BIT STRING
#define ASN_TAG_OCTSTRING		4	//OCTET STRING
#define ASN_TAG_NULL			5	//NULL
#define ASN_TAG_OBJID			6	//OBJECT IDENTIFIER
#define ASN_TAG_OBJDESC			7	//ObjectDescriptor
#define ASN_TAG_EXTERNAL		8	//INSTANCE OF, EXTERNAL
#define ASN_TAG_REAL			9	//REAL
#define ASN_TAG_ENUM			10	//ENUMERATED
#define ASN_TAG_EMBEDDED		11	//EMBEDDED PDV
#define ASN_TAG_UTF8			12	//UTF8String
#define ASN_TAG_ROID			13	//RELATIVE-OID
#define ASN_TAG_SEQ				16	//SEQUENCE, SEQUENCE OF
#define ASN_TAG_SET				17	//SET, SET OF
#define ASN_TAG_NUMSTRING		18	//NumericString
#define ASN_TAG_PRINTSTRING		19	//PrintableString
#define ASN_TAG_TELETEX			20	//TeletexString, T61String
#define ASN_TAG_VIDEOTEX		21	//VideotexString
#define ASN_TAG_IA5STRING		22	//IA5String
#define ASN_TAG_UTCTIME			23	//UTCTime
#define ASN_TAG_GENTIME			24	//GeneralizedTime
#define ASN_TAG_GRAPHSTRING		25	//GraphicString
#define ASN_TAG_ISO646			26	//VisibleString, ISO646String
#define ASN_TAG_GENSTRING		27	//GeneralString
#define ASN_TAG_UNISTRING		28	//UniversalString
#define ASN_TAG_CHSTRING		29	//CHARACTER STRING
#define ASN_TAG_BMPSTRING		30	//BMPString

#define PK_TAG_CLASS			0xCA
#define PK_TAG_INTERFACE		0xCC
#define PK_TAG_METHOD			0xE0
#define PK_TAG_PROPERTY			0xA0
#define PK_TAG_DEBUG			0xDB
#define PK_TAG_TCBLOCK			0xEE

typedef struct pk_object {
	uchar tag;
	void * codebase;
	void * next;
} pk_object; 

typedef struct pk_class {
	pk_object base;
	pk_object * parent;
	void * properties;
	uchar name[256];
} pk_class;

typedef struct pk_interface {
	pk_class base;
	uchar libname[256];
} pk_interface;

typedef struct pk_method {
	pk_object base;
	pk_class * parent;
	uchar name[256];
	uchar numargs;
	int offset;				//offset relative to class module
	uchar event;
	uchar alias[64];
	uint8 call_type;
	uint8 ret_type;
	uint8 param_types[16];
	void (* callback)(void );
} pk_method;

typedef struct pk_property {
	pk_object base;
	uchar name[256];
	uint16 offset;				//global offset relative to class
} pk_property;

typedef struct pk_debug_info {
	pk_object base;
	int line;
	int address;
} pk_debug_info;

typedef struct pk_tcblock {
	pk_object base;
	int start;
	int cblock;
	uint8 num_args;
} pk_tcblock;


#ifdef __cplusplus
extern "C" {
#endif

LIB_API pk_class * pk_create_class(uchar * name);
pk_method * pk_create_method(uchar * name, uchar numargs, int offset);
LIB_API pk_interface * pk_create_interface(uchar * name, uchar * libname, pk_object * base);
pk_tcblock * pk_create_tcblock(int start, int cblock, uint8 num_args);
pk_object * pk_add_object(pk_object ** root, pk_object * obj);
pk_tcblock * pk_install_tcblock(pk_object ** root, int start, int cblock, uint8 num_args);
pk_class * pk_install_class(pk_object ** root, uchar * name);
pk_interface * pk_install_interface(pk_object ** root, uchar * name, uchar * libname);
pk_class * pk_select_class(pk_object * root, uchar * name);
pk_class * pk_register_class(uchar * name);
pk_interface * pk_register_interface(uchar * name, uchar * libname);
pk_tcblock * pk_register_tcblock(int start, int end, uint8 num_args);
pk_method * pk_select_method(pk_class * parent, uchar * name);
LIB_API pk_method * pk_register_method(pk_class * parent, uchar * name, uchar numargs, uint8 call_type, uint8 ret_type, uint16 offset, void (* callback)(void));
LIB_API void pk_set_arg(pk_method * method, uint8 id, uint16 type);
LIB_API void pk_set_callback(pk_method * method, void (* callback)());
pk_method * pk_register_method_args(pk_class * parent, uchar * name, uchar numargs);
pk_method * pk_register_menu(pk_class * parent, uchar * name, uchar * aliasname);
pk_method * pk_register_event(pk_class * parent, uchar * name, uchar id);

void pk_set_root(pk_object * root);
pk_object * pk_get_root();
void pk_merge_root(pk_object * root, pk_object * new_root);
pk_object * pk_get_last_root(pk_object * root);
uint32 pk_flush_root();
uint32 pk_flush_objects(pk_object * object, uchar * buffer) ;
uint32 pk_flush_entries(pk_object * object, uchar * buffer) ;

void pk_import_directory(uchar * path);
pk_object * pk_decode_file(uchar * path);
uint16 pk_decode_header_seq(FILE * file, uint16 offset, void * codebase);
void * pk_decode_data_seq(FILE * file, uint16 offset) ;
uint16 pk_decode_class_seq(FILE * file, uint16 offset, void * codebase);
uint16 pk_decode_class_header(FILE * file, uint16 offset, uint16 length, void * codebase);
uint16 pk_decode_method_header(pk_object * parent, FILE * file, uint16 offset, uint16 length, void * codebase);

void pk_init(uchar * inpath);
void pk_file_flush(uchar * codes, uint32 csize);
void pk_set_codebase(pk_object * root, uint8 * codebase);
void pk_dispatch_tc_entries(pk_object ** root, pk_tcblock ** entries);


#ifdef __cplusplus
}
#endif
#define _PKG_ENCODER__H
#endif
