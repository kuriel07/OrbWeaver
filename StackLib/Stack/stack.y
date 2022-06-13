%{
int errors = 0;
int yyerror ( char *s );
/*************************************************************************
Compiler for the Stack language
***************************************************************************/
/*=========================================================================
C Libraries, Symbol Table, Code Generator & other C code
=========================================================================*/
#include <stdio.h> /* For I/O */
#include <stdlib.h> /* For malloc here and in symbol table */
#include <string.h> /* For strcmp in symbol table */
#include <stdarg.h>
#include "token.h"
#include "sem_proto.h"
#include "il_streamer.h"
#include "asm_streamer.h"
#include "lex_proto.h"
#include "scr_generator.h"

#define YYDEBUG 1 /* For Debugging */
#define alloca malloc

%}

%union semrec /* The Semantic Records */
{
	uchar string[4096];
	struct {
		uint16 length;
		uchar value[4096];
	} bytes;
	struct {
		uint16 type;
		uchar string[4096];
	} object;
	uint32 state;
	uint16 api_id;
	uchar cast_string[10];
	yytoken token;
}

/*=========================================================================
TOKENS
=========================================================================*/
%start program
%token VARIABLE		257		//foo, a, index, label		--> variable
%token CONSTANT		258		//"asda", {0,0,0}			--> constant value
%token N_CONSTANT	259
%token ARRAY		260		//bytes array
%token NUMERIC		261		//float number
%token N_NUMERIC	262		//negative number

%token P_VOID		290
%token P_AUTO		291
%token P_BYTE		292
%token P_HWORD		293
%token P_WORD		294
%token P_DWORD		295
%token P_FLOAT		296
%token P_DOUBLE		297
%token P_STRING		298
%token P_PTR		299

%token P_VAR	300		//string
%token P_NEW	301		//new
%token P_HEX	302		//array
%token P_SIZEOF	310		//sizeof

%token P_STDCALL	314
%token P_FASTCALL	315	

%token P_TRY		345
%token P_CATCH		346
%token P_IN			347
%token P_FOREACH	348
%token P_GOTO		349	//goto
%token P_IF		350		//if
%token P_ELSE		351		//else
%token P_SWITCH	352		//switch
%token P_CASE		353		//case 
%token P_DEFAULT	354
%token P_BREAK		355		//break
%token P_FOR		356
%token P_WHILE		357		//while
%token P_DO		358		//while
%token P_CONTINUE	359


%token P_FUNCTION	360		//function
%token P_RETURN		361		//return
%token P_ALIAS		362		//alias
%token P_EVENT		363		//event
%token P_PROTO		364		//proto
%token P_EXTERN		365		//extern
%token P_SYSCALL	366		//syscall

//third gen capability
%token P_CLASS		380		//class
%token P_INTERFACE	381		//interface
%token P_PACKAGE	382
%token P_IMPORT		383
%token P_PUBLIC		385		//public
%token P_PRIVATE	386		//private
%token P_PROTECTED	387		//protected
%token P_LIB		388		//lib

%token L_SB		398		//left square bracket
%token R_SB		399		//right square bracket
%token L_CL		400		//left curl
%token R_CL		401		//right curl
%token EOS			402		//semicolon (end of statement)
%token EOC			403		//colon (end of label)
%token L_BR		406		//left bracket
%token R_BR		407		//right bracket
%token T_EQ		410		//type equal
%token T_GT		411		//type greater than
%token T_LT		412		//type less than
%token T_GTEQ		413		//type greater than
%token T_LTEQ		414		//type less than
%token T_NE		415		//type not equal
%token T_AND		416		//type and
%token T_OR		417		//type or


%token S_EQ			450		//assignment
%token S_ADD		451		//+
%token S_SUB		452		//-
%token S_ADDEQ		453		//+=
%token S_SUBEQ		454		//-=
%token S_ADDADD		455		//++
%token S_SUBSUB		456		//--
%token S_MUL		457		//multiplication(*)
%token S_DIV		458		//division(/)
%token S_MULEQ		459		//lazy multiplication
%token S_DIVEQ		460		//lazy division
%token S_MOD		461		//modulo
%token S_MODEQ		462		//lazy modulation
%token S_RNEXT		470		//arrow right
%token S_PDOT		471		//dot
%token S_AND		472
%token S_OR			473
%token S_XOR		474
%token S_NOT		475
%token S_ANDEQ		476
%token S_OREQ		477
%token S_XOREQ		478
%token S_SHL		480
%token S_SHR		481
%token S_SHLEQ		482
%token S_SHREQ		483

%type<string> VARIABLE
%type<string> var_init_decl
%type<bytes> CONSTANT
%type<bytes> N_CONSTANT
%type<bytes> NUMERIC
%type<bytes> N_NUMERIC
%type<token> ARRAY
//%type<bytes> constant_expr
%type<api_id> instance_val
%type<string> instance_decl
%type<bytes> constant_val
%type<bytes> numeric_val
%type<state> accessor_func
%type<api_id>	lhs_val
%type<api_id>   lhs_func
%type<api_id>   rhs_val
%type<string> cast_val
%type<api_id> var_type_primitive
%type<api_id> var_type_param
%type<api_id> var_type
%type<api_id> call_type
//%type<api_id> cast_val

%right S_EQ S_ADDEQ S_SUBEQ S_MULEQ S_DIVEQ S_MODEQ S_ANDEQ S_OREQ S_XOREQ S_SHLEQ S_SHREQ
%left S_SUB S_ADD
%left S_MUL S_DIV S_MOD
%left S_AND S_OR S_XOR
%left S_SHL S_SHR 
%left S_NOT
/*=========================================================================
GRAMMAR RULES for the Simple language
=========================================================================*/
%%	
/*********************************************** SEMANTIC RULES SHALL BE WRITTEN HERE (LANGUAGE DEPENDENT) ***********************************************/
program: /* empty */ 
| global_decl program 
| global_decl 
;
stmts: /* empty */
| block_stmt stmts
| block_stmt
;
multi_block_stmt: /* empty */
| matched_if_stmt
| open_if_stmt
| try_catch_stmt
;
single_block_stmt: /* empty */
| local_decl EOS { sp_flush_scope_now(); }
| L_CL { sp_create_new_scope(NULL); sp_push_varctx_scope(); } stmts R_CL { sp_pop_varctx_scope(); sp_flush_scope_now(); sp_destroy_scope(); } /* a block consist of several statements */
| while_stmt
| for_stmt
| case_stmt
| stmt_end 												/* a single statement can be considered a single block as long end with semicolon */
| label_decl										
| error_stmt EOS
| EOS { sp_flush_scope_now(); } 
;
block_stmt: /* empty */
| single_block_stmt
| multi_block_stmt
;
global_decl: /* empty */
| P_IMPORT { } global_import_path EOS { } 
| P_CLASS VARIABLE { sp_start_class_body($2); } L_CL class_stmts R_CL { sp_end_class_body(); }
| P_INTERFACE VARIABLE P_LIB CONSTANT { sp_start_interface_body($2, $4.value); } L_CL interface_stmts R_CL { sp_end_interface_body(); }
| P_EXTERN P_FUNCTION sys_func_decl EOS
| error_stmt 
;
interface_stmts: interface_stmt interface_stmts
| interface_stmt
;
class_stmts: class_stmt class_stmts
| class_stmt
;
case_block_stmt: /* empty */
| case_block_stmt case_single_stmt
| case_single_stmt
;
inline_stmt: inline_stmt ',' stmt
| stmt
;
inline_var_decl: inline_var_decl ',' VARIABLE { sp_create_variable($3); }
| inline_var_decl ',' VARIABLE S_EQ { sp_create_new_scope(NULL); } lazy_expr { sp_pop_symrec(); symrec * rec = sp_create_variable($3); sp_flush_scope(); sp_destroy_scope(); sp_store_stack(rec->name); sp_clear_operation_stack(); sp_lhs_clear(); } 	/* variable declaration with assignment */ 
| var_decl { }
;
case_expr_param: L_BR exprval R_BR
;
expr_params: L_BR exprs R_BR { sp_clear_operation_stack(); sp_lhs_clear(); }
;
body_func_params: L_BR body_func_param_list R_BR								/* params used during function declaration */
;
stmt_end: stmt EOS { sp_flush_scope_now(); sp_clear_operation_stack(); sp_lhs_clear(); }
;
lazy_stmts:
| lazy_stmt ',' lazy_stmts
| lazy_stmt
;
local_decl: inline_var_decl { sp_clear_operation_stack(); sp_lhs_clear(); }
;
/*********************************************** SEMANTIC ACTION SHALL BE WRITTEN HERE (MACHINE DEPENDENT) ***********************************************/
global_import_path: VARIABLE { sp_push_construct($1); } '.' { sp_push_construct(_RECAST(uchar *, "\\")); } global_import_path
| VARIABLE { sp_push_construct($1); } 
| '*'
;
var_init_decl: P_VAR VARIABLE { strcpy(_RECAST(char *, $$), _RECAST(const char *, $2)); /*sp_push_symrec(sp_create_variable($2));*/ }	
;
var_decl: var_init_decl S_EQ { sp_create_new_scope(NULL); } lazy_expr { sp_pop_symrec(); symrec * rec = sp_create_variable($1); sp_flush_scope_now(); sp_destroy_scope(); sp_store_stack(rec->name); sp_clear_operation_stack(); sp_lhs_clear(); } 	/* variable declaration with assignment */
| var_init_decl	{ sp_create_variable($1); }		/* variable declaration without assignment */				
;
label_decl: VARIABLE EOC { sp_new_label($1); }
;
matched_if_stmt: 
| P_IF { sp_push_symrec(sp_create_label(_RECAST(uchar *, "__b"))); sp_push_symrec(sp_create_label(_RECAST(uchar *, "__i"))); } expr_params { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); } matched_if_stmt  
	else_stmt
	{ symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
| single_block_stmt
;
else_stmt: /* empty */
| P_ELSE 
	{ symrec * rec = sp_pop_symrec(); symrec * rel = sp_create_label(_RECAST(uchar *, "__l")); sp_push_symrec(rel); sp_jump_to(INS_JMP, rel->name); sp_new_label(rec->name); } 
	matched_if_stmt
;
open_if_stmt: 
| P_IF { sp_push_symrec(sp_create_label(_RECAST(uchar *, "__b"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); } expr_params { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); } multi_block_stmt { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
| P_IF { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__b"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); } expr_params { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); } matched_if_stmt 
	P_ELSE 
	{ symrec * rec = sp_pop_symrec(); symrec * rel = sp_create_label(_RECAST(uchar *,"__l")); sp_push_symrec(rel); sp_jump_to(INS_JMP, rel->name); sp_new_label(rec->name); } 
	open_if_stmt
	{ symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
;
while_stmt: P_WHILE { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__h"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__w"))); sp_start_loop(); } expr_params { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); } block_stmt 
	{ symrec * rec = sp_pop_symrec(); sp_end_loop(); sp_new_label(rec->name); }
| P_DO { sp_push_symrec(sp_start_loop()); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__w"))); } block_stmt 
	P_WHILE expr_params EOS { symrec * rec = sp_pop_symrec(); sp_pop_symrec(); sp_flush_scope_now(); sp_new_label(rec->name); }
;
catch_stmt: P_CATCH L_BR { sp_push_symrec(sp_start_lambda_decl(0)); } lambda_param_list R_BR 
{ symrec * func; symrec * wfunc = sp_pop_symrec(); sp_execute_lambda_call(wfunc); 
  func = sp_start_lambda_body(sp_pop_symrec()); if(func == NULL) YYABORT;
		sp_push_symrec(wfunc); sp_push_symrec(func); sp_flush_scope_now(); } 
	block_stmt 
{ symrec * func = sp_pop_symrec(); sp_end_lambda_decl(func); sp_push_inode(sp_create_inode(INS_CALL, func)); }
| P_CATCH single_block_stmt 
;
try_catch_stmt: P_TRY { sp_flush_scope_now(); sp_push_try_catch(); } 
  	single_block_stmt 
{  symrec * rel = sp_create_label(_RECAST(uchar *,"__tc")); sp_push_symrec(rel); sp_jump_to(INS_JMP, rel->name); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__cr"))); sp_set_cblock(); } 
  catch_stmt 
{ symrec * func = sp_pop_symrec(); symrec * rel = sp_pop_symrec(); sp_flush_scope_now(); sp_new_label(rel->name); 
	sp_pop_try_catch(func->ptotal& 0x7F); }
;
for_stmt: P_FOR { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__f"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); } 
	for_expr_stmt { symrec * rec = sp_pop_symrec(); rec = sp_pop_symrec(); sp_new_label(rec->name); }
| P_FOREACH { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__f"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); } 
	foreach_expr_stmt { symrec * rec = sp_pop_symrec(); rec = sp_pop_symrec(); sp_new_label(rec->name); 
	sp_flush_scope_now(); }
;
for_init_stmt: /* empty */ { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__fv"))); /* dummy variable */ }
| inline_var_decl { sp_clear_operation_stack(); sp_lhs_clear(); }
//| for_init_stmt ',' lazy_stmt { sp_flush_scope(); sp_clear_current_scope(); }
//| lazy_stmt { sp_flush_scope(); sp_clear_current_scope(); }
;
for_expr_stmt: L_BR for_init_stmt EOS { sp_flush_scope_now(); sp_clear_current_scope(); sp_start_loop(); }
	exprs EOS { symrec * rec = sp_pop_symrec(); sp_flush_scope_now(); sp_new_label(rec->name); sp_create_new_scope(NULL); }
	lazy_stmts R_BR { sp_clear_current_scope(); sp_create_new_scope(NULL); } block_stmt { sp_merge_scope_left(); sp_end_scope(); sp_end_loop(); sp_clear_current_scope(); }
;
foreach_init_stmt: var_init_decl { sp_push_symrec(sp_create_variable($1)); }
| VARIABLE { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $1)); }
;
foreach_expr_stmt: L_BR foreach_init_stmt P_IN { sp_flush_scope_now(); sp_clear_current_scope(); }
	rhs_val R_BR { symrec * rec; sp_pop_symrec(); sp_start_each_loop(sp_pop_symrec()); sp_flush_scope_now(); rec = sp_pop_symrec(); ; 
	sp_jump_to(INS_JFALSE, sp_peek_symrec()->name); sp_push_symrec(rec); sp_create_new_scope(NULL); }
   	{ sp_clear_current_scope(); sp_create_new_scope(NULL); } block_stmt { sp_merge_scope_left(); sp_end_scope(); sp_end_each_loop(); }
;
stmt: /* empty */
| lazy_stmts { sp_pop_symrec(); }
| P_BREAK { symrec * rec = sp_peek_symrec(); sp_jump_to(INS_JMP, rec->name); }
| P_CONTINUE { sp_continue_loop(); }
| P_RETURN { sp_exit_function_body(); }
| P_RETURN exprval { sp_return_function_body(); }
| P_GOTO VARIABLE { sp_jump_to(INS_JMP, $2); }
;
lazy_stmt: lazy_expr { sp_push_inode(sp_create_inode(INS_OBJDEL, NULL)); sp_clear_operation_stack(); sp_lhs_clear(); sm_printf("lazy_stmt>lazy_expr\n"); }
| assignment { sp_clear_operation_stack(); sp_lhs_clear(); }
| lhs_val { sp_push_inode(sp_create_inode(INS_OBJDEL, NULL)); sp_clear_operation_stack(); sp_lhs_clear(); sm_printf("lazy_stmt>lhs_val\n");  }
;
assignment: lhs_val S_EQ { sm_printf("equalizer\n"); sp_lhs_push_api_call($1); sp_set_scope_var(sp_peek_symrec()); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_end_expr(sp_peek_symrec()); sm_printf("assign %s\n", sp_peek_symrec()->name); }
| lhs_val S_ADDEQ { sp_lhs_push_api_call($1); sp_lhs_get($1); sp_set_scope_var(sp_peek_symrec()); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_SUBEQ { sp_lhs_push_api_call($1); sp_lhs_get($1); sp_set_scope_var(sp_peek_symrec()); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_MULEQ { sp_lhs_push_api_call($1); sp_lhs_get($1); sp_set_scope_var(sp_peek_symrec()); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_DIVEQ { sp_lhs_push_api_call($1); sp_lhs_get($1); sp_set_scope_var(sp_peek_symrec()); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_MODEQ { sp_lhs_push_api_call($1); sp_lhs_get($1); sp_set_scope_var(sp_peek_symrec()); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_ANDEQ { sp_lhs_push_api_call($1); sp_lhs_get($1); sp_set_scope_var(sp_peek_symrec()); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_AND, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_OREQ  { sp_lhs_push_api_call($1); sp_lhs_get($1); sp_set_scope_var(sp_peek_symrec()); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_OR, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_XOREQ { sp_lhs_push_api_call($1); sp_lhs_get($1); sp_set_scope_var(sp_peek_symrec()); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_XOR, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_SHLEQ { sp_lhs_push_api_call($1); sp_lhs_get($1); sp_set_scope_var(sp_peek_symrec()); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHL, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_SHREQ { sp_lhs_push_api_call($1); sp_lhs_get($1); sp_set_scope_var(sp_peek_symrec()); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHR, NULL)); sp_end_expr(sp_peek_symrec()); }
| S_ADDADD VARIABLE { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $2)); sp_set_scope_var(sp_peek_symrec()); sp_load_stack($2); sp_load_constant(_RECAST(uchar *, "1")); sp_operation_stack(INS_ADD); sp_store_stack($2); }
| S_SUBSUB VARIABLE { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $2)); sp_set_scope_var(sp_peek_symrec()); sp_load_stack($2); sp_load_constant(_RECAST(uchar *, "1")); sp_operation_stack(INS_SUB); sp_store_stack($2); }
| VARIABLE S_ADDADD { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $1)); sp_set_scope_var(sp_peek_symrec()); sp_lz_constant_after_scope($1, _RECAST(uchar *, "1"), INS_ADD); }
| VARIABLE S_SUBSUB { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $1)); sp_set_scope_var(sp_peek_symrec()); sp_lz_constant_after_scope($1, _RECAST(uchar *, "1"), INS_SUB); }
;
scope_assignment: rhs_val S_EQ { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_end_expr(sp_peek_symrec()); }
| rhs_val S_ADDEQ { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); sp_end_expr(sp_peek_symrec()); }
| rhs_val S_SUBEQ { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); sp_end_expr(sp_peek_symrec()); }
| rhs_val S_MULEQ { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); sp_end_expr(sp_peek_symrec()); }
| rhs_val S_DIVEQ { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); sp_end_expr(sp_peek_symrec()); }
| rhs_val S_MODEQ { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); sp_end_expr(sp_peek_symrec()); }
| rhs_val S_ANDEQ { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_AND, NULL)); sp_end_expr(sp_peek_symrec()); }
| rhs_val S_OREQ  { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_OR, NULL)); sp_end_expr(sp_peek_symrec()); }
| rhs_val S_XOREQ { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_XOR, NULL)); sp_end_expr(sp_peek_symrec()); }
| rhs_val S_SHLEQ { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_SHL, NULL)); sp_end_expr(sp_peek_symrec()); }
| rhs_val S_SHREQ { sp_lhs_push_api_call($1); sp_rhs_get($1); sp_set_scope_var(sp_peek_symrec()); sp_push_varctx_scope(); } rhs_lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_SHR, NULL)); sp_end_expr(sp_peek_symrec()); }
| step_variable {  }
;
step_variable:
| S_ADDADD VARIABLE { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $2)); sp_set_scope_var(sp_peek_symrec()); sp_load_stack($2); sp_load_constant(_RECAST(uchar *, "1")); sp_operation_stack(INS_ADD); sp_store_stack($2); }
| S_SUBSUB VARIABLE { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $2)); sp_set_scope_var(sp_peek_symrec()); sp_load_stack($2); sp_load_constant(_RECAST(uchar *, "1")); sp_operation_stack(INS_SUB); sp_store_stack($2); }
| VARIABLE S_ADDADD { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $1)); sp_set_scope_var(sp_peek_symrec()); sp_lz_constant_after_scope($1, _RECAST(uchar *, "1"), INS_ADD); }
| VARIABLE S_SUBSUB { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $1)); sp_set_scope_var(sp_peek_symrec()); sp_lz_constant_after_scope($1, _RECAST(uchar *, "1"), INS_SUB); }
;
instance_scope: assignment {  }
| lazy_expr { sp_clear_operation_stack(); sp_lhs_clear();  }
;
instance_val: lazy_val_decl { sm_printf("lazy_val_decl %s\n", sp_peek_symrec()->name); }
//| instance_val S_RNEXT VARIABLE L_BR { sp_pop_symrec(); sp_push_symrec(sp_start_method_call($3)); sp_create_new_scope(NULL); } method_param_list R_BR { symrec * meth = sp_pop_symrec(); sp_destroy_scope(); sp_flush_scope(); sp_push_symrec(sp_end_method_call(meth)); sm_printf("rhs method call\n"); }
| instance_val S_RNEXT VARIABLE L_BR { sm_printf("instance method call\n"); sp_create_method_scope(); sp_push_symrec(sp_start_method_call($3)); sp_create_new_scope(NULL); } lhs_method_param_list R_BR { symrec * meth = sp_pop_symrec();  sp_flush_scope_now(); sp_destroy_scope(); sp_end_method_call(meth); sm_printf("lhs method call\n"); } lhs_method_call { sp_destroy_method_scope(); }
| instance_val S_PDOT VARIABLE L_BR { sp_pop_symrec(); sp_flush_scope_now(); sm_printf("rhs api access %s\n", $3); sp_push_symrec(sp_start_function_call($3, 1)); sp_create_new_scope(NULL); } call_param_list R_BR { symrec * rec = sp_pop_symrec(); sp_flush_scope_now(); sp_destroy_scope(); sp_push_symrec(sp_end_function_call(rec)); }
| instance_val S_RNEXT VARIABLE { sm_printf("ins var access 2\n"); sp_lz_load_constant($3); $$=2; sp_push_symrec(sp_lhs_load(2, 1, sp_pop_symrec())); sm_printf("ins var access 2 end\n"); }
| instance_val S_PDOT VARIABLE { sm_printf("rhs object access\n"); sp_lz_load_constant($3); sp_push_symrec(sp_lhs_load(18, 1, sp_pop_symrec())); }
| instance_val L_SB lazy_expr R_SB { sp_pop_symrec(); sm_printf("rhs array access 1\n"); sp_push_symrec(sp_lhs_load(17, 1, sp_pop_symrec())); } 
| instance_val L_SB lazy_expr EOC lazy_expr R_SB { sp_pop_symrec(); sp_pop_symrec(); sm_printf("rhs array access 2\n"); sp_push_symrec(sp_lhs_load(17, 2, sp_pop_symrec())); }
//| lazy_scope_expr {  }
| L_BR { } rhs_lazy_scope R_BR {   }
;
rhs_val: lazy_val_function { $$=0;  }
| instance_val { $$=0; }
;
lhs_val: lhs_val S_PDOT VARIABLE { sp_lhs_push_api_call($1); sp_lhs_get($1); sm_printf("lhs object access\n"); sp_lz_load_constant($3); $$=18; sp_lhs_set(18); }
| lhs_val S_RNEXT VARIABLE { sp_lhs_push_api_call(128); sp_lhs_get(128); sm_printf("lhs var access\n"); sp_lz_load_constant($3); $$=2; sp_lhs_set(2); }
| lhs_val L_SB lhs_expr R_SB { sp_lhs_push_api_call($1); sp_lhs_get($1); sm_printf("lhs array access\n"); sp_pop_symrec(); $$=17; sp_lhs_set(17); sm_printf("lhs array access done\n"); }
| lhs_val S_PDOT VARIABLE L_BR { sm_printf("lhs api access %s\n", $3); sp_lhs_push_api_call($1); sp_lhs_get($1); sp_push_symrec(sp_start_function_call($3, 1)); sp_create_new_scope(NULL); } call_param_list R_BR { symrec * rec = sp_pop_symrec(); sp_flush_scope_now(); sp_destroy_scope(); sp_end_function_call(rec); }
//| lhs_val S_RNEXT VARIABLE L_BR { sp_lhs_lazy_get(0); sp_push_symrec(sp_start_method_call($3)); sp_create_new_scope(NULL); } method_param_list R_BR { symrec * meth = sp_pop_symrec(); sp_destroy_scope(); sp_end_method_call(meth); sm_printf("lhs method call\n"); }
| lhs_val S_RNEXT VARIABLE L_BR { sp_lhs_get($1); sm_printf("lhs method call\n"); sp_create_method_scope(); sp_push_symrec(sp_start_method_call($3)); sp_create_new_scope(NULL); } lhs_method_param_list R_BR { symrec * meth = sp_pop_symrec(); sp_flush_scope_now(); sp_destroy_scope(); sp_end_method_call(meth); sm_printf("lhs method call\n"); } lhs_method_call { sp_destroy_method_scope(); }
| VARIABLE { sm_printf("push %s\n", $1); sp_lhs_store(0, sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $1))); $$=0; sm_printf("push %s done\n", $1); } 
;
lhs_method_call: /* empty */
//| S_RNEXT VARIABLE L_BR { sp_push_symrec(sp_start_method_call($2)); sp_create_new_scope(NULL); } lhs_method_param_list R_BR lhs_method_call { symrec * meth = sp_pop_symrec(); sp_destroy_scope(); sp_end_method_call(meth); sm_printf("lhs method call\n"); }
| S_RNEXT VARIABLE L_BR { sp_create_method_scope(); sp_enqueue_symrec(sp_push_symrec(sp_start_method_call($2))); sp_create_new_scope(NULL); } lhs_method_param_list R_BR { symrec * meth = sp_dequeue_symrec(); sp_pop_symrec(); sp_destroy_scope(); sp_end_method_call(meth); sm_printf("lhs method call\n"); } lhs_method_call { sp_destroy_method_scope(); }
;
lhs_expr: L_BR lhs_expr R_BR 
// L_BR { sp_create_new_scope(NULL); } lhs_expr R_BR { sm_printf("lhs_scope\n"); sp_destroy_scope(); }
| lhs_expr S_ADD lhs_expr { sp_pop_symrec(); sp_push_varctx(sp_create_varctx(INS_ADD, NULL)); }
| lhs_expr S_SUB lhs_expr { sp_pop_symrec(); sp_push_varctx(sp_create_varctx(INS_SUB, NULL)); }
| lhs_expr S_MUL lhs_expr { sp_pop_symrec(); sp_push_varctx(sp_create_varctx(INS_MUL, NULL)); }
| lhs_expr S_DIV lhs_expr { sp_pop_symrec(); sp_push_varctx(sp_create_varctx(INS_DIV, NULL)); }
| lhs_expr S_MOD lhs_expr { sp_pop_symrec(); sp_push_varctx(sp_create_varctx(INS_MOD, NULL)); }
| step_variable { sp_lhs_store(0, sp_get_scope_var()); }
| VARIABLE { sm_printf("push %s\n", $1); sp_lhs_store(0, sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $1))); } 
| constant_val { sp_push_constant_s($1.value, $1.length); sp_lhs_store(0, sp_push_symrec(st_sym_select(SYM_TYPE_CONST, $1.value))); } 
| numeric_val { sp_push_constant($1.value); sp_lhs_store(0, sp_push_symrec(st_sym_select(SYM_TYPE_CONST, $1.value))); sp_lhs_store(INS_SYSCALL1, (symrec *)40); }
| error { YYABORT; }
;
lazy_scope_assignment: 
  rhs_val S_EQ    { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
| rhs_val S_ADDEQ { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
| rhs_val S_SUBEQ { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
| rhs_val S_MULEQ { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
| rhs_val S_DIVEQ { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
| rhs_val S_MODEQ { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
| rhs_val S_ANDEQ { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_AND, NULL)); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
| rhs_val S_OREQ  { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_OR , NULL)); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
| rhs_val S_XOREQ { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_XOR, NULL)); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
| rhs_val S_SHLEQ { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_SHL, NULL)); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
| rhs_val S_SHREQ { sp_lhs_push_api_call($1); sp_lhs_set($1); sp_rhs_get($1); sp_create_new_scope(NULL); sp_push_varctx_scope(); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_pop_varctx_scope(); sp_push_inode(sp_create_inode(INS_SHR, NULL)); sp_destroy_scope(); sp_end_expr(sp_get_scope_var()); }
;
lhs_scope_expr:
| lhs_val N_CONSTANT { sp_lhs_api_call($1, 2); sp_lz_load_constant_s($2.value, $2.length); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| lhs_val N_NUMERIC { sp_lhs_api_call($1, 2); sp_lz_load_numeric($2.value); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| lhs_val S_ADD { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| lhs_val S_SUB { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); }
| lhs_val S_MUL { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); }
| lhs_val S_DIV { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); }
| lhs_val S_MOD { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); }
| lhs_val S_AND { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_AND, NULL)); }
| lhs_val S_OR { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_OR, NULL)); }
| lhs_val S_XOR { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_XOR, NULL)); }
| lhs_val S_SHL { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHL, NULL)); }
| lhs_val S_SHR { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHR, NULL)); }
| lhs_val T_EQ { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CREQ, NULL)); }
| lhs_val T_NE { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRNE, NULL)); }
| lhs_val T_LT { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLT, NULL)); }
| lhs_val T_GT { sp_lhs_api_call($1, 2);  } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGT, NULL)); }
| lhs_val T_LTEQ { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLTEQ, NULL)); }
| lhs_val T_GTEQ { sp_lhs_api_call($1, 2); } lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGTEQ, NULL)); }
| S_NOT lhs_val { sp_lhs_api_call($2, 2); sp_push_inode(sp_create_inode(INS_NOT, NULL)); }
;
lazy_scope: //assignment
 assignment {  } 
| lhs_scope_expr { sp_clear_operation_stack(); sp_lhs_clear();  }
;
lazy_scope_expr:
  L_BR { sp_flush_scope(); sp_push_varctx_scope(); sp_create_new_scope(NULL); } lazy_scope R_BR { sp_flush_scope(); sp_lhs_get(17); sp_clear_operation_stack(); sp_lhs_clear(); sp_destroy_scope(); sp_pop_varctx_scope(); sm_printf("scope assignment end\n"); }
;
lazy_expr: lazy_scope_expr
| lazy_expr N_CONSTANT { sp_lz_load_constant_s($2.value, $2.length); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| lazy_expr N_NUMERIC { sp_lz_load_numeric($2.value); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| lazy_expr S_ADD lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| lazy_expr S_SUB lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); }
| lazy_expr S_MUL lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); }
| lazy_expr S_DIV lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); }
| lazy_expr S_MOD lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); }
| lazy_expr S_AND lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_AND, NULL)); }
| lazy_expr S_OR lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_OR, NULL)); }
| lazy_expr S_XOR lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_XOR, NULL)); }
| lazy_expr S_SHL lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHL, NULL)); }
| lazy_expr S_SHR lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHR, NULL)); }
| lazy_expr T_EQ lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CREQ, NULL)); }
| lazy_expr T_NE lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRNE, NULL)); }
| lazy_expr T_LT lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLT, NULL)); }
| lazy_expr T_GT lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGT, NULL)); }
| lazy_expr T_LTEQ lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLTEQ, NULL)); }
| lazy_expr T_GTEQ lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGTEQ, NULL)); }
| S_NOT lazy_expr { sp_push_inode(sp_create_inode(INS_NOT, NULL)); }
| rhs_val { sm_printf("lazy_expr:rhs_val\n"); }
| lambda_func_decl {  }
;
rhs_lazy_scope: //assignment
 assignment {  } 
| lhs_scope_expr { }
;
rhs_lazy_scope_expr:
  L_BR { sp_flush_scope(); sp_push_varctx_scope(); sp_create_new_scope(NULL); } rhs_lazy_scope R_BR { sp_flush_scope(); sp_lhs_get(17); sp_clear_operation_stack(); sp_lhs_clear(); sp_destroy_scope(); sp_pop_varctx_scope(); sm_printf("scope assignment end\n"); }
;
rhs_lazy_expr: rhs_lazy_scope_expr
| rhs_lazy_expr N_CONSTANT { sp_lz_load_constant_s($2.value, $2.length); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| rhs_lazy_expr N_NUMERIC { sp_lz_load_numeric($2.value); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| rhs_lazy_expr S_ADD rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| rhs_lazy_expr S_SUB rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); }
| rhs_lazy_expr S_MUL rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); }
| rhs_lazy_expr S_DIV rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); }
| rhs_lazy_expr S_MOD lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); }
| rhs_lazy_expr S_AND rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_AND, NULL)); }
| rhs_lazy_expr S_OR rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_OR, NULL)); }
| rhs_lazy_expr S_XOR rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_XOR, NULL)); }
| rhs_lazy_expr S_SHL rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHL, NULL)); }
| rhs_lazy_expr S_SHR rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHR, NULL)); }
| rhs_lazy_expr T_EQ rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CREQ, NULL)); }
| rhs_lazy_expr T_NE rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRNE, NULL)); }
| rhs_lazy_expr T_LT rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLT, NULL)); }
| rhs_lazy_expr T_GT rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGT, NULL)); }
| rhs_lazy_expr T_LTEQ rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLTEQ, NULL)); }
| rhs_lazy_expr T_GTEQ rhs_lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGTEQ, NULL)); }
| S_NOT rhs_lazy_expr { sp_push_inode(sp_create_inode(INS_NOT, NULL)); }
| rhs_val { sm_printf("rhs_lazy_expr:rhs_val\n"); }
| lambda_func_decl {  }
;
var_type_primitive: P_BYTE { $$ = VM_INT8; }
| P_HWORD { $$ = VM_INT16; }
| P_WORD { $$ = VM_INT32; }
| P_DWORD { $$ = VM_INT64; }
| P_FLOAT { $$ = VM_FLOAT; }
| P_DOUBLE { $$ = VM_DOUBLE; }
| P_VAR { $$ = 0; }
;
var_type_param: var_type_primitive { $$ = $1; }
| var_type_primitive L_SB R_SB { $$ = VM_ARRAY | $1; }
| P_STRING { $$ = VM_STRING; }
| P_PTR { $$ = VM_POINTER; }
;
var_type: var_type_primitive
| P_AUTO { $$ = VM_AUTO_POINTER; }
| P_PTR { $$ = VM_POINTER; }
| P_VOID { $$ = 1; }
;
scope_val: assignment { sm_printf("scope val assignment\n"); sp_push_inode(sp_create_inode(INS_OBJPUSH, sp_get_scope_var())); }
| lazy_expr { }
;
scope_expr: L_BR { sp_flush_scope(); sp_create_new_scope(NULL); } scope_val R_BR
;
cast_val: L_BR var_type R_BR { sprintf(_RECAST(char *, $$), "%d", $2); }
| scope_expr { symrec * rec = sp_get_scope_var(); sp_flush_scope(); sp_destroy_scope(); }
;
cast_expr: cast_val { sp_push_symrec(sp_start_api_call(30, 2)); } rhs_val { symrec * rec = sp_pop_symrec(); sp_lz_load_constant_s($1, strlen(_RECAST(const char *,$1))); sp_end_function_call(sp_pop_symrec()); sp_push_symrec(rec); sp_flush_scope(); }
| cast_val {  }
;
lazy_val_decl_new: P_NEW VARIABLE L_BR R_BR { sp_push_symrec(sp_lz_create_instance($2)); }
| VARIABLE { sp_push_symrec(sp_lz_load_variable($1)); } 
| array_decl { sp_push_symrec(sp_lz_load_array_constant()); }
| numeric_val { sp_push_symrec(sp_lz_load_numeric($1.value)); }
;
lazy_val_decl: lazy_val_decl_new {  }
| lazy_func_call {  }
| args_expr { }
| lazy_val_decl S_ADDADD { sp_push_inode(sp_create_inode(INS_OBJDUP, NULL)); sp_lz_load_constant(_RECAST(uchar *, "1")); sp_push_inode(sp_create_inode(INS_ADD, NULL)); sp_push_inode(sp_create_inode(INS_OBJPOP, sp_peek_symrec())); }
| lazy_val_decl S_SUBSUB { sp_push_inode(sp_create_inode(INS_OBJDUP, NULL)); sp_lz_load_constant(_RECAST(uchar *, "1")); sp_push_inode(sp_create_inode(INS_SUB, NULL)); sp_push_inode(sp_create_inode(INS_OBJPOP, sp_peek_symrec())); }
| S_ADDADD lazy_val_decl { sp_lz_load_constant(_RECAST(uchar *, "1")); sp_push_inode(sp_create_inode(INS_ADD, NULL)); sp_push_inode(sp_create_inode(INS_OBJDUP, NULL)); sp_push_inode(sp_create_inode(INS_OBJPOP, sp_peek_symrec())); }
| S_SUBSUB lazy_val_decl { sp_lz_load_constant(_RECAST(uchar *, "1")); sp_push_inode(sp_create_inode(INS_SUB, NULL)); sp_push_inode(sp_create_inode(INS_OBJDUP, NULL)); sp_push_inode(sp_create_inode(INS_OBJPOP, sp_peek_symrec())); }
;
lazy_val_function: cast_expr {  }
| instance_decl { sp_push_symrec(sp_lz_load_class($1)); }
| P_SIZEOF L_BR lazy_expr R_BR { sp_push_inode(sp_create_inode(INS_OBJSZ, NULL)); }
| constant_val { sp_push_symrec(sp_lz_load_constant_s($1.value, $1.length)); }
;
lazy_val: lazy_val_function {  }
| lazy_val_decl {  }
| lambda_func_decl {  }
;
lazy_func_call: VARIABLE L_BR { symrec * rec = sp_push_symrec(sp_start_function_call($1, 0)); sm_printf("start call func: %s\n",rec->name); } call_param_list R_BR { symrec * rec = sp_peek_symrec(); sm_printf("end call func: %s\n",rec->name); sp_end_function_call(rec); }
;
array_decl: P_HEX L_BR { sp_new_array_constant(); } constant_seq R_BR
;
constant_val: CONSTANT { $$=$1; }
| N_CONSTANT { $$=$1; }
;
numeric_val: NUMERIC { $$=$1; }
| N_NUMERIC { $$=$1; }
;
constant_seq :  /* empty */
| constant_val { sp_push_array_constant($1.value); } ',' constant_seq
| constant_val { sp_push_array_constant($1.value); } 
;
args_expr: L_CL { sp_push_symrec(sp_start_api_call(15, 0)); } obj_params R_CL { sp_push_symrec(sp_end_function_call(sp_pop_symrec())); }
| L_SB { sp_push_symrec(sp_start_api_call(16, 0)); } arr_params R_SB { sp_push_symrec(sp_end_function_call(sp_pop_symrec())); }
;
obj_params: { }
| obj_param { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); } ',' obj_params
| obj_param { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
;
arr_params: { }
| arr_param { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); } ',' arr_params
| arr_param { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
;
obj_param: /* empty */ 
| VARIABLE { sp_push_symrec(sp_start_api_call(14, 2)); sp_lz_load_constant($1); } EOC lazy_expr { sp_pop_symrec(); sp_push_symrec(sp_end_function_call(sp_pop_symrec())); }
;
arr_param: /* empty */
| obj_param
| lazy_val { }
;
lambda_func_decl: P_FUNCTION L_BR { sp_push_symrec(sp_start_lambda_decl(0)); } lambda_param_list R_BR 
{ symrec * func; sp_push_symrec(sp_execute_lambda_call(sp_pop_symrec())); 
  func = sp_start_lambda_body(sp_pop_symrec()); if(func == NULL) YYABORT;
		sp_push_symrec(func); sp_flush_scope_now(); } 
	block_stmt 
{ sp_end_lambda_decl(sp_peek_symrec()); sp_lz_load_lambda(sp_peek_symrec()); }
;
instance_decl: VARIABLE { strcpy(_RECAST(char *, $$), _RECAST(const char *, $1)); }
;
case_stmt: P_SWITCH case_expr_param { sp_push_symrec(sp_start_case()); sp_push_symrec(sp_create_label(_RECAST(uchar *, "__d"))); } 
	L_CL case_block_stmt R_CL { symrec * rec = sp_pop_symrec(); symrec * srec = sp_pop_symrec(); sp_end_case(srec); sp_new_label(rec->name); } 
;
case_single_stmt: /* empty */
| P_CASE constant_val { sp_push_constant_s($2.value, $2.length); } EOC { symrec * rec = sp_pop_symrec(); sp_label_case_s(sp_peek_symrec(), $2.value, $2.length); sp_push_symrec(rec); }
| P_DEFAULT EOC { symrec * rec = sp_pop_symrec(); sp_default_case(sp_peek_symrec()); sp_push_symrec(rec); }
| block_stmt
;
exprs: expr T_AND { symrec * srec = sp_pop_symrec(), * brec = sp_pop_symrec(); sp_push_symrec(brec); sp_push_symrec(srec); sp_jump_to(INS_JFALSE, brec->name); } exprs
| expr T_OR { symrec * srec = sp_peek_symrec(); sp_jump_to(INS_JTRUE, srec->name); } exprs
| expr { symrec * srec = sp_pop_symrec(); symrec * rec = sp_peek_symrec(); sp_jump_to(INS_JFALSE, rec->name); sp_push_symrec(srec); }
;
expr: exprval { }
;
exprval: lazy_expr { symrec * srec = sp_pop_symrec(); symrec * rec = sp_peek_symrec(); rec->sa_currec = srec; sp_flush_scope_now(); sm_printf("flush end\n"); }
//lazy_expr { symrec * srec = sp_pop_symrec(); symrec * rec = sp_peek_symrec(); rec->sa_currec = srec; sp_flush_scope(); sm_printf("flush end\n"); }
;
sys_func_decl: VARIABLE L_BR { sp_push_symrec(sp_declare_function($1)); } sys_param_list R_BR sys_extended_decl { symrec * rec = sp_pop_symrec(); if(sp_declare_function_end(rec->name) == NULL) YYABORT; }										/* internal function prototype */
;
sys_extended_decl: /* empty */
| P_SYSCALL CONSTANT { sp_declare_function_extern($2.value); }
;
sys_param_list: /* empty */
| VARIABLE { sp_declare_function_param(0); } ',' sys_param_list
| VARIABLE { sp_declare_function_param(0); }  
;
lambda_param_list: /* empty */
| VARIABLE { sp_push_symrec(sp_create_lambda_param($1)); } ','  lambda_param_list { symrec * rec = sp_pop_symrec(); sp_lz_store_variable(rec->name); }
| VARIABLE { symrec * rec = sp_create_lambda_param($1); sp_lz_store_variable($1); }
;
call_param_list: /* empty */
| lazy_expr { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); } ',' {sm_printf("call_param\n"); sp_flush_scope(); } call_param_list
| lazy_expr { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
;
method_param_list: /* empty */
| lazy_expr { sp_pop_symrec(); sp_push_method_param(sp_peek_symrec()); }
| lazy_expr { sp_pop_symrec(); sp_push_method_param(sp_peek_symrec()); } ',' {sm_printf("method_param\n"); sp_flush_scope(); } method_param_list	//original
;
lhs_method_param_list: /* empty */
| lazy_expr { sp_pop_symrec(); sp_push_method_param(sp_peek_symrec()); sp_flush_method_param(); }
//| lazy_expr { sp_pop_symrec(); sp_push_method_param(sp_peek_symrec()); } ',' {sm_printf("method_param\n"); sp_flush_scope(); } lhs_method_param_list	//original
| lazy_expr { sp_pop_symrec(); sp_push_method_param(sp_peek_symrec()); sp_flush_method_param(); } ',' { sp_destroy_method_scope(); sp_create_method_scope(); } lhs_method_param_list
;
accessor_func: P_PUBLIC P_FUNCTION { $$ = TRUE; }
| P_PRIVATE P_FUNCTION { $$ = FALSE; }
| P_FUNCTION { $$ = FALSE; }
;
interface_param_list: /* empty */
| var_type_param { sp_declare_function_param($1); } ',' interface_param_list
| var_type_param { sp_declare_function_param($1); }  
| var_type_param VARIABLE { sp_declare_function_param($1); } ',' interface_param_list
| var_type_param VARIABLE { sp_declare_function_param($1); }  
;
interface_func_decl: VARIABLE L_BR { sp_push_symrec(sp_declare_interface_function($1)); } interface_param_list R_BR { symrec * rec = sp_peek_symrec(); if(sp_declare_function_end(rec->name) == NULL) YYABORT; }									/* internal function prototype */
;
call_type: /* empty */	{ $$ = CALL_TYPE_C; }
| P_STDCALL  { $$ = CALL_TYPE_STD; }
| P_FASTCALL { $$ = CALL_TYPE_FAST; }
;
interface_stmt: call_type var_type interface_func_decl EOS { sp_register_method(sp_pop_symrec(), $1, $2); }
| error_stmt 
;
class_stmt: accessor_func VARIABLE { sp_push_symrec(sp_create_function($2)); sp_create_new_scope(NULL); } body_func_params
	{ symrec * func = sp_start_function_body(sp_pop_symrec(), $1); if(func == NULL) YYABORT;
		sp_push_symrec(func); sp_flush_scope_now(); } body_func_extended_decl 
	block_stmt 
	{ sp_end_function_body(sp_pop_symrec()); sp_flush_scope_now(); } 			/*function declaration with alias */
| P_EXTERN P_FUNCTION sys_func_decl EOS
| error_stmt
;
body_func_extended_decl: 	/* empty */
| P_ALIAS CONSTANT { sp_install_menu($2.value, sp_peek_symrec()); }		/* install current function as menu (setup menu) */
| P_EVENT CONSTANT { sp_install_event($2.value, sp_peek_symrec()); }		/* install current function as event (setup event) */
;
body_func_param_list: /* empty */
| VARIABLE { sp_push_symrec(sp_create_param($1)); } ','  body_func_param_list { symrec * rec = sp_pop_symrec(); sp_lz_store_variable(rec->name); }
| VARIABLE { symrec * rec = sp_create_param($1); sp_lz_store_variable($1); }
;
error_stmt: error { YYABORT; }
;
%%

void sp_init_parser(void) {
	//yylval.token = (yytoken *)malloc(sizeof(yytoken));
	errors = 0;
	yyinit();
}

uint32 sp_parse(void) {
	//start parse
	yyparse();
	//cleanup for any resources
	sp_cleanup_parser();			//semantic parser clean up
	lp_clear_globals();				//lexical analyzer clean up
	
	return errors;
}

extern void sp_error_push(uchar * msg);
void sp_error(uchar * msg, ...) {
	uchar buffer[512];
	uchar pbuffer[520];
	va_list argptr;
	va_start(argptr, msg);
	vsprintf(_RECAST(char *, buffer), _RECAST(const char *, msg), argptr);
	sprintf(_RECAST(char *, pbuffer), "[line: %d] %s", p_config->line + 1, buffer);
	sp_error_push(pbuffer);
	//printf("%s\n", buffer);
	//yyerror ( _RECAST(char *, buffer));
	va_end(argptr);              /* Reset variable argument list. */
	//errors ++;
}

/*=========================================================================
MAIN
=========================================================================*/
#ifdef STANDALONE_COMPILER
void main( int argc, char *argv[] )
{ 
	//extern FILE *yyin;
	int i = 1;
	err_record * iterator;
	uint16 strCount = 0;
	uchar gen_asm = 0, gen_il = 0, gen_scr = 0;
	//++argv;			//skip exepath
	uchar opt_level = 0;
	//sp_init_parser();
	if(argc == 1) goto print_usage;
	while(i != (argc - 1)) {
		if(strcmp(argv[i], "-ca") == 0) {
			gen_il = 1;
		} else if(strcmp(argv[i], "-c") == 0) {
			gen_asm = 1;
		} else if(strcmp(argv[i], "-cd") == 0) {
			gen_scr = 1;
		} else if(strcmp(argv[i], "-o1") == 0) {
			opt_level |= IS_OPTIMIZE_L1;
		} else if(strcmp(argv[i], "-o2") == 0) {
			opt_level |= IS_OPTIMIZE_L2 | IS_OPTIMIZE_L1;
		} else {
			print_usage:
			printf("usage: stack [options] filename\n	\
					options:\
					\n-ca\tcompilassembler [source->bin]\
					\n-cd\tscript generator [source->apdu]\
					\n-c\tcompile [source->asm]\
					\n-o1\tpeephole optimization\
					\n-o2\tdeadcode elimination");
			exit(0);
		}
		i++;
	}
	//yyin = fopen( argv[i], "r" );
	if(sp_init(_RECAST(uchar *, argv[i])) != NULL) {
		if(gen_il) is_init(_RECAST(uchar *, argv[i]));
		if(gen_asm) as_init(_RECAST(uchar *, argv[i]));
		if(gen_scr) sc_init(_RECAST(uchar *, argv[i]));
		i = sp_parse();
		if(i == 0) {
			is_link_optimize(opt_level);
			if(gen_il) {	
				is_file_flush();
			}
			if(gen_asm) {
				as_flush();
			}
			if(gen_scr) {
				sc_flush();
				sc_cleanup();
			}
		} else {
			iterator = sp_error_get_enumerator();
			while(iterator != NULL) {
				//lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
				strCount ++;
				printf("%s\n", iterator->buffer);
				iterator = sp_error_next_record(iterator);
			}
		}
		//fclose(yyin);
	}	
}
#endif

/*=========================================================================
YYERROR
=========================================================================*/
extern void sp_error(uchar *, ...);
int yyerror ( char *s ) /* Called by yyparse on error */
{
	errors++;
	//printf("Error (%i) on file %s line %i: %s\n", errors, p_config->filename, p_config->line + 1, s);
	sp_error(SP_ERR_PARSER, s, yylval.bytes);
	return 0;
}
/**************************** End Grammar File ***************************/
