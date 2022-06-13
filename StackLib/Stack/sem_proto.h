#ifndef _SEM_PROTO__H
#include "../defs.h"
#include "token.h"
#include "sym_table.h"
#include "lex_proto.h"
#include "pkg_encoder.h"

#define SP_ERR_UNREF_FUNCTION		_RECAST(uchar *, "Call to unreferenced function %s")
#define SP_ERR_UNREF_VARIABLE		_RECAST(uchar *, "Access to unreferenced variable %s")
#define SP_ERR_UNREF_LABEL			_RECAST(uchar *, "Jump to unreferenced label %s")
#define SP_ERR_UNREF_SYMBOL			_RECAST(uchar *, "Detected unreferenced symbol %s")
#define SP_ERR_LESS_ARGUMENTS		_RECAST(uchar *, "Less arguments during function call %s")
#define SP_ERR_OVER_ARGUMENTS		_RECAST(uchar *, "Over arguments during function call %s")
#define SP_ERR_UNDEF_FUNCTION		_RECAST(uchar *, "Undefined function %s")
#define SP_ERR_UNDEF_VARIABLE		_RECAST(uchar *, "Undefined variable %s")
#define SP_ERR_UNDEF_LABEL			_RECAST(uchar *, "Undefined label %s")
#define SP_ERR_UNDEF_EXTERNAL		_RECAST(uchar *, "Undefined external %s")
#define SP_ERR_MULTI_FUNCTION		_RECAST(uchar *, "Multiple function declaration %s")
#define SP_ERR_MULTI_VARIABLE		_RECAST(uchar *, "Multiple variable declaration %s")
#define SP_ERR_MULTI_LABEL			_RECAST(uchar *, "Multiple label declaration %s")
#define SP_ERR_MULTI_EXTERNAL		_RECAST(uchar *, "Multiple external declaration %s")
#define SP_ERR_UNDEF_CLASS			_RECAST(uchar *, "Undefined class %s")
#define SP_ERR_UNDEF_METHOD			_RECAST(uchar *, "Undefined method %s")
#define SP_ERR_NO_OVERLOAD_METHOD	_RECAST(uchar *, "No overloaded method %s with %i arguments")
#define SP_ERR_MUST_PUBLIC			_RECAST(uchar *, "Function %s must be marked as public")
#define SP_ERR_MAXIMUM_VARIABLES	_RECAST(uchar *, "Maximum %d variables declaration reached (%s)")
#define SP_ERR_INVALID_LAMBDA_DECL	_RECAST(uchar *, "Invalid lambda declaration %s")		
#define SP_ERR_NESTED_ACCESS		_RECAST(uchar *, "Nested access not supported %s")		
#define SP_ERR_PARSER				_RECAST(uchar *, "Semantic error: %s, %s")

#ifdef STACK_HOST
#define sm_printf			printf
#else
#define sm_printf
#endif

#define sp_push_varctx_scope() sp_push_varctx_scope_s(__LINE__)
#define sp_pop_varctx_scope() sp_pop_varctx_scope_s(__LINE__)

#define SEM_MAX_STACK		256

typedef struct err_record {
	struct err_record * next;
	uchar buffer[256];
} err_record;

typedef struct ins_node {
	uchar size;
	uchar ins;
	symrec * rec;
	struct ins_node * next;
	symrec * scope_var;
} ins_node;

typedef struct lambda_context {
	uint32 context_cntr;
	symrec * func;
	symrec * exit_func;
} lambda_context;

typedef struct var_context {
	uint8 operation;
	symrec * var;
	ins_node * inode;
} var_context;

typedef struct scope_operation {
	ins_node * op_stack[SEM_MAX_STACK];
	uint8 op_index;
	var_context * varctx_stack[SEM_MAX_STACK];
	uint8 varctx_index;
} scope_operation;

typedef struct try_catch_block {
	int start;
	int end;
	int cblock;
	uint8 num_args;
} try_catch_block;

#ifdef __cplusplus
extern "C" {
#endif
void sp_error_clear();
err_record * sp_error_get_enumerator();
err_record * sp_error_next_record(err_record *rec);

//expression scoping for lazy expr
ins_node * sp_create_inode(uchar ins, symrec * rec);
ins_node * sp_push_iroot(ins_node * inode);
ins_node * sp_push_inode(ins_node * inode);
ins_node * sp_pop_inode();
ins_node * sp_get_inode();
symrec * sp_set_scope_var(symrec * rec);
symrec * sp_get_scope_var();
void sp_push_varctx_scope_s(int line);
void sp_pop_varctx_scope_s(int line);
void sp_create_new_scope(symrec * var);					//create new scope, 2 scope (current scope + after scope)
void sp_clone_scope();						//duplicate current scope
void sp_flush_scope_s(ins_node * node) ;
void sp_flush_scope();						//flush scope but do not destroy both (current scope + after scope)
void sp_flush_scope_now_s(ins_node * root);
void sp_flush_scope_now() ;
void sp_create_method_scope() ;
void sp_destroy_method_scope() ;
void sp_flush_method_param();
void sp_destroy_scope();					//flush and destroy scope (current scope + after scope), write to il_streamer
void sp_release_scope();					//only destroy scope, no flushing
void sp_merge_scope_left() ;						//merge both scope with both prev scope, prev_current = current + prev_current, prev_after = prev_after + after
void sp_merge_scope_right();				//merge both scope with both prev scope, prev_current = prev_current + current, prev_after = prev_after + after
void sp_swap_scope();						//swap after and current, current <-> after scope, use twice to return order
void sp_clear_current_scope();				//clear current scope, current -> { }
void sp_end_scope();
		
symrec * sp_push_symrec(symrec * rec);
symrec * sp_pop_symrec(void);
symrec * sp_enqueue_symrec(symrec * rec);
symrec * sp_dequeue_symrec(void);
symrec * sp_peek_symrec(void);
symrec * sp_look_symrec(void);
void sp_clear_symrec();
symrec * sp_declare_function(uchar * funcname);
symrec * sp_declare_interface_function(uchar * funcname) ;
void sp_declare_function_param(uint8 type);
void sp_declare_function_extern(uchar * apinum);
symrec * sp_declare_function_end(uchar * funcname);
void sp_install_api(uchar * funcname, uint16 api_id, uint8 num_args);
void sp_push_operation(uint16 api_id, uint8 num_args);
void sp_push_operation_stack(ins_node * inode);
void sp_clear_operation_stack();
void sp_end_expr(symrec * rec);

symrec * sp_create_label(uchar * prefix) ;
symrec * sp_create_variable(uchar * varname);
symrec * sp_create_param(uchar * varname);
symrec * sp_create_function(uchar * funcname) ;
symrec * sp_start_function_body(symrec * func, uchar is_public);
uint32 sp_return_function_body(void); 
uint32 sp_exit_function_body(void);
uint32 sp_end_function_body(symrec * func);
symrec * sp_start_api_call(uint16 api_id, uchar icount) ;
symrec * sp_start_function_call(uchar * funcname, uchar icount);
uint32 sp_push_function_param(symrec * funcrec);
symrec * sp_end_function_call(symrec * func);

uint32 sp_jump_to(uchar ins, uchar * label) ;
uint32 sp_new_label(uchar * label);
uint32 sp_load_stack(uchar * varname);
uint32 sp_load_constant(uchar * constname);
uint32 sp_load_numeric(uchar * constname);
uint32 sp_load_array(uchar * arrval, uint8 length);
uint32 sp_store_stack(uchar * varname);
uint32 sp_call_function(uchar * funcname);
uint32 sp_operation_stack(uchar ins);
symrec * sp_start_loop(void);
symrec * sp_start_each_loop(symrec * rec);
uint32 sp_continue_loop(void);
uint32 sp_end_loop(void);
uint32 sp_end_each_loop(void);
uint32 sp_push_constant(uchar * symname);
uint32 sp_push_constant_s(uchar * symname, uint16 length) ;
uint32 sp_push_array(uchar * arrval, uint16 length);

//switch-case statement
symrec * sp_start_case();
uint32 sp_label_case_s(symrec * jumptable, uchar * name, uint16 length);
uint32 sp_label_case(symrec * jumptable, uchar * name);
uint32 sp_default_case(symrec * jumptable);
uint32 sp_end_case(symrec * jumptable);

void sp_new_array_constant(void);
void sp_push_array_constant(uchar * constname);
symrec * sp_lz_load_array_constant(void);

//constant construct
void sp_start_construct();
void sp_push_construct(uchar * value);
uchar * sp_end_construct();

symrec * sp_lhs_load_variable(uchar * varname);
symrec * sp_lhs_store_variable(uchar * varname);
void sp_lhs_push_api_call(int api_id);
void sp_lhs_api_call(int api_id, uint8 arg_count);
void sp_lhs_get_s(uint16 api_id, uint8 index);
void sp_lhs_get(uint16 api_id);
void sp_rhs_get_s(uint8 index);
void sp_rhs_get(uint16 api_id);
void sp_lhs_set(uint16 api_id);
void sp_lhs_lazy_get(uint16 api_id);
void sp_lhs_store(uint8 ins, symrec * rec);
symrec * sp_lhs_load(uint16 api_id, uint8 num_args, symrec * rec);
void sp_lhs_clear();

symrec * sp_lz_load_stack(symrec * rec);
symrec * sp_lz_load_constant_s(uchar * constname, uint16 length);
symrec * sp_lz_load_constant(uchar * constname);
symrec * sp_lz_load_numeric(uchar * constname);
symrec * sp_lz_load_array(uchar * arrval, uint16 length);
symrec * sp_lz_create_instance(uchar * constname);
symrec * sp_lz_load_variable(uchar * varname);
symrec * sp_lz_store_variable(uchar * varname);
symrec * sp_lz_load_method(uchar * funcname);
symrec * sp_lz_variable_after_scope(uchar * dstname, uchar * srcname, uchar ins);
symrec * sp_lz_constant_after_scope(uchar * dstname, uchar * constname, uchar ins);
symrec * sp_lz_array_after_scope(uchar * dstname, uchar * arrval, uint8 length, uchar ins);
symrec * sp_lz_load_lambda(symrec * lambda) ;

//lambda expression (2018.01.08)
lambda_context * sp_push_lambda();			//push new lambda function context
lambda_context * sp_pop_lambda();			//pop previous function
symrec * sp_start_lambda_decl(uchar icount);
symrec * sp_create_lambda_param(uchar * varname);
symrec * sp_execute_lambda_call(symrec * rec);
uint32 sp_skip_lambda_body(void);
symrec * sp_start_lambda_body(symrec * rec);
symrec * sp_end_lambda_decl(symrec * func);

//3rd gen support
void sp_import_package(uchar * pkgname);
symrec * sp_lz_load_class(uchar * classname);
symrec * sp_start_method_call(uchar * methname);
uint32 sp_push_method_param(symrec * rec);
symrec * sp_end_method_call(symrec * methrec);
symrec * sp_end_method_call_scope(symrec * methrec);

pk_class * sp_start_class_body(uchar * name);
pk_method * sp_register_method(symrec * rec, uint8 call_type, uint8 ret_type);
void sp_end_class_body(void);

void sp_push_try_catch();
void sp_set_cblock();
void sp_pop_try_catch(int num_args);

symrec * sp_install_menu(uchar * menuname, symrec * methrec);
symrec * sp_install_event(uchar * eventname, symrec * methrec);

void sp_init_parser(void);
pp_config * sp_init(uchar * filename);
pp_config * sp_open(uchar * filename);
uint32 sp_parse(void);
void sp_cleanup_parser(void);
pp_config * sp_clr_init(uchar * codebuffer, int32 size);

void sp_error(uchar * msg, ...);

#ifdef __cplusplus
}
#endif
#define _SEM_PROTO__H
#endif
