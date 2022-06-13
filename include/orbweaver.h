#ifndef ORBWEAVER__H
#define ORBWEAVER__H

//type definition
typedef int int32;
typedef int eint32;
typedef unsigned long uint32;
typedef unsigned long euint32;
typedef unsigned long ulong;
typedef short int16;
typedef unsigned short uint16;
typedef unsigned short euint16;
//typedef unsigned short uint;
typedef unsigned char uchar;
typedef char eint8;
typedef unsigned char euint8;
typedef char int8;
typedef unsigned char uint8;

#if defined(WIN32) && !defined(__MINGW32__) && \
  (!defined(_MSC_VER) || _MSC_VER<1600) && !defined(__WINE__)
#include <BaseTsd.h>
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef __int64 int64;
typedef unsigned __int64 uint64;
#endif

#if (defined(WIN64) || defined(WIN32)) && (defined(_MSC_VER) && _MSC_VER>=1600)
#include <stdint.h>
typedef __int64 int64;
typedef unsigned __int64 uint64;
#endif

#if defined( __GNUC__)
#include <stdint.h>
typedef int64_t int64;
typedef uint64_t uint64;
#endif		//end __GNUC__

//@dir unsigned long * @dir zpage_ptr_to_zero_page;
#ifdef __cplusplus_cli
#define _RECAST(T, x) reinterpret_cast<T>(x)
#else
#define _RECAST(T, x) ((T)x)
#endif

#if defined(_MSC_VER)	//  Microsoft 
#define LIB_API __declspec(dllexport)
#define EXPORT	LIB_API
#define IMPORT __declspec(dllimport)

#if defined (_M_X64) || defined(_M_AMD64) || defined(_M_ARM64)
typedef uint64 intptr;
#else
typedef int intptr;
#endif

#elif defined(__GNUC__)	//  GCC
#define LIB_API __attribute__((visibility("default")))
#define EXPORT	LIB_API
#define IMPORT

#if defined(__aarch64__) || defined(__x64__) || defined(__x86_64__)
typedef uint64 intptr;
#else
typedef int intptr;
#endif

#else
//  do nothing and hope for the best?
#define LIB_API
#define EXPORT
#define IMPORT
typedef int intptr;
#pragma warning Unknown dynamic link import/export semantics.
#endif

typedef intptr u_ptr;			//typedef cast untuk pointer, diganti menyesuaikan target hardware


#define VM_OBJ_TYPE_ARR			0x0040
#define VM_OBJ_TYPE_REF			0x0080
#define VM_OBJ_TYPE_UNSIGNED		0x0020
#define VM_OBJ_TYPE_PRECISION		0x0010
#define VM_OBJ_TYPE_ANY			0x0000

#define VM_INT8					sizeof(int8)
#define VM_INT16				sizeof(int16)
#define VM_INT32				sizeof(int32)
#define VM_INT64				sizeof(int64)
#define VM_FLOAT				(VM_OBJ_TYPE_PRECISION | sizeof(float))
#define VM_DOUBLE				(VM_OBJ_TYPE_PRECISION | sizeof(double))
#define VM_STRING				(VM_OBJ_TYPE_ARR | sizeof(int8))
#define VM_FUNCTION				(VM_OBJ_TYPE_ARR | sizeof(int8))
#define VM_POINTER				sizeof(void *)	
#define VM_VAR					(VM_OBJ_TYPE_ANY | 0)

typedef void * pk_class_p;
typedef void * vm_object_p;
typedef void * pk_method_p;
typedef void * pk_object_p;

#define CAST_FUNC(x)		((void (*)())x)

#define DAL_TYPE_ARRAY		0x4000
#define DAL_TYPE_RECORD		0x4000
#define DAL_TYPE_ROWS		0x8000
#define DAL_TYPE_COLUMN		0x6000
#define DAL_TYPE_INTEGER	0x0001
#define DAL_TYPE_REAL		0x0002
#define DAL_TYPE_STRING		0x0010
#define DAL_TYPE_BYTES		0x0080
#define DAL_TYPE_BIGINT		0x0004

#define DAL_CONN_PGSQL		0x01

typedef struct dal_conn dal_conn;
typedef struct dal_prepared_statement dal_prepared_statement;
typedef struct dal_result dal_result;
typedef struct dal_node dal_node;
typedef struct dal_array dal_array;
typedef struct dal_key dal_key;
typedef struct dal_key dal_column;
typedef struct dal_array dal_record;
typedef struct dal_data dal_data;
typedef struct dal_data dal_cell;
typedef struct dal_interface dal_interface;

struct dal_interface {
	//open connection
	uint8 (*open)(dal_conn * conn);
	//close connection
	void (*close)(dal_conn * conn);
	//query statement
	dal_result * (*query)(dal_conn * conn, char * statement);
	//prepare statement
	dal_prepared_statement * (*prepare)(dal_conn * conn, char * name, char * statement);
	//execute statement
	dal_result * (*execute)(dal_conn * conn, dal_prepared_statement * statement, int count, char * args[]);
	//fetch all result
	dal_array * (*fetch)(dal_result *);
	//get result count
	int (*count)(dal_result *);
	//get first result
	dal_array * (*first)(dal_result *);
	//get next result
	dal_array * (*next)(dal_result *);
};

struct dal_conn {
	uint8 type;
	dal_interface * backend;
	void * conn;
	char connect_str[512];
	void * heap;
	dal_prepared_statement * statements;
	dal_result * results;		//keep track of all result
};

struct dal_prepared_statement {
	dal_conn * conn;
	dal_prepared_statement * next;
	char name[256];
	void * prepared_statement;
	char query[1];
};

struct dal_result {
	dal_conn * conn;
	dal_result * next;
	dal_column * columns;
	void * enumerator;
	void * records;
	void * result;
};

struct dal_node {
	dal_conn * conn;
	uint16 type;
	dal_node * next;
	dal_node * next_node;
};

struct dal_key {
	dal_node base;
	int index;
	uint8 name[1];
};

struct dal_array {
	dal_node base;
	dal_column * columns;
};

struct dal_data {
	dal_node base;
	uint32 length;
	uint8 data[1];
};

struct ticktime {
	long sec;
	long ns;
};

struct datetime {
	uint8 second;
	uint8 minute;
	uint8 hour;
	uint8 day;
	uint8 date;
	uint8 month;
	uint16 year;	
	uint16 days;			//days of year
	int8 tz;				//timezone
};
typedef struct ticktime ticktime;
typedef struct ticktime * ticktime_p;
typedef struct datetime datetime;
typedef struct datetime * datetime_p;


#ifdef __cplusplus
extern "C" {
#endif
	IMPORT vm_object_p vm_load_bool(void * ctx, uchar value);
	IMPORT vm_object_p vm_create_object(void * ctx, uint32 length, uchar * bytes);
	IMPORT vm_object_p vm_create_immutable_object(void * ctx, int length, void * bytes);
	IMPORT vm_object_p vm_clone_object(void * ctx, vm_object_p obj);
	IMPORT vm_object_p vm_create_interface(void * ctx, pk_class_p base, void * ptr, void(*release)(void*, void*));
	IMPORT vm_object_p vm_create_interface_enumerator(void * ctx, pk_class_p base, void * ptr, pk_class_p elem_base, int index, size_t elem_size, int count, void (* release)(void *, void *));
	IMPORT vm_object_p vm_create_reference(void * ctx, void * ptr);
	IMPORT vm_object_p vm_create_operand(void * ctx, uint16 type, void * ptr, uint32 size);

	IMPORT void * vm_function_get_instance(void * func);
	IMPORT void vm_exec_function(void * ctx, void * func);
	IMPORT void * vm_load_callback(void * ctx, void * func);
	IMPORT void vm_release_callback(void * ctx, void * func);
	IMPORT void vm_push_argument(void * ctx, int size, void * buffer);
	//IMPORT uint8 vm_object_get_type(vm_object_p obj);
	//IMPORT uint16 vm_object_get_text(void * ctx, vm_object_p obj, uint8 * text);
	//IMPORT uint16 vm_object_get_length(void * ctx, vm_object_p obj);
	IMPORT void vm_release_object(void * ctx, vm_object_p obj);
	IMPORT int vm_array_get_count(void * ctx, void * content);
	IMPORT void vm_exec_callback(void * ctx, void * func, int num_args, ...);

	IMPORT void * vm_install_callback(void * ctx, void * func);
	IMPORT char * vm_get_callback_name(void * ctx, void * callback) ;

	IMPORT pk_class_p pk_create_class(uchar * name);
	IMPORT pk_method_p pk_register_method(pk_class_p parent, uchar * name, uchar numargs, uint8 call_type, uint8 ret_type, uint16 offset, void(*callback)() );
	IMPORT void pk_set_arg(pk_method_p method, uint8 id, uint16 type);
	IMPORT void pk_set_callback(pk_method_p method, void(*callback)());

	IMPORT pk_object_p vm_register_interface(void * ctx, const char * name, void * base);
	IMPORT pk_object_p vm_select_interface(void * ctx, const char * name);
	IMPORT pk_object_p vm_interface_add_method(void * ctx, pk_object_p parent, const char * name, uint8 ret_type, uint8 num_args, void(*callback)(), ...);

	IMPORT char * vm_object_to_jsonstring(char * content) ;
	IMPORT void vm_jsonstring_release(char * json);

	//DAL APIs (database abstraction layer)
	//create database connection
	IMPORT dal_conn * dal_create_conn(char * conn_str);
	//open connection
	IMPORT uint8 dal_open(dal_conn * conn);
	//close connection
	IMPORT void dal_close(dal_conn * conn);
	//query statement
	IMPORT dal_result * dal_query(dal_conn * conn, char * statement);
	//prepare statement
	IMPORT dal_prepared_statement * dal_prepare(dal_conn * conn, char * name, char * statement);
	//execute statement
	IMPORT dal_result * dal_execute(dal_conn * conn, dal_prepared_statement * statement, int count, char * args[]);
	//fetch all result
	IMPORT dal_array * dal_fetch_all(dal_result *);
	//get result count
	IMPORT int dal_num_rows(dal_result *);
	//get first result
	IMPORT dal_array * dal_fetch_first(dal_result *);
	//get next result
	IMPORT dal_array * dal_fetch_next(dal_result *);
	//get result at/key
	IMPORT dal_data * dal_row_at(dal_array *, int index);
	IMPORT dal_data * dal_row_key(dal_array *, char * name);
	IMPORT void dal_dump_result(dal_result *);

	IMPORT char * vm_get_request_url(void * ctx) ;

	//misc APIs
	IMPORT uint32 util_iso8601_to_filetime(struct datetime * dtime);
	IMPORT uint8 util_iso8601_from_filetime(struct datetime * dtime, long filetime) ;
	IMPORT uint32 util_iso8601_decode(char * str);
	IMPORT void util_iso8601_encode(struct datetime * dtime, uint8 * buffer);
#ifdef __cplusplus
}
#endif

#endif
