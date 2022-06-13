#include "../defs.h"
#include "../config.h"
#include "../Stack/pkg_encoder.h"
//#include <sstream>
#include "midgard.h"
#include "vm_stream.h"
#ifndef _VM_STACK__H

#define MAX_BUFFER_SIZE				65536


#define INS_NOP			0	//-> no operation 		(pseudo instruction)
#define INS_SCTX		8	//-> save context for variables (function call)
#define INS_RCTX		9	//-> restore context memory (function return)

#define INS_LOAD		10
#define INS_STORE		11
#define INS_CAST		12

#define INS_OBJCONST	16	//-> load constant to stack
#define INS_OBJNEW		17	//-> allocate new instance
#define INS_OBJDUP		18	//-> duplicate stack
#define INS_OBJDEL		19	//-> delete (relative to sp, remove current object from stack)
#define INS_OBJSZ		22	//-> size of object on stack
#define INS_OBJSUB		23	//-> explode object (relative to sp)

#define INS_OBJPUSH		25	//-> push variable to stack		
#define INS_OBJPOP		26	//-> pop variable from stack
#define INS_OBJSTORE		29	//-> store variable from stack

//stack operation
#define INS_ADD		32	//-> addition (relative to sp)
#define INS_SUB		33	//-> subtract (relative to sp)
#define INS_MUL		34	//-> multiplication
#define INS_DIV		35	//-> division
#define INS_MOD		36	//-> modulus
#define INS_AND		40	//-> and operation
#define INS_OR		41	//-> or operation
#define INS_XOR		42	//-> xor operation
#define INS_NOT		43	//-> not operation
#define INS_SHL		48	//-> shift left operation
#define INS_SHR		49	//-> shift right operation

#define INS_END		60	//-> end function			(pseudo instruction)
#define INS_FUNC	61	//-> create function		(pseudo instruction)
#define INS_LBL		62	//-> create label			(pseudo instruction)
#define INS_CREQ		64	//-> jump if equal (relative to pc)
#define INS_CRNE			65	//-> jump not equal (relative to pc)
#define INS_CRGT			66	//-> jump greater than (relative to pc)
#define INS_CRLT			67	//-> jump less than (relative to pc)
#define INS_CRGTEQ			68	//-> jump greater than (relative to pc)
#define INS_CRLTEQ			69	//-> jump less than (relative to pc)
#define INS_JMP				70	//-> jump unconditional (relative to pc)
#define INS_JFALSE			71	//-> jump if false
#define INS_JTRUE			72	//-> jump if true
#define INS_SWITCH			74	//-> switch

#define INS_RET		126	//-> finished operation
#define INS_CALL		127
#define INS_SYSCALL		128
#define INS_SYSCALL0	128
#define INS_SYSCALL1	129
#define INS_SYSCALL2	130
#define INS_SYSCALL3	131
#define INS_SYSCALL4	132
#define INS_SYSCALL5	133
#define INS_SYSCALL6	134
#define INS_SYSCALL7	135
#define INS_SYSCALL8	136
#define INS_SYSCALL9	137
#define INS_SYSCALL10	138
#define INS_SYSCALL11	139
#define INS_SYSCALL12	140
#define INS_SYSCALL13	141
#define INS_SYSCALL14	142
#define INS_SYSCALL15	143

#define INS_EXTCALL		160
#define INS_EXTCALL0	160
#define INS_EXTCALL1	161
#define INS_EXTCALL2	162
#define INS_EXTCALL3	163
#define INS_EXTCALL4	164
#define INS_EXTCALL5	165
#define INS_EXTCALL6	166
#define INS_EXTCALL7	167
#define INS_EXTCALL8	168
#define INS_EXTCALL9	169
#define INS_EXTCALL10	170
#define INS_EXTCALL11	171
#define INS_EXTCALL12	172
#define INS_EXTCALL13	173
#define INS_EXTCALL14	174
#define INS_EXTCALL15	175

#define INS_F2O			96	//-> offset to object
#define INS_F2O_0		96	//-> offset to object
#define INS_F2O_1		97	//-> offset to object
#define INS_F2O_2		98	//-> offset to object
#define INS_F2O_3		99	//-> offset to object
#define INS_F2O_4		100	//-> offset to object
#define INS_F2O_5		101	//-> offset to object
#define INS_F2O_6		102	//-> offset to object
#define INS_F2O_7		103	//-> offset to object
#define INS_F2O_8		104	//-> offset to object
#define INS_F2O_9		105	//-> offset to object
#define INS_F2O_10		106	//-> offset to object
#define INS_F2O_11		107	//-> offset to object
#define INS_F2O_12		108	//-> offset to object
#define INS_F2O_13		109	//-> offset to object
#define INS_F2O_14		110	//-> offset to object
#define INS_F2O_15		111	//-> offset to object
//.db	-> data byte
//.dw	-> data word


//virtual machine exception						 
#define VX_SYSTEM_EXCEPTION				0 
#define VX_UNIMPLEMENTED_APIS			1
#define VX_UNKNOWN_INSTRUCTION			2
#define VX_STACK_OVERFLOW				3 
#define VX_INSUFFICIENT_HEAP	 		4
#define VX_STACK_UNDERFLOW				5
#define VX_OUT_OF_BOUNDS				6
#define VX_UNRESOLVED_CLASS				7
#define VX_UNRESOLVED_METHOD			8
#define VX_ARGUMENT_MISMATCH			9
#define VX_INVALID_CONTEXT				10
#define VX_DIV_BY_ZERO					11
#define VX_UNSUPPORTED_ARGUMENT			12
#define VX_UNSUPPORTED_TYPE				13

#define VM_STATE_INIT		0x00
#define VM_STATE_RUN 		0x01
#define VM_STATE_SUSPEND	0x03
#define VM_STATE_EXCEPTION	0x04
#define VM_STATE_ABORT		0x07

#define VM_MUTABLE_OBJECT	0x80
#define VM_IMMUTABLE_OBJECT	0x00

#define VM_MAGIC_MASK		0xF0
#define VM_MAGIC			0x80		//string/auto type
#define VM_OPR_MAGIC		0xA0		//strong type variable
#define VM_EXT_MAGIC		0xD0		//custom operand
#define VM_OBJ_MAGIC		0xE0		//object type (array/object)
#define VM_MAGIC_MASK_ZERO	(VM_MAGIC | 0x0F)

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) \
 || defined(__OpenBSD__) || defined(__APPLE__)  || defined(__sun) || defined(__hpux) \
 || defined(__osf__) || defined(__unix__) || defined(LINUX) || defined(_AIX)
#define VM_MAX_VAR_NAME				64
#else
#define VM_MAX_VAR_NAME				16
#endif
#define VA_OBJECT_MAX_SIZE			0x1000

#define VM_OBJ_TYPE_ARR			0x0040
#define VM_OBJ_TYPE_REF			0x0080
#define VM_OBJ_TYPE_UNSIGNED	0x0020
#define VM_OBJ_TYPE_PRECISION	0x0010
#define VM_OBJ_TYPE_ANY			0x0000
#define VM_OBJ_TYPE_INSTANCE	0x0800
#define VM_OBJ_TYPE_INTERFACE	0x0400

#define VM_INT8					sizeof(int8)
#define VM_INT16				sizeof(int16)
#define VM_INT32				sizeof(int32)
#define VM_INT64				sizeof(int64)
#define VM_FLOAT				(VM_OBJ_TYPE_PRECISION | sizeof(float))
#define VM_DOUBLE				(VM_OBJ_TYPE_PRECISION | sizeof(double))
#define VM_STRING				(VM_OBJ_TYPE_ARR | sizeof(int8))
#define VM_POINTER				sizeof(void *)	
#define VM_ARRAY				VM_OBJ_TYPE_ARR
#define VM_AUTO_POINTER			(VM_OBJ_TYPE_REF | sizeof(void *))
#define VM_VAR					VM_OBJ_TYPE_ANY

#define CALL_TYPE_C		0
#define CALL_TYPE_STD		1
#define CALL_TYPE_FAST		2

typedef struct vm_object vm_object;
typedef struct sys_context sys_context;
typedef struct vm_variable vm_variable;
typedef struct vm_context vm_context;
typedef struct vm_function vm_function;
typedef struct vm_instance vm_instance;
typedef struct vm_tcblock vm_tcblock;
typedef struct vm_interface vm_interface;
typedef struct vm_interface_enumerator vm_interface_enumerator;
typedef struct vm_api_entry vm_api_entry;
typedef struct vm_extension vm_extension;
typedef struct vm_custom_opcode vm_custom_opcode;

typedef struct vm_object {
	uint8 mgc_refcount;		//4(high) magic, 4(low) refcounter
	uint8 lock;				//lock counter (amount of object referencing to this object)
	uint16 type;
	uint32 len;
	struct vm_object * next;		//for immutable object
	void (* release)(void * ctx, void * ptr);
	uint8 bytes[1];
} vm_object;

typedef struct sys_context {
	uchar num_of_params;
} sys_context;

typedef struct vm_variable {
	uint8 mgc;
	struct vm_variable * next;
	uint8 name[VM_MAX_VAR_NAME];
	uint16 len;
	uint8 bytes[1];
} vm_variable;

typedef struct vm_context {
	vm_variable ** vars;
	uint16 offset;
	vm_variable * var_root;
} vm_context;

typedef struct vm_function {
	void * base;
	vm_instance * instance;
	uint8 arg_count;
	uint16 offset;
} vm_function;

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) \
 || defined(__OpenBSD__) || defined(__APPLE__)  || defined(__sun) || defined(__hpux) \
 || defined(__osf__) || defined(__unix__) || defined(LINUX) || defined(_AIX)
#define VM_MAX_STACK_SIZE		2000
#else
#define VM_MAX_STACK_SIZE		64
#endif

typedef struct vm_tcblock {
	uint32 _start;
	uint32 _end;
	uint32 _catch;
	uint8 num_args;
	vm_instance * snapshot;
	struct vm_tcblock * next;
} vm_tcblock;

typedef struct vm_instance {
	vm_context base;
	uint16 sp;		//stack pointer
	uint16 bp;		//base pointer
	uint16 pc;
	pk_object * info;			//package root entries
	vm_tcblock * tcs;			//try-catch block entries
	void * base_address;
	heap_manager * heap;
	uchar vm_state;
	uchar recursive_exec;		//set recursive_exec flag to false
	uint16 current_api;
	sys_context sysc;
	vm_object * ret;
	stream_file * stream;
	uint16 exception_code;		//current exception code
	vm_object * object_list;	//list of immutable objects
	void * data;				//3rd party data
	void * vm_stacks[VM_MAX_STACK_SIZE];
} vm_instance;

typedef struct vm_interface {
	void * ptr;			//actual data
	pk_class * base;	//base interface
} vm_interface;

typedef struct vm_interface_enumerator {
	vm_interface base;
	pk_class * elem_base;
	int index;
	size_t elem_size;
	int count;
} vm_interface_enumerator;

#define VM_DEF_ARG			vm_instance * ctx
#define VM_ARG				ctx

typedef struct vm_api_entry {
	uint16 id;
	char * name;
 	void (* entry)(VM_DEF_ARG); 
 	vm_object * (* exit)(uint8, uint8 *);
} vm_api_entry;

typedef struct vm_custom_opcode {
	uint16 (* text)(VM_DEF_ARG, vm_object *, uint8 * buffer);
	vm_object * (* _add)(VM_DEF_ARG, vm_object *, vm_object *);
	vm_object * (* _mul)(VM_DEF_ARG, vm_object *, vm_object *);
	vm_object * (* _div)(VM_DEF_ARG, vm_object *, vm_object *);
	vm_object * (* _sub)(VM_DEF_ARG, vm_object *, vm_object *);
	vm_object * (* _mod)(VM_DEF_ARG, vm_object *, vm_object *);
	//logical operation
	vm_object * (* _and)(VM_DEF_ARG, vm_object *, vm_object *);
	vm_object * (* _or)(VM_DEF_ARG, vm_object *, vm_object *);
	vm_object * (* _xor)(VM_DEF_ARG, vm_object *, vm_object *);
	vm_object * (* _shl)(VM_DEF_ARG, vm_object *, vm_object *);
	vm_object * (* _shr)(VM_DEF_ARG, vm_object *, vm_object *);
	vm_object * (* _not)(VM_DEF_ARG, vm_object *);
} vm_custom_opcode;

typedef struct vm_extension {
	uint8 tag;
	vm_custom_opcode * apis;
	uint8 payload[1];
} vm_extension;

typedef sys_context * sys_handle;

#define vm_push_stack(x)	ctx->vm_stacks[ctx->sp--] = x
#define vm_pop_stack()		ctx->vm_stacks[++ctx->sp]
#define vm_push_base(x) 	ctx->vm_stacks[ctx->bp++] = x
#define vm_set_stack(x)		ctx->vm_stacks[ctx->sp] = x
#define vm_get_stack(x)		ctx->vm_stacks[ctx->sp]
#define vm_push_base(x)		ctx->vm_stacks[ctx->bp++] = x
#define vm_pop_base()		ctx->vm_stacks[--ctx->bp]
#define vm_get_base(i)		ctx->vm_stacks[ctx->bp - i]
#define vm_set_base(i,x)	ctx->vm_stacks[ctx->bp - i] = x
#define vm_stack(x)			ctx->vm_stacks[x]
#define vm_get_sp()			ctx->sp
#define vm_set_sp(x)		ctx->sp = x
#define vm_get_pc()			ctx->pc
#define vm_set_pc(x)		ctx->pc = x
#define vm_add_pc(x)		ctx->pc += x
#define vm_get_bp()			ctx->bp
#define vm_set_bp(x)		ctx->bp = x
#define vm_get_package()	ctx->base_address
#define vm_set_package(x)	ctx->base_address = x
#define vm_get_state()		ctx->vm_state
#define vm_get_sysc()		ctx->sysc
#define vm_get_rexec()		ctx->recursive_exec
#define vm_set_rexec(x)		ctx->recursive_exec = x
#define vm_get_context()	ctx->base
#define vm_get_cur_api()	ctx->current_api
#define vm_set_cur_api(x)	ctx->current_api = x
#define vm_get_retval()		ctx->ret
#define vm_set_retval(x)	ctx->ret = x
#define vm_get_info()		ctx->info
#define vm_set_info(x)		ctx->info = x
#define vm_get_tcs()		ctx->tcs
#define vm_set_tcs(x)		ctx->tcs = x
#define vm_get_heap()		ctx->heap
#define vm_set_heap(x)		ctx->heap = x
#define vm_get_stream()		ctx->stream
#define vm_set_stream(x)	ctx->stream = x
#define vm_get_exception()	ctx->exception_code
#define vm_set_exception(x) ctx->exception_code = x

#ifdef __cplusplus
extern "C" {
#endif

void vm_memcpy(void * dst, void * src, size_t sz);	
int vm_memcmp(void * dst, void * src, size_t sz);
void vm_memset(void * dst, int set, size_t sz);	
size_t vm_strlen(const char *);
void vm_object_ref(vm_object * obj);
void vm_object_unref(vm_object * obj);
uint8 vm_is_numeric(vm_object * obj);
uint8 vm_is_numeric_s(uint8 * buffer, int len);
uint8 vm_is_precision(uint8 * buffer, int len);

void vm_init_global(void *);			//called once
vm_object * vm_push_stack_arc(VM_DEF_ARG, vm_object * obj);
vm_object * vm_pop_stack_arc(VM_DEF_ARG);
LIB_API vm_object * vm_load_bool(VM_DEF_ARG, uchar value);
vm_object * vm_load_constant(VM_DEF_ARG, uint32 offset);
LIB_API vm_object * vm_create_object(VM_DEF_ARG, uint32 length, void * bytes);
LIB_API vm_object * vm_create_immutable_object(VM_DEF_ARG, uint32 length, void * bytes);
LIB_API vm_object * vm_create_arg(VM_DEF_ARG, uint32 length, void * bytes);
LIB_API vm_object * vm_clone_object(VM_DEF_ARG, vm_object * obj);
LIB_API vm_object * vm_create_interface(VM_DEF_ARG, pk_class * base, uint8 * ptr, void (*release)(void *, void *));
LIB_API vm_object * vm_create_interface_enumerator(VM_DEF_ARG, pk_class * base, uint8 * ptr, void * elem_base, int index, size_t elem_size, int count, void (* release)(void *, void *));
LIB_API vm_object * vm_create_extension(VM_DEF_ARG, uint8 tag, vm_custom_opcode * apis, uint16 length, uchar * bytes);
LIB_API vm_object * vm_create_reference(VM_DEF_ARG, void * ptr);
LIB_API vm_object * vm_create_operand(VM_DEF_ARG, uint16 type, void * ptr, uint32 size);
LIB_API uint8 vm_object_get_type(vm_object * obj);
LIB_API uint16 vm_object_get_text(VM_DEF_ARG, vm_object * obj, uint8 * text);
LIB_API uint16 vm_object_get_length(VM_DEF_ARG, vm_object * obj);
LIB_API uint32 vm_array_get_count(VM_DEF_ARG, uint8 * content);
uint8 vm_ext_get_tag(vm_object * obj);
LIB_API void vm_release_object(VM_DEF_ARG, vm_object * obj);
vm_object * vm_size_object(VM_DEF_ARG, vm_object * op1);
vm_object * vm_split_object(VM_DEF_ARG, vm_object * op2, vm_object * op1, vm_object * target);
vm_object * vm_operation_object(VM_DEF_ARG, uchar opcode, vm_object * op2, vm_object * op1);
uchar vm_cmp_object(VM_DEF_ARG, vm_object * op2, vm_object * op1);
vm_object * vm_compare_object(VM_DEF_ARG, uint8 opcode, vm_object * op2, vm_object * op1);
vm_object * vm_load_class_sta(VM_DEF_ARG, uchar * classname);
vm_object * vm_load_class(VM_DEF_ARG, vm_object * clsobj);
pk_method * vm_load_method(VM_DEF_ARG, vm_object * clsobj, vm_object * mthobj);
extern vm_object * vm_interface_exec_sta(VM_DEF_ARG, uint8 num_args, vm_object * iface, uint8 * method_name);
extern vm_object * vm_interface_exec(VM_DEF_ARG, uint8 num_args, vm_object * iface, vm_object * method);
//context
vm_context * vm_get_current_context();
vm_variable * vm_variable_new(vm_variable ** root, uint8 type, uint16 length, uint8 * bytes);
void vm_variable_release(vm_variable ** root, vm_variable * var);
void vm_variable_clear(vm_variable ** root);

void vm_garbage_collect(VM_DEF_ARG);												//perform garbage collection (ref counting) with copying collector
void vm_update_mutator(VM_DEF_ARG, vm_object * old_addr, vm_object * new_addr);			//replace mutator pre-allocated address with new allocated address
LIB_API void vm_invoke_exception(VM_DEF_ARG, uchar excp) ;

//vm ports apis
extern void vm_init(VM_DEF_ARG, pk_object * root, uint32 heap_size);
uint8 vm_init_exec(VM_DEF_ARG, char * classname, char * method);
LIB_API void * vm_function_get_instance(vm_function * func);
LIB_API void vm_exec_function(VM_DEF_ARG, vm_function * func);
LIB_API void * vm_load_callback(VM_DEF_ARG, vm_function * func);
LIB_API void vm_release_callback(VM_DEF_ARG, vm_function * func);
LIB_API void vm_exec_callback(VM_DEF_ARG, vm_function * func, int num_args, ...);
//vm ext ports APIs
LIB_API char * vm_get_request_url(VM_DEF_ARG);
LIB_API void * vm_install_callback(VM_DEF_ARG, vm_function * func) ;
LIB_API char * vm_get_callback_name(VM_DEF_ARG, void * callback);

LIB_API void vm_push_argument(VM_DEF_ARG, int size, uint8 * buffer);
void vm_push_argument_object(VM_DEF_ARG, vm_object * obj);
extern void vm_decode(VM_DEF_ARG, uint32 offset, uchar num_args, ...);
extern uint32 vm_fetch(VM_DEF_ARG, uint32 offset, uchar * buffer, uint32 size);
extern void vm_close(VM_DEF_ARG);

//syscall apis
LIB_API vm_object * vm_get_argument(VM_DEF_ARG, uchar index);
LIB_API uchar vm_get_argument_count(VM_DEF_ARG);
vm_object * vm_syscall(VM_DEF_ARG, uint16 api_id);				//external hook function
void va_sys_exec(VM_DEF_ARG) _REENTRANT_ ;

LIB_API pk_object * vm_register_interface(VM_DEF_ARG, const char * name, pk_object * base) ;
LIB_API pk_object * vm_select_interface(VM_DEF_ARG, const char * name) ;
LIB_API pk_object * vm_interface_add_method(VM_DEF_ARG, pk_object * parent, const char * name, uint8 ret_type, uint8 num_args, void (* callback)(), ...) ;

//external library APIs
LIB_API int vm_bytes_length(void * ptr);
LIB_API char * vm_arg_jsonstring(char * content);
LIB_API void vm_arg_release(char * json) ;

void vm_push_tcblock(VM_DEF_ARG, pk_tcblock * block);
vm_tcblock * vm_pop_tcblock(VM_DEF_ARG);
void vm_locate_tcblock(VM_DEF_ARG);

#ifdef __cplusplus
}
#endif
#define _VM_STACK__H
#endif
