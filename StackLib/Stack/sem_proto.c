#include "../defs.h"
#include "../config.h"
#include "sym_table.h"
#include "il_streamer.h"
#include "token.h"
#include "pkg_encoder.h"
#include "pkg_linker.h"
#include "scr_generator.h"
#include "lex_proto.h"
#include "sem_proto.h"
#include <stdarg.h>

#define LAMBDA_DEPTH		32
extern uint32 _current_offset;			//already defined by il_streamer
err_record * _sp_err_head = NULL;
err_record * _sp_err_tail = NULL;
uint32 _sp_offstack[SEM_MAX_STACK];
symrec * _sp_symstack[SEM_MAX_STACK];
symrec * _sp_symqueue[SEM_MAX_STACK];
int _sp_offstack_index = 0;
int _sp_symstack_index = 0;
int _sp_symqueue_index = 0;
uint32 _sp_label_index = 0;
uint32 _sp_context_cntr = 0;
symrec * _sp_current_function = NULL;			//currently parsed function, a symbol stored the current function declaration
symrec * _sp_end_function_body = NULL;			//a label marked the end of function body (but still within function body)
symrec * _sp_skip_function_body = NULL;			//a label marked the next statement outside function body

lambda_context _lambda_stack[LAMBDA_DEPTH];
uint16 _lambda_index = 0;

lambda_context * sp_push_lambda() {
	lambda_context * curctx;
	if(_lambda_index == LAMBDA_DEPTH) return NULL;
	curctx = &_lambda_stack[_lambda_index++];
	curctx->context_cntr = _sp_context_cntr;
	curctx->func = _sp_current_function;
	curctx->exit_func = _sp_end_function_body;
	_sp_context_cntr = 0;
	return curctx;
}

lambda_context * sp_pop_lambda() {
	lambda_context * curctx;
	if(_lambda_index == 0) return NULL;
	curctx = &_lambda_stack[--_lambda_index];
	_sp_context_cntr = curctx->context_cntr;
	_sp_current_function = curctx->func;
	_sp_end_function_body = curctx->exit_func;
	return curctx;
}

void sp_push_offset(void) {								//
	_sp_offstack[_sp_offstack_index++] = _current_offset;
}

uint32 sp_pop_offset(void) {								//
	return _sp_offstack[--_sp_offstack_index]; 
}

symrec * sp_push_symrec(symrec * rec) {								//
	//printf("push stack_index:%d\n", _sp_symstack_index);
	_sp_symstack[_sp_symstack_index++] = rec;
	//sm_printf("==================PUSH stack_index:%d %s\n", _sp_symstack_index - 1, rec->name);
	return rec;
}

symrec * sp_pop_symrec(void) {								//
	symrec * rec;
	//if(_sp_symstack_index == 0) return NULL;
	//rec = _sp_symstack[_sp_symstack_index-1];
	rec = _sp_symstack[--_sp_symstack_index];
	//sm_printf("==================POP stack_index:%d %s\n", _sp_symstack_index, rec->name);
	//return _sp_symstack[--_sp_symstack_index]; 
	return rec;
}

symrec * sp_enqueue_symrec(symrec * rec) {								//
	//printf("push stack_index:%d\n", _sp_symstack_index);
	_sp_symqueue[_sp_symqueue_index++] = rec;
	//sm_printf("==================ENQUEUE queue_index:%d %s\n", _sp_symqueue_index - 1, rec->name);
	return rec;
}

symrec * sp_dequeue_symrec(void) {								//
	symrec * rec;
	//if(_sp_symstack_index == 0) return NULL;
	//rec = _sp_symstack[_sp_symstack_index-1];
	if(_sp_symqueue_index == 0) return NULL;
	//sp_pop_symrec();
	rec = _sp_symqueue[0];
	_sp_symqueue_index--;
	for(int i=0;i<_sp_symqueue_index;i++) {
		_sp_symqueue[i] = _sp_symqueue[i+1];
	}
	//sm_printf("==================DEQUEUE queue_index:%d %s\n", _sp_symqueue_index, rec->name);
	//return _sp_symstack[--_sp_symstack_index]; 
	return rec;
}

symrec * sp_look_symrec(void) {								//
	symrec * rec = _sp_symqueue[_sp_symqueue_index-1];
	return rec; 
}

symrec * sp_peek_symrec(void) {								//
	symrec * rec = _sp_symstack[_sp_symstack_index-1];
	return rec; 
}

symrec * sp_prev_symrec(void) {								//
	symrec * rec = _sp_symstack[_sp_symstack_index-2];
	return rec; 
}

void sp_clear_symrec() {
	uint16 i;
	sm_printf("==============CLEAR STACK\n");
	for(i=0;i<_sp_symstack_index;i++) {
		_sp_symstack[i] = NULL;
	}
	_sp_symstack_index = 0;
	sm_printf("==============CLEAR QUEUE\n");
	for(i=0;i<_sp_symqueue_index;i++) {
		_sp_symqueue[i] = NULL;
	}
	_sp_symqueue_index = 0;
}

ins_node * _op_stack[SEM_MAX_STACK];
uint8 _op_index = 0;
var_context * _sp_varctx_stack[SEM_MAX_STACK];
uint8 _sp_varctx_index = 0;

scope_operation * _sp_scope_stack[SEM_MAX_STACK];
uint8 _sp_scope_index = 0;

void sp_push_varctx_scope_s(int line) {
	scope_operation * scope;
	//return;
	if(_sp_scope_index == SEM_MAX_STACK) return;
	scope = (scope_operation *)malloc(sizeof(scope_operation));
	memset(scope, 0, sizeof(scope_operation));
	memcpy(scope->op_stack, _op_stack, sizeof(_op_stack));
	memcpy(scope->varctx_stack, _sp_varctx_stack, sizeof(_sp_varctx_stack));
	scope->op_index = _op_index;
	scope->varctx_index = _sp_varctx_index;
	_op_index = 0;
	_sp_varctx_index = 0;
	_sp_scope_stack[_sp_scope_index++] = scope;
	//sm_printf("sp_push_varctx_scope\n");
}

void sp_pop_varctx_scope_s(int line) {
	scope_operation * scope;
	//return;
	if(_sp_scope_index == 0) return;
	scope = _sp_scope_stack[--_sp_scope_index];
	memcpy(_op_stack, scope->op_stack, sizeof(_op_stack));
	memcpy(_sp_varctx_stack, scope->varctx_stack, sizeof(_sp_varctx_stack));
	_op_index = scope->op_index;
	_sp_varctx_index = scope->varctx_index;
	free(scope);
	//sm_printf("sp_pop_varctx_scope\n");
}

var_context * sp_push_varctx(var_context * vctx) {
	_sp_varctx_stack[_sp_varctx_index++] = vctx;
	return vctx;
}

var_context * sp_pop_varctx() {
	return _sp_varctx_stack[--_sp_varctx_index];
}

var_context * sp_create_varctx(uint8 operation, symrec * var) {
	var_context * vctx = (var_context *)malloc(sizeof(var_context));
	vctx->operation = operation;
	vctx->var = var;
	vctx->inode = NULL;
	return vctx;
}

symrec * sp_lhs_load_variable(uchar * varname) {
	symrec * rec = st_sym_select(SYM_TYPE_VAR, varname); 
	if(rec != NULL) { sp_push_inode(sp_create_inode(INS_OBJPUSH, rec)); return rec; }
	if(rec == NULL) return sp_lz_load_method(varname);//try load method if no variable found
	return rec;
}

symrec * sp_lhs_store_variable(uchar * varname) {
	symrec * rec = st_sym_select(SYM_TYPE_VAR, varname); 
	if(rec == NULL) { sp_error(SP_ERR_UNDEF_VARIABLE, varname); return NULL; }
	sp_push_inode(sp_create_inode(INS_OBJPOP, rec));
	return rec;
}

uint32 sp_gencode(uint32 offset, uchar opcode, ...) {
	va_list argptr;
	symrec * rec;
	int szinst = 0, index;
	//va_start(argptr, opcode);
	va_start(argptr, opcode);
	//return is_gencode(offset, opcode, argptr);
	rec = va_arg(argptr, symrec *);
/*
	switch(opcode) {
		case INS_LBL:					//create new label
			//rec = va_arg(argptr, symrec *);
			szinst = 0;				//predirective instruction (not implemented)
			break;
		case INS_OBJPUSH:
		case INS_OBJPOP:
		case INS_OBJSTORE:
			//rec = va_arg(argptr, symrec *);
			szinst = 2;
			break;
		case INS_SWITCH:
			//rec = va_arg(argptr, symrec *);
			szinst = 3;
			break;
		case INS_OBJDEL:
		case INS_OBJSUB:
		case INS_OBJSZ:
		case INS_OBJDUP:
		case INS_ADD:
		case INS_SUB:
		case INS_MUL:
		case INS_DIV:
		case INS_MOD:
		case INS_AND:
		case INS_OR:
		case INS_XOR:
		case INS_SHL:
		case INS_SHR:
		case INS_NOT:
		case INS_RET:
		case INS_NOP:
		case INS_CREQ:			//64	//-> compare equal 
		case INS_CRNE:			//65	//-> compare not equal
		case INS_CRGT:			//66	//-> compare greater than 
		case INS_CRLT:			//67	//-> compare less than
		case INS_CRGTEQ:			//68	//-> compare greater than
		case INS_CRLTEQ:			//69	//-> compare less than 
			szinst = 1;
			break;
		case INS_SYSCALL0:
		case INS_SYSCALL1:
		case INS_SYSCALL2:
		case INS_SYSCALL3:
		case INS_SYSCALL4:
		case INS_SYSCALL5:
		case INS_SYSCALL6:
		case INS_SYSCALL7:
		case INS_SYSCALL8:
		case INS_SYSCALL9:
		case INS_SYSCALL10:
		case INS_SYSCALL11:
		case INS_SYSCALL12:
		case INS_SYSCALL13:
		case INS_SYSCALL14:
		case INS_SYSCALL15:
			index = rec;
			if(index < 254) {
				szinst = 2;
			} else {
				szinst = 4;
			}
			break;
		case INS_SCTX:
		case INS_RCTX:
			//index = va_arg(argptr, uint32);
			szinst = 2;
			break;
		case INS_F2O_0:
		case INS_F2O_1:
		case INS_F2O_2:
		case INS_F2O_3:
		case INS_F2O_4:
		case INS_F2O_5:
		case INS_F2O_6:
		case INS_F2O_7:
		case INS_F2O_8:
		case INS_F2O_9:
		case INS_F2O_10:
		case INS_F2O_11:
		case INS_F2O_12:
		case INS_F2O_13:
		case INS_F2O_14:
		case INS_F2O_15:
		case INS_CALL:
		case INS_EXTCALL0:
		case INS_EXTCALL1:
		case INS_EXTCALL2:
		case INS_EXTCALL3:
		case INS_EXTCALL4:
		case INS_EXTCALL5:
		case INS_EXTCALL6:
		case INS_EXTCALL7:
		case INS_EXTCALL8:
		case INS_EXTCALL9:
		case INS_EXTCALL10:
		case INS_EXTCALL11:
		case INS_EXTCALL12:
		case INS_EXTCALL13:
		case INS_EXTCALL14:
		case INS_EXTCALL15:
		case INS_OBJCONST:
			szinst = 0;
			break;
		case INS_JMP:					//jump to specified label
		case INS_JTRUE:
		case INS_JFALSE:
			szinst = 3;
			break;
		case INS_OBJNEW:
			szinst = 3;
			break;

		default: break;
	}
*/
	sp_push_inode(sp_create_inode(opcode, rec)); 
	va_end(argptr);              /* Reset variable argument list. */
	return szinst;
}

void sp_lhs_get_s(uint16 api_id, uint8 windex) {
	var_context * vctx;
	symrec * var = sp_peek_symrec();
	uint8 index = windex;
	//if(api_id != 0) sp_clone_scope();
	if (windex == 0) { sm_printf("lhs_get_empty! index\n"); }
	for(index = 0; index < windex; index++) {
		vctx = _sp_varctx_stack[index];
		printf("varctx : [%d,]\n", vctx->operation);
		if(vctx->operation == 0) {
			switch (vctx->var->type & 0x0F) {
				case SYM_TYPE_CONST:
					_current_offset += sp_gencode(_current_offset, INS_OBJCONST, vctx->var); break;
				case SYM_TYPE_VAR:
					_current_offset += sp_gencode(_current_offset, INS_OBJPUSH, vctx->var); break;
				default: break;
			}
		} else {
			//if(vctx->operation == 17 || vctx->operation)
			_current_offset += sp_gencode(_current_offset, vctx->operation, vctx->var);
		}
	}
}

void sp_lhs_api_call(int api_id, uint8 arg_count) {
	sm_printf("sp_lhs_api_call\n");
	if (api_id != 0) {
		sm_printf("syscall_%d?%d\n", api_id, 2);
		//sp_push_varctx(sp_create_varctx(INS_SYSCALL + 2, (symrec *)api_id));
		sp_push_inode(sp_create_inode(INS_SYSCALL + arg_count, (symrec *)api_id));
	}
}

void sp_lhs_push_api_call(int api_id) {
	if (api_id != 0) {
		sm_printf("push syscall_%d?%d\n", api_id, 2);
		sp_push_varctx(sp_create_varctx(INS_SYSCALL + 2, (symrec *)api_id));
	}
}

void sp_lhs_get(uint16 api_id) {
	uint8 i;
	if(api_id == 128) return;		//INS_SYSCALL0
	sp_lhs_get_s(api_id, _sp_varctx_index);
	//if(api_id != 0) sp_end_function_call(sp_start_api_call(api_id, 2));		//PUSH VAR
}

void sp_rhs_get_s(uint8 windex) {
	var_context * vctx;
	symrec * var = sp_peek_symrec();
	uint8 index = windex;
	sp_clone_scope();
	if (windex == 0) { sm_printf("rhs_get_empty!\n"); }
	for (index = 0; index < windex; index++) {
		vctx = _sp_varctx_stack[index];
		if (vctx->operation == 0) {
			switch (vctx->var->type & 0x0F) {
			case SYM_TYPE_CONST:
				_current_offset += sp_gencode(_current_offset, INS_OBJCONST, vctx->var); break;
			case SYM_TYPE_VAR:
				_current_offset += sp_gencode(_current_offset, INS_OBJPUSH, vctx->var); break;
			default: break;
			}
		}
		else {
			//if(vctx->operation == 17 || vctx->operation)
			_current_offset += sp_gencode(_current_offset, vctx->operation, vctx->var);
		}
	}
}

void sp_rhs_get(uint16 api_id) {
	uint8 i;
	if (api_id == 128) return;		//INS_SYSCALL0
	sp_rhs_get_s(_sp_varctx_index);
	//if (api_id != 0) sp_end_function_call(sp_start_api_call(api_id, 2));		//PUSH VAR
}

void sp_lhs_lazy_get_s(uint8 windex) {
	var_context * vctx;
	symrec * var = sp_peek_symrec();
	uint8 index = windex;
	printf("clone scope\n");
	sp_clone_scope();
	if (windex == 0) { sm_printf("lhs_get_empty!\n"); }
	for (index = 0; index < windex; index++) {
		vctx = _sp_varctx_stack[index];
		if (vctx->operation == 0) {
			switch (vctx->var->type & 0x0F) {
			case SYM_TYPE_CONST:
				//_current_offset += sp_gencode(_current_offset, INS_OBJCONST, vctx->var); break;
				sp_push_inode(sp_create_inode(INS_OBJCONST, vctx->var));
				break;
			case SYM_TYPE_VAR:
				//_current_offset += sp_gencode(_current_offset, INS_OBJPUSH, vctx->var); break;
				sp_push_inode(sp_create_inode(INS_OBJPUSH, vctx->var));
				break;
			default: break;
			}
		}
		else {
			//if(vctx->operation == 17 || vctx->operation)
			//_current_offset += sp_gencode(_current_offset, vctx->operation, vctx->var);
			sp_push_inode(sp_create_inode(vctx->operation, vctx->var));
		}
	}
}

void sp_lhs_lazy_get(uint16 api_id) {
	uint8 i;
	if (api_id == 128) return;		//INS_SYSCALL0
	sp_lhs_lazy_get_s(_sp_varctx_index);
	//if (api_id != 0) sp_end_function_call(sp_start_api_call(api_id, 2));		//PUSH VAR
}

void sp_lhs_set(uint16 api_id) {
	uint8 i;
	var_context * vctx;
	symrec * var = sp_peek_symrec();
	//vctx = sp_push_varctx(sp_create_varctx(INS_SYSCALL1, (symrec *)api_id));
	//vctx->inode = sp_get_inode ();
	//sp_end_function_call(sp_start_api_call(api_id, 2));		//get OR at syscall
	//for(i = 0; i <= _sp_varctx_index;i++) {
	if(api_id == 2) {
		sp_push_operation(1, 3);			//var set operation
	} else {
		sp_push_operation(22, 3);			//arg set operation
	}
	//}
}

void sp_lhs_store(uint8 ins, symrec * rec) {
	if(rec != NULL) {
		sm_printf("sp_lhs_store %s\n", rec->name);
		if(_op_index == 0) {
			switch(rec->type & 0x0F) {
				case SYM_TYPE_VAR:		//store result 
					sp_push_operation_stack(sp_create_inode(INS_OBJPOP, rec));
					break;
				default:				//discard result
					sp_push_operation_stack(sp_create_inode(INS_OBJDEL, rec));
					break;
			}
		}
		sp_push_varctx(sp_create_varctx(ins, rec));
	}
}

symrec * sp_lhs_load(uint16 api_id, uint8 num_args, symrec * rec) {
	symrec * wrec;
	if(api_id != 0) {
		wrec = sp_end_function_call(sp_start_api_call(api_id, num_args + 1));
	} else {
		wrec = sp_lz_load_variable(rec->name);
	}
	return wrec;
}

void sp_lhs_clear() {
	uint8 index;
	sm_printf("lhs clear\n");
	for(index =0; index < _sp_varctx_index; index++) {
		if(_sp_varctx_stack[index] != NULL) free(_sp_varctx_stack[index]);
	}
	_sp_varctx_index = 0;
}

static ins_node * _sp_inode_stack[SEM_MAX_STACK * 2];
static int _sp_inode_index = 0;
extern const char * _ins_name[];
ins_node * sp_create_inode(uchar ins, symrec * rec) {
	ins_node * inode = (ins_node *)malloc(sizeof(ins_node));
	inode->ins = ins;
	inode->rec = rec;
	inode->next = NULL;
	inode->scope_var = NULL;
	return inode;
}

ins_node * sp_push_iroot(ins_node * inode) { _sp_inode_stack[_sp_inode_index++] = inode; return inode; }
ins_node * sp_pop_iroot() { 
	if(_sp_inode_index < 1) return NULL; 
	return _sp_inode_stack[--_sp_inode_index]; 
}
ins_node * sp_current_iroot() { 
	if(_sp_inode_index < 1) return NULL;
	return _sp_inode_stack[_sp_inode_index - 1]; 
}
ins_node * sp_right_iroot() { 
	if(_sp_inode_index < 2) return NULL;
	return _sp_inode_stack[_sp_inode_index - 2]; 
}
ins_node * sp_left_iroot() { 
	if(_sp_inode_index < 3) return NULL;
	return _sp_inode_stack[_sp_inode_index - 3]; 
}

ins_node * sp_push_inode(ins_node * inode) {
	ins_node * iroot = sp_current_iroot();
	ins_node * itr = iroot;
	if(itr == NULL) { return itr; }
	while(itr->next != NULL) {
		itr = itr->next;
	}
	itr->next = inode;
	return inode;
}

ins_node * sp_pop_inode() {
	ins_node * iroot = sp_current_iroot();
	ins_node * itr = iroot;
	ins_node * prev = NULL;
	if(itr == NULL) { return itr; }
	while(itr->next != NULL) {
		prev = itr;
		itr = itr->next;
	}
	if(prev == NULL) iroot = itr->next;
	else prev->next = itr->next;
	itr->next = NULL;
	return itr;
}

ins_node * sp_get_inode() {
	ins_node * iroot = sp_current_iroot();
	ins_node * itr = iroot;
	if(itr == NULL) return itr;
	while(itr->next != NULL) {
		itr = itr->next;
	}
	return itr;
}

uint8 g_arrLen;
uint8 g_arrBuffer[256];
void sp_new_array_constant(void) {
	g_arrLen = 0;
}

void sp_push_array_constant(uchar * constname) {
	int val = atoi((const char *)constname);
	g_arrBuffer[g_arrLen++] = (uchar)val;
}

symrec * sp_lz_load_array_constant(void) {
	return sp_lz_load_array(g_arrBuffer, g_arrLen);
}

static pk_class * _sp_prog_class = NULL;
pk_class * sp_start_class_body(uchar * name) {
	_sp_prog_class = pk_register_class(name);
	return _sp_prog_class;
}

void sp_end_class_body(void) {
	_sp_prog_class = NULL;
}

pk_class * sp_start_interface_body(uchar * name, uchar * libname) {
	_sp_prog_class = (pk_class *)pk_register_interface(name, libname);
	return _sp_prog_class;
}

void sp_end_interface_body(void) {
	_sp_prog_class = NULL;
}

pk_method * sp_register_method(symrec * rec, uint8 call_type, uint8 ret_type) {
	pk_method * meth = NULL;
	//char fname[256];
	if(_sp_prog_class != NULL) { 
		//sprintf(fname, "%s?%d", rec->name, rec->ptotal & 0x7F);
		meth = pk_register_method(_sp_prog_class, rec->name, rec->ptotal & 0x7F, call_type, ret_type, _current_offset, NULL);
		memcpy(meth->param_types, rec->param_types, rec->ptotal & 0x7F);
	}
	return meth;
}

symrec * sp_create_label(uchar * prefix) {					//create new label with prefix for backpatching purpose
	uchar lblname[256];
	symrec * newlbl = NULL;
	sprintf(_RECAST(char *,lblname), "%s%i", prefix, _sp_label_index++);
	//printf("create label %s\n", lblname);
	newlbl = st_sym_select(SYM_TYPE_LABEL, lblname);
	if(newlbl == NULL) {
		newlbl = st_sym_insert(SYM_TYPE_LABEL, lblname, SYM_UNDEFINED);
	}
	newlbl->parent = _sp_current_function;						//assign label parent, for removal
	return newlbl;
}

symrec * sp_create_variable(uchar * varname) {					//create new label with prefix for backpatching purpose
	uchar lblname[256];
	symrec * newvar = NULL;
	newvar = st_sym_select(SYM_TYPE_VAR, varname);
	if(newvar == NULL) {
#ifdef SP_MAX_VARS
#if SP_MAX_VARS != 0
		if(_sp_context_cntr >= SP_MAX_VARS) {	
			sp_error(SP_ERR_MAXIMUM_VARIABLES, SP_MAX_VARS-1, varname);
		}
#endif
#endif
		//printf("create variable %s\n", varname);
		newvar = st_sym_insert(SYM_TYPE_VAR, varname, _sp_context_cntr);
		_sp_context_cntr++;
	}
	newvar->parent = _sp_current_function;						//assign variable parent, for removal
	return newvar;
}

symrec * sp_create_param(uchar * varname) {					//create new label with prefix for backpatching purpose
	uchar lblname[256];
	symrec * newvar = NULL;
	newvar = st_sym_select(SYM_TYPE_VAR, varname);
	if(newvar == NULL) {
		//printf("create variable %s\n", varname);
		newvar = st_sym_insert(SYM_TYPE_VAR, varname, _sp_context_cntr);
		_sp_context_cntr++;
	}
	if(_sp_current_function != NULL) {
		newvar->parent = _sp_current_function;						//assign variable parent, for removal
		if((_sp_current_function->ptotal & 0x80) == 0) {		//check if function count is locked
			_sp_current_function->pcount ++;
		}
	}
	return newvar;
}

symrec * sp_create_function(uchar * funcname) {					//create new label with prefix for backpatching purpose
	uchar lblname[256];
	symrec * newfunc = NULL;
	newfunc = st_sym_select(SYM_TYPE_FUNCTION, funcname);			//function prototype already declared
	if(newfunc != NULL) {
		//newfunc->offset = _current_offset;
	} else {
		//printf("create function %s\n", funcname);
		newfunc = st_sym_create(SYM_TYPE_FUNCTION, funcname, SYM_UNDEFINED);
	}
	_sp_current_function = newfunc;
	_sp_context_cntr = 0;
	return newfunc;
}

symrec * sp_declare_interface_function(uchar * funcname) {
	uint32 offset;
	symrec * func = st_sym_select(SYM_TYPE_FUNCTION, funcname);			//function prototype already declared
	if(func == NULL) {
		//printf("pushed constant : %s\n", funcname);
		offset = sp_push_constant_s(funcname, strlen((const char *)funcname)); 
		func = sp_create_function(funcname);			//create function and add to symbol table
		st_sym_add_record(func);
		func->type |= SYM_TYPE_CONST;
		func->offset = offset;
	}
	_sp_current_function = func;
	return func;
}

symrec * sp_declare_function(uchar * funcname) {
	symrec * func = st_sym_select(SYM_TYPE_FUNCTION, funcname);			//function prototype already declared
	if(func == NULL) {
		func = sp_create_function(funcname);			//create function and add to symbol table
		st_sym_add_record(func);
	}
	_sp_current_function = func;
	return func;
}

void sp_declare_function_param(uint8 type) {
	if(_sp_current_function != NULL) {
		if((_sp_current_function->ptotal & 0x80) == 0) {		//check if function count is locked
			_sp_current_function->param_types[_sp_current_function->ptotal] = type;
			_sp_current_function->ptotal ++;
		}
	}
}

void sp_declare_function_extern(uchar * apinum) {
	uint32 api_id = atoi(_RECAST(const char *,apinum));
	if(_sp_current_function != NULL) {
		_sp_current_function->type |= SYM_TYPE_EXTERNAL;
		_sp_current_function->offset = api_id;		//-> external api
	}
}

symrec * sp_declare_function_end(uchar * funcname) {
	uchar buffer[256];
	symrec * func = NULL;
	func = st_sym_select(SYM_TYPE_FUNCTION, funcname);
	if(func != NULL) {
		sprintf(_RECAST(char *, buffer), "%s?%i", _sp_current_function->name, _sp_current_function->ptotal);
		if(st_sym_select(SYM_TYPE_FUNCTION, buffer) != NULL) {
			sp_error(SP_ERR_MULTI_FUNCTION, buffer);
			return NULL;
		}
		_sp_current_function->ptotal |= 0x80;		//lock function count
		_sp_current_function = NULL;
	}
	return func;
}

//public api
void sp_install_api(uchar * funcname, uint16 api_id, uint8 num_args) {
	symrec * func = st_sym_select(SYM_TYPE_FUNCTION, funcname);			//function prototype already declared
	if(func == NULL) {
		func = sp_create_function(funcname);			//create function and add to symbol table
		st_sym_add_record(func);
	}
	func->type |= SYM_TYPE_EXTERNAL;
	func->offset = api_id;		//-> external api
	func->ptotal = 0x80 | num_args;		//lock function count
}

symrec * sp_start_function_body(symrec * func, uchar is_public) {
	uchar funcname[256];
	lblrec * lbl;
	jmprec * itr;
	symrec * newfunc = NULL;
	sprintf(_RECAST(char *, funcname), "%s?%i", func->name, _sp_context_cntr); 
	newfunc = st_sym_select(SYM_TYPE_FUNCTION, funcname);			//function prototype already declared
	if(newfunc != NULL) {											//relase temp function record, use function record from symtable
		if(newfunc->offset == SYM_UNDEFINED) {
			st_sym_update_parent(func, newfunc);
			st_sym_dispose(func);
			newfunc->offset = _current_offset;
			_sp_current_function = newfunc;									//local variable/label anchor
			itr = (jmprec *)newfunc->list;								//search queued list for backpatching
			while(itr != NULL) {
				is_gencode(itr->offset, itr->ins, newfunc);
				itr = itr->next;
			}
			st_jmp_clear(newfunc);
		}  else {
			//function already defined
			//printf(_RECAST(char *, SP_ERR_MULTI_FUNCTION), funcname);
			fflush(0);
			sp_error(SP_ERR_MULTI_FUNCTION, func->name);
			return NULL;
		} 
	} else {
		//printf("create function %s\n", funcname);
		//newfunc = st_sym_insert(SYM_TYPE_FUNCTION, funcname, _current_offset);
		//fflush(0);
		if(st_sym_select(SYM_TYPE_FUNCTION, func->name) != NULL) {
			sp_error(SP_ERR_MULTI_FUNCTION, func->name);
			return NULL;
		}
		newfunc = st_sym_add_record(func);
		//printf("register method args : %i\n", func->pcount & 0x7F);
		func->ptotal = func->pcount;			//set function numargs
		func->ptotal |= 0x80;					//lock function numargs
		sprintf(_RECAST(char *, func->name), "%s", funcname);
		func->length = strlen(_RECAST(const char *, funcname));
		_sp_current_function = newfunc;									//local variable/label anchor
	}
	_sp_end_function_body = sp_create_label(_RECAST(uchar *,"__n"));								//register function
	//_sp_context_cntr = 0;											//reset context counter
	//sp_create_new_scope();
	_current_offset += is_gencode(_current_offset, INS_LBL, newfunc);			//generate function label
	if(is_public) {
		lbl = is_get_lblrec(_current_offset);										//get current function label
		//register method rec, assign method rec to label->tag for future reference during optimization
		//printf("register method args : %i\n", newfunc->ptotal & 0x7F);
		lbl->tag = sp_register_method(newfunc, 0, 0);		
	}

	newfunc->offset = _current_offset;
	_current_offset += is_gencode(_current_offset, INS_SCTX, 0);
	sp_create_variable(funcname);
	return newfunc;
}

uint32 sp_exit_function_body(void) {
	uint32 offset;
	if(_sp_end_function_body != NULL) {
		offset = sp_jump_to(INS_JMP, _sp_end_function_body->name);
	}
	return offset;
}

uint32 sp_return_function_body(void) {
	uint32 offset = 0;
	symrec * retvar = NULL;
	//printf("return function body\n");
	if(_sp_end_function_body != NULL) {
		if(_sp_current_function == NULL) return offset;
		retvar = st_sym_select(SYM_TYPE_VAR, _sp_current_function->name);
		//if(retvar == NULL) printf("invalid retvar %s\n", _sp_current_function->name);
		//printf("return function body %s\n", retvar->name);

		sp_store_stack(retvar->name);
		offset = sp_jump_to(INS_JMP, _sp_end_function_body->name);
	}
	return offset;
}

uint32 sp_end_function_body(symrec * func) {
	uchar lblname[256];
	//printf("end function body\n");
	if(func == NULL) return _current_offset;
	if(func != NULL) {
		is_gencode(func->offset, INS_SCTX, _sp_context_cntr);
	}
	if(_sp_end_function_body != NULL) {
		sp_new_label(_sp_end_function_body->name);
		_sp_end_function_body = NULL;
	}
	sp_destroy_scope();
	sm_printf("return variable : %s\n", func->name);
	sp_load_stack(func->name);							//load return value
	sp_flush_scope_now();
	_current_offset += is_gencode(_current_offset, INS_RCTX, _sp_context_cntr);
	_current_offset += is_gencode(_current_offset, INS_RET);
	//printf("clear locals\n");
	st_sym_clear_locals(func);
	_sp_context_cntr = 0;
	_sp_current_function->ptotal |= 0x80;		//lock function count
	_sp_current_function = NULL;
	//printf("end function body\n");
	return _current_offset;
}

symrec * sp_start_api_call(uint16 api_id, uchar icount) {
	uchar buffer[15];
	symrec * func;
	static int id = 0;
	sp_push_varctx_scope();
	sprintf((char *)buffer, "_f%d_%d", api_id, id++);
	func = st_sym_select(SYM_TYPE_FUNCTION, buffer);		//if this is system call then it should be declared here
	if(func != NULL) {				//function already defined
		func->pcount = icount;
		return func;
	}
	//printf("create new function\n");
	//create new function but don't add it to symbol table yet
	func = st_sym_create(SYM_TYPE_FUNCTION, buffer, SYM_UNDEFINED);
	func->pcount = icount;
	if(func == NULL) return func;
	st_sym_add_record(func);
	func->type |= SYM_TYPE_EXTERNAL;
	func->offset = api_id;		//-> external api
	//add to symbol table for future reference
	//st_sym_add_record(func);
	return func;
}

symrec * sp_start_function_call(uchar * funcname, uchar icount) {
	symrec * func;
	symrec * fvar;
	//check for object call
	fvar = st_sym_select(SYM_TYPE_VAR, funcname);			
	if(fvar != NULL) {		//check for system function
		sp_push_inode(sp_create_inode(INS_OBJPUSH, fvar));
		sp_push_varctx_scope();
		return fvar;
	}

	func = st_sym_select(SYM_TYPE_FUNCTION, funcname);		//if this is system call then it should be declared here
	if(func != NULL) {				//function already defined
		func->pcount = icount;
		sp_push_varctx_scope();
		return func;
	}
	//printf("create new function\n");
	//create new function but don't add it to symbol table yet
	func = st_sym_create(SYM_TYPE_FUNCTION, funcname, SYM_UNDEFINED);
	func->pcount = icount;
	sp_push_varctx_scope();
	return func;
}

uint32 sp_push_function_param(symrec * funcrec) {
	if(funcrec != NULL) {
		//printf("func : %s\n", funcrec->name);
		funcrec->pcount++;
	}
	sp_pop_varctx_scope();
	sp_push_varctx_scope();
	return _current_offset;
}

symrec * sp_end_function_call(symrec * rec) {
	uchar funcname[256];
	ins_node * temp;
	symrec * frec = NULL;
	//printf("end func call\n");
	//check for system call
	frec = st_sym_select(SYM_TYPE_FUNCTION, rec->name);			
	if(frec != NULL && frec->type & SYM_TYPE_EXTERNAL) {		//check for system function
		sp_pop_varctx_scope();
		sp_push_inode(sp_create_inode(INS_SYSCALL + frec->pcount, (symrec *)(frec->offset & 0xFFFF)));
		return frec;
	}

	//lambda execution
	if(rec->type & SYM_TYPE_VAR) {
		sp_pop_varctx_scope();
		sp_push_inode(sp_create_inode(INS_SYSCALL + (rec->pcount + 1), (symrec *)31));
		return rec;
	}

	sprintf(_RECAST(char *,funcname), "%s?%i", rec->name, rec->pcount);
	frec = st_sym_select(SYM_TYPE_FUNCTION, funcname);		//check if this function already added to symbol table
	if(frec != NULL) {										//variable not found
		
		if(((frec->ptotal & 0x80) != 0) && rec->pcount < (frec->ptotal & 0x7F)) sp_error(SP_ERR_LESS_ARGUMENTS, frec->name);
		if(((frec->ptotal & 0x80) != 0) && rec->pcount > (frec->ptotal & 0x7F)) sp_error(SP_ERR_OVER_ARGUMENTS, frec->name);
		//_current_offset += sp_gencode(_current_offset, INS_CALL, rec);
		sp_push_inode(sp_create_inode(INS_CALL, frec));
		st_sym_dispose(rec);
	} else {
		//reserve the address location for backpatching
		//frec = st_sym_add_record(rec);		//add this function to symbol table
		frec = rec;
		frec->ptotal = frec->pcount;		//set function numargs
		frec->ptotal |= 0x80;			//lock function numargs
		sprintf(_RECAST(char *,frec->name), "%s", funcname);
		frec->length = strlen(_RECAST(const char *, funcname));
		sp_push_inode(sp_create_inode(INS_CALL, frec));
	}
	sp_pop_varctx_scope();
	return frec;
}

uint32 sp_jump_to(uchar ins, uchar * label) {				//jump to specified label, if not exist then reserved address for later backpatch
	symrec * rec = st_sym_select(SYM_TYPE_LABEL, label);
	if(rec == NULL) {										//variable not found
		rec = st_sym_insert(SYM_TYPE_LABEL, label, SYM_UNDEFINED);
		if(rec == NULL) return _current_offset;
	}
	if(rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
		st_jmp_add(rec, ins, _current_offset);
	} 
	_current_offset += is_gencode(_current_offset, ins, rec);
	return _current_offset;
}

uint32 sp_new_label(uchar * label) {						//create new label, apply backpatch here
	jmprec * itr = NULL;
	symrec * rec = st_sym_select(SYM_TYPE_LABEL, label);
	if(rec != NULL) {
		if(rec->offset == SYM_UNDEFINED) {
			//printf("undefined %08x\n", _current_offset);
			rec->offset = _current_offset;
		} else {
			//printf("Multiple reference to label\n");
			sp_error(SP_ERR_MULTI_LABEL, label);
			return _current_offset;
		}
	} else {
		rec = st_sym_insert(SYM_TYPE_LABEL, label, _current_offset);
	}
	rec->parent = _sp_current_function;						//assign variable parent, for removal
	_current_offset += is_gencode(_current_offset, INS_LBL, rec);
	itr = (jmprec *)rec->list;								//search queued list for backpatching
	while(itr != NULL) {
		is_gencode(itr->offset, itr->ins, rec);
		itr = itr->next;
	}
	st_jmp_clear(rec);									//release queue
	return _current_offset;
}

uint32 sp_load_stack(uchar * varname) {						//load variable/constant to stack
	symrec * rec = st_sym_select(SYM_TYPE_VAR, varname);
	if(rec != NULL) {			//variable not found
		printf("var load stack : %s\n", rec->name);
		_current_offset += sp_gencode(_current_offset, INS_OBJPUSH, rec);
		return _current_offset;
	}
	//printf("Undeclared indentifier %s\n", varname);
	sp_error(SP_ERR_UNDEF_VARIABLE, varname);
	return _current_offset;
}

uint32 sp_load_constant(uchar * constname) {						//load variable/constant to stack
	symrec * rec = st_sym_select(SYM_TYPE_CONST, constname);
	//printf("load constant %s\n", constname);
	if(rec == NULL) {
		sp_push_constant(constname);
		rec = st_sym_select(SYM_TYPE_CONST, constname);
	}	
	if(rec != NULL) {
		_current_offset += sp_gencode(_current_offset, INS_OBJCONST, rec);
		return _current_offset;
	}
	//printf("Undeclared indentifier %s\n", varname);
	sp_error(SP_ERR_UNDEF_VARIABLE, constname);
	return _current_offset;
}

uint32 sp_load_numeric(uchar * constname) {						//load variable/constant to stack
	symrec * rec = st_sym_select(SYM_TYPE_CONST, constname);
	//printf("load constant %s\n", constname);
	if(rec == NULL) {
		sp_push_constant(constname);
		rec = st_sym_select(SYM_TYPE_CONST, constname);
	}	
	if(rec != NULL) {
		_current_offset += sp_gencode(_current_offset, INS_OBJCONST, rec);
		_current_offset += sp_gencode(_current_offset, INS_SYSCALL1, (symrec *)40);
		return _current_offset;
	}
	//printf("Undeclared indentifier %s\n", varname);
	sp_error(SP_ERR_UNDEF_VARIABLE, constname);
	return _current_offset;
}

uint32 sp_load_array(uchar * arrval, uint8 length) {						//load variable/constant to stack
	symrec * rec = st_sym_select_array(arrval, length);
	if(rec == NULL) {
		sp_push_array(arrval, length);
		rec = st_sym_select_array(arrval, length);
	}	
	if(rec != NULL) {
		_current_offset += sp_gencode(_current_offset, INS_OBJCONST, rec);
		return _current_offset;
	}
	//printf("Undeclared indentifier %s\n", varname);
	sp_error(SP_ERR_UNDEF_VARIABLE, arrval);
	return _current_offset;
}

uint32 sp_store_stack(uchar * varname) {							//store stack to variable
	symrec * rec;
	rec = st_sym_select(SYM_TYPE_VAR, varname);
	if(rec != NULL) {										//variable not found
		_current_offset += sp_gencode(_current_offset, INS_OBJPOP, rec);
		return _current_offset;
	}
	//printf("Undeclared indentifier %s\n", varname);
	sp_error(SP_ERR_UNDEF_VARIABLE, varname);
	return _current_offset;
}

uint32 sp_operation_stack(uchar ins) {
	_current_offset += sp_gencode(_current_offset, ins);
	return _current_offset;
}


static symrec * _sp_loop_stack[SEM_MAX_STACK];
static int32 _sp_loop_stack_index = 0;
static void sp_loop_stack_push(symrec * label) { _sp_loop_stack[_sp_loop_stack_index++] = label; }
static symrec * sp_loop_stack_pop() { return _sp_loop_stack[--_sp_loop_stack_index]; }
static symrec * sp_loop_stack_peek() { return _sp_loop_stack[_sp_loop_stack_index - 1]; }

symrec * sp_start_loop(void) {
	symrec * _sp_loop_label = sp_create_label(_RECAST(uchar *,"__l"));
	sp_new_label(_sp_loop_label->name);
	sp_loop_stack_push(_sp_loop_label);
	return _sp_loop_label;
}

symrec * sp_start_each_loop(symrec * var) {
	symrec * _sp_loop_label = sp_create_label(_RECAST(uchar *,"__l"));
	//call .first() here
	//printf("%s %s\n", var->name, exit_label->name);
	sp_push_inode(sp_create_inode(INS_SYSCALL1, (symrec *)28));
	sp_flush_scope_now(); 
	sp_new_label(_sp_loop_label->name);
	sp_loop_stack_push(_sp_loop_label);
	sp_push_inode(sp_create_inode(INS_OBJDUP, NULL));
	sp_push_inode(sp_create_inode(INS_SYSCALL1, (symrec *)29));
	sp_push_inode(sp_create_inode(INS_OBJDUP, NULL));
	sp_push_inode(sp_create_inode(INS_OBJPOP, var));
	sp_lz_load_constant(_RECAST(uchar *, ""));
	sp_push_inode(sp_create_inode(INS_CRNE, NULL));
	return _sp_loop_label;
}

uint32 sp_continue_loop(void) {
	symrec * _sp_loop_label = sp_loop_stack_peek();
	if(_sp_loop_label != NULL) {
		_current_offset = sp_jump_to(INS_JMP, _sp_loop_label->name);
	}
	return _current_offset;
}

uint32 sp_end_loop(void) {
	symrec * _sp_loop_label = sp_loop_stack_peek();
	if(_sp_loop_label != NULL) {
		_current_offset = sp_jump_to(INS_JMP, _sp_loop_label->name);
		_sp_loop_label = NULL;
		sp_loop_stack_pop();
	}
	return _current_offset;
}

uint32 sp_end_each_loop(void) {
	symrec * _sp_loop_label = sp_loop_stack_peek();
	if(_sp_loop_label != NULL) {
		_current_offset = sp_jump_to(INS_JMP, _sp_loop_label->name);
		_sp_loop_label = NULL;
		sp_loop_stack_pop();
	}
	//delete previously pushed enumerator (.first)
	sp_push_inode(sp_create_inode(INS_OBJDEL, NULL));
	return _current_offset;
}

uint32 sp_push_constant(uchar * symname) {
	symrec * var = NULL;
	uint32 offset = 0;
	var = st_sym_select(SYM_TYPE_CONST, symname);
	if(var == NULL) {
		offset = is_push_constant(strlen(_RECAST(const char *,symname)), symname);
		var = st_sym_insert(SYM_TYPE_CONST, symname, offset);
	}
	return offset;
}

uint32 sp_push_constant_s(uchar * symname, uint16 length) {
	symrec * var = NULL;
	uint32 offset = 0;
	var = st_sym_select_s(SYM_TYPE_CONST, symname, length);
	if(var == NULL) {
		offset = is_push_constant(length, symname);
		var = st_sym_insert_s(SYM_TYPE_CONST, symname, length, offset);
	}
	return offset;
}

uint32 sp_push_array(uchar * arrval, uint16 length) {
	symrec * var = NULL;
	uint32 offset = 0;
	var = st_sym_select_array(arrval, length);
	if(var == NULL) {
		offset = is_push_constant(length, arrval);
		var = st_sym_insert_array(arrval, length, offset);
	}
	return offset;
}

symrec * sp_push_constant_offset(uint16 c_offset) {
	symrec * var = NULL;
	uint32 offset = 0;
	uchar sbuffer[10];
	//uchar buffer[2] = { (c_offset >> 8), (c_offset & 0xFF) };
	sprintf(_RECAST(char *,sbuffer), "%d", c_offset);
	var = st_sym_select(SYM_TYPE_CONST, sbuffer);
	if(var == NULL) {
		offset = is_push_constant(strlen(_RECAST(const char *,sbuffer)), sbuffer);
		var = st_sym_insert(SYM_TYPE_CONST, sbuffer, offset);
	}
	return var;
}

uint32 sp_push_class(uchar * symname) {
	symrec * var = NULL;
	uint32 offset = 0;
	var = st_sym_select(SYM_TYPE_CLASS, symname);
	if(var == NULL) {
		offset = is_push_constant(strlen(_RECAST(const char *,symname)), symname);
		var = st_sym_insert(SYM_TYPE_CLASS, symname, offset);
	}
	return offset;
}


static uchar _sp_case_constants[514];
symrec * sp_start_case() {
	uchar buffer[256];
	symrec * jumptable;
	memset(buffer, 0, sizeof(buffer));
	jumptable = st_sym_insert(SYM_TYPE_CONST, buffer, 0);
	jumptable->length = sizeof(uint16);
	*((uint16 *)jumptable->name) = 0x0000;
	jumptable->parent = _sp_current_function;	
	st_jmp_add(jumptable, INS_SWITCH, _current_offset);
	_current_offset += is_gencode(_current_offset, INS_SWITCH, jumptable);
	return jumptable;
}

uint32 sp_label_case_s(symrec * jumptable, uchar * name, uint16 length) {
	symrec * rec = st_sym_select_s(SYM_TYPE_CONST, name, length);
	uint32 offlabel = rec->offset;
	uint32 cur_offset = jumptable->length;
	*((uint16 *)(jumptable->name + cur_offset)) = end_swap16(offlabel);
	*((uint16 *)(jumptable->name + cur_offset + 2)) = end_swap16(_current_offset);
	jumptable->length += 4;
	return _current_offset;
}

uint32 sp_label_case(symrec * jumptable, uchar * name) {
	return sp_label_case_s(jumptable, name, strlen(_RECAST(const char *, name)));
}

uint32 sp_default_case(symrec * jumptable) {
	*((uint16 *)jumptable->name) = end_swap16((uint16)_current_offset);
	return _current_offset;
}

uint32 sp_end_case(symrec * jumptable) {
	jmprec * itr = NULL;
	uint32 jtable_offset = 0;
	if(*((uint16 *)jumptable->name) == 0) {			//case default didn't defined
		*((uint16 *)jumptable->name) = end_swap16((uint16)_current_offset);
	}
	jtable_offset = is_push_constant(jumptable->length, jumptable->name) ;
	jumptable->offset = jtable_offset;
	itr = (jmprec *)jumptable->list;								//search queued list for backpatching
	while(itr != NULL) {
		is_gencode(itr->offset, itr->ins, jumptable);
		itr = itr->next;
	}
	st_jmp_clear(jumptable);
	return _current_offset;	
}

void sp_cleanup_parser(void) {
	st_sym_clear_globals();
}

static uchar _sp_construct_buffer[256];
uint16 _sp_construct_index = 0;
void sp_start_construct() {
	_sp_construct_index = 0;
	memset(_sp_construct_buffer, 0, sizeof(_sp_construct_buffer));
}

void sp_push_construct(uchar * value) {
	sprintf(_RECAST(char *,_sp_construct_buffer + _sp_construct_index), "%s", value);
	_sp_construct_index += strlen(_RECAST(const char *,value));
}

uchar * sp_end_construct() {
	return _sp_construct_buffer;
}

symrec * sp_lz_load_stack(symrec * rec) {						//load variable/constant to stack
	if(rec == NULL) return rec;
	switch(rec->type & 0x0F) {
		case SYM_TYPE_VAR: sp_push_inode(sp_create_inode(INS_OBJPUSH, rec)); break;
		case SYM_TYPE_CONST: sp_push_inode(sp_create_inode(INS_OBJCONST, rec)); break;
		default: break;
	}
	return rec;
}

symrec * sp_lz_load_constant(uchar * constname) {
	symrec * srec; 
	fflush(0);
	srec = st_sym_select(SYM_TYPE_CONST, constname);
	if(srec == NULL) { sp_push_constant(constname); srec = st_sym_select(SYM_TYPE_CONST, constname); }
	sp_push_inode(sp_create_inode(INS_OBJCONST, srec));
	return srec;
}

symrec * sp_lz_load_numeric(uchar * constname) {
	symrec * srec; 
	fflush(0);
	srec = st_sym_select(SYM_TYPE_CONST, constname);
	if(srec == NULL) { sp_push_constant(constname); srec = st_sym_select(SYM_TYPE_CONST, constname); }
	sp_push_inode(sp_create_inode(INS_OBJCONST, srec));
	sp_push_inode(sp_create_inode(INS_SYSCALL1, (symrec *)40));
	return srec;
}

symrec * sp_lz_load_constant_s(uchar * constname, uint16 length) {
	symrec * srec; 
	fflush(0);
	srec = st_sym_select_s(SYM_TYPE_CONST, constname, length);
	if(srec == NULL) { sp_push_constant_s(constname, length); srec = st_sym_select_s(SYM_TYPE_CONST, constname, length); }
	//if(srec == NULL) printf("no record %s %d\n", constname, length);
	sp_push_inode(sp_create_inode(INS_OBJCONST, srec));
	return srec;
}

symrec * sp_lz_load_array(uchar * arrval, uint16 length) {
	symrec * srec; 
	fflush(0);
	srec = st_sym_select_array(arrval, length);
	if(srec == NULL) { sp_push_array(arrval, length); srec = st_sym_select_array(arrval, length); }
	sp_push_inode(sp_create_inode(INS_OBJCONST, srec)); 
	return srec;
}

symrec * sp_lz_create_instance(uchar * constname) {
	symrec * srec = NULL; 
	uint32 offset;
	pk_object * cls;
	fflush(0);
	cls = (pk_object *)pk_select_class(pk_get_root(), constname);
	if(cls != NULL) {
		switch(((pk_object *)cls)->tag) {
			case PK_TAG_CLASS:
				srec = st_sym_select(SYM_TYPE_CONST, constname);
				if(srec == NULL) { 
					offset = sp_push_constant(constname); 
					srec = st_sym_select(SYM_TYPE_CONST, constname); 
					srec->offset = offset;
				}
				sp_push_inode(sp_create_inode(INS_OBJNEW, srec)); 
				break;
			case PK_TAG_INTERFACE:
				srec = st_sym_select(SYM_TYPE_CONST, constname);
				if(srec == NULL) { 
					offset = sp_push_constant(constname); 
					srec = st_sym_select(SYM_TYPE_CONST, constname); 
					srec->offset = offset;
				}
				sp_push_inode(sp_create_inode(INS_OBJCONST, srec));
				//system call dl_open
				sp_push_inode(sp_create_inode(INS_SYSCALL1, (symrec *)36));
				break;
			default: break;
		}
	
	}
	return srec;
}

symrec * sp_lz_load_method(uchar * funcname) {
	uchar fbuffer[64];
	symrec * func;
	pk_class * pkc;
	sprintf(_RECAST(char *,fbuffer), "%s?%i", funcname, 0);
	//if this is system call then it should be declared here
	func = st_sym_select(SYM_TYPE_FUNCTION, fbuffer);
	if(func == NULL) {				//function already defined
		func = st_sym_create(SYM_TYPE_FUNCTION, fbuffer, SYM_UNDEFINED);
		func = st_sym_add_record(func);
	}
	if(func != NULL) {										//variable not found
		sp_push_inode(sp_create_inode((INS_F2O + (func->ptotal & 0x0F)), func));
	}
	return func;
}

symrec * sp_lz_load_lambda(symrec * lambda) {
	if(lambda != NULL) {										//variable not found
		//printf("push %s\n", lambda->name);
		sp_push_inode(sp_create_inode((INS_F2O + (lambda->ptotal & 0x0F)), lambda));
	}
	lambda->type |= SYM_TYPE_VAR;
	return lambda;
}

symrec * sp_lz_load_variable(uchar * varname) {
	symrec * rec;	
	if(varname == NULL) return NULL;	//{ sp_error(SP_ERR_UNDEF_VARIABLE, varname); return NULL; }
	rec = st_sym_select(SYM_TYPE_VAR, varname); 
	if(rec != NULL) { sp_push_inode(sp_create_inode(INS_OBJPUSH, rec)); return rec; }
	if(rec == NULL) return sp_lz_load_method(varname);//try load method if no variable found
	return rec;
}

symrec * sp_lz_store_variable(uchar * varname) {
	symrec * rec = st_sym_select(SYM_TYPE_VAR, varname); 
	if(rec == NULL) { sp_error(SP_ERR_UNDEF_VARIABLE, varname); return NULL; }
	sp_push_inode(sp_create_inode(INS_OBJPOP, rec));
	return rec;
}

void sp_push_operation_stack(ins_node * inode) {
	_op_stack[_op_index++] = inode;
}

void sp_push_operation(uint16 api_id, uint8 num_args) {
	//push current API call into operation stack
	sp_push_operation_stack(sp_create_inode(INS_SYSCALL + (num_args & 0x0F), (symrec *)(api_id)));
}

void sp_clear_operation_stack() {
	_op_index = 0;
	sm_printf("symstack : %d, op : %d\n", _sp_symstack_index, _op_index);
}

void sp_end_expr(symrec * rec) {
	ins_node * inode;
	if(rec == NULL) return;
	if(_op_index != 0) {
		//if(_op_index > 1) { sp_error(SP_ERR_NESTED_ACCESS, ""); return; }
		while(_op_index != 0) {
			inode = _op_stack[--_op_index];
			//if(inode->rec->type & SYM_TYPE_VAR)
				sp_push_inode(inode);
		}
		if(inode->ins != INS_OBJPOP) sp_push_inode(sp_create_inode(INS_OBJDEL, NULL));
		//return;
		return;
	}
	if(rec->type & SYM_TYPE_VAR) sp_push_inode(sp_create_inode(INS_OBJPOP, rec));
	else sp_push_inode(sp_create_inode(INS_OBJDEL, NULL));
}

symrec * sp_lz_constant_after_scope(uchar * dstname, uchar * constname, uchar ins) {
	//current scope
	symrec * rec;	// = sp_lz_load_variable(dstname);
	ins_node * temp;
	//ins_node * temp2;
	//if(rec == NULL) { sp_error(SP_ERR_UNDEF_VARIABLE, dstname); return NULL; }
	temp = sp_pop_iroot();
	//temp2 = sp_pop_iroot();
	//after scope
	rec = sp_lz_load_variable(dstname);
	sp_lz_load_constant(constname);
	sp_push_inode(sp_create_inode(ins, NULL)); 
	if(rec->type & SYM_TYPE_VAR) sp_push_inode(sp_create_inode(INS_OBJPOP, rec));
	else sp_push_inode(sp_create_inode(INS_OBJDEL, rec));
	//sp_push_iroot(temp2);
	sp_push_iroot(temp);
	//current scope
	return rec;
}

symrec * sp_lz_array_after_scope(uchar * dstname, uchar * arrval, uint8 length, uchar ins) {
	ins_node * temp;
	//current scope
	symrec * rec;	// = sp_lz_load_variable(dstname);
	//if(rec == NULL) { sp_error(SP_ERR_UNDEF_VARIABLE, dstname); return NULL; }
	temp = sp_pop_iroot();
	//after scope
	rec = sp_lz_load_variable(dstname);
	sp_lz_load_array(arrval, length);
	sp_push_inode(sp_create_inode(ins, NULL)); 
	if(rec->type & SYM_TYPE_VAR) sp_push_inode(sp_create_inode(INS_OBJPOP, rec));
	else sp_push_inode(sp_create_inode(INS_OBJDEL, rec));
	sp_push_iroot(temp);
	//current scope
	return rec;
}

symrec * sp_lz_variable_after_scope(uchar * dstname, uchar * srcname, uchar ins) {
	symrec * srec;
	ins_node * temp;
	//current scope
	symrec * rec;// = sp_lz_load_variable(dstname); 
	//if(rec == NULL) { sp_error(SP_ERR_UNDEF_VARIABLE, dstname); return NULL; }
	temp = sp_pop_iroot();
	//after scope
	rec = sp_lz_load_variable(dstname);
	srec = sp_lz_load_variable(srcname);
	if(srec == NULL) { sp_error(SP_ERR_UNDEF_VARIABLE, srcname); return NULL; } 

	sp_push_inode(sp_create_inode(ins, NULL)); 
	if(rec->type & SYM_TYPE_VAR) sp_push_inode(sp_create_inode(INS_OBJPOP, rec));
	else sp_push_inode(sp_create_inode(INS_OBJDEL, rec));
	sp_push_iroot(temp);
	return rec;
}

void sp_clear_current_scope() {
	ins_node * inode = NULL;
	ins_node * root = sp_current_iroot();
	if(root != NULL) {
		inode = root->next;
		while(inode != NULL) {
			root->next = inode->next;
			memset(inode, 0, sizeof(ins_node));
			free(inode);
			inode = root->next;
		}
	}
}

void sp_clone_scope() {
	ins_node * root = sp_current_iroot();
	ins_node * root_after = sp_right_iroot();
	ins_node * inode = NULL;
	//flush current scope
	sm_printf("sp_clone_scope\n");
	if(root != NULL) {
		inode = root->next;
		while(inode != NULL) {
			printf("ins : %s\n", _ins_name[inode->ins]);
			switch(inode->ins) {
				case INS_F2O_0:
				case INS_F2O_1:
				case INS_F2O_2:
				case INS_F2O_3:
				case INS_F2O_4:
				case INS_F2O_5:
				case INS_F2O_6:
				case INS_F2O_7:
				case INS_F2O_8:
				case INS_F2O_9:
				case INS_F2O_10:
				case INS_F2O_11:
				case INS_F2O_12:
				case INS_F2O_13:
				case INS_F2O_14:
				case INS_F2O_15:
					if(inode->rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
						st_jmp_add(inode->rec, inode->ins, _current_offset);
					}
					break;
				case INS_CALL:
					if(inode->rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
						st_jmp_add(inode->rec, INS_CALL, _current_offset);
					}
					break;
				default: break;
			}
			_current_offset += sp_gencode(_current_offset, inode->ins, inode->rec);
			//root->next = inode->next;
			//memset(inode, 0, sizeof(ins_node));
			//free(inode);
			inode = inode->next;
		}
	}
#if 0		//fixed when calling sp_lhs_get_s/sp_rhs_get/sp_lhs_get "scope after" automatically cloned (2019.07.06)
	//flush after scope
	if(root_after == NULL) return;
	inode = root_after->next;
	while(inode != NULL) {
		switch(inode->ins) {
			case INS_F2O_0:
			case INS_F2O_1:
			case INS_F2O_2:
			case INS_F2O_3:
			case INS_F2O_4:
			case INS_F2O_5:
			case INS_F2O_6:
			case INS_F2O_7:
			case INS_F2O_8:
			case INS_F2O_9:
			case INS_F2O_10:
			case INS_F2O_11:
			case INS_F2O_12:
			case INS_F2O_13:
			case INS_F2O_14:
			case INS_F2O_15:
				if(inode->rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
					st_jmp_add(inode->rec, inode->ins, _current_offset);
				}
				break;
			case INS_CALL:
				if(inode->rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
					st_jmp_add(inode->rec, INS_CALL, _current_offset);
				}
				break;
			default: break;
		}
		_current_offset += sp_gencode(_current_offset, inode->ins, inode->rec);
		//root_after->next = inode->next;
		//memset(inode, 0, sizeof(ins_node));
		//free(inode);
		inode = inode->next;
	}
#endif
}

void sp_flush_scope_now_s(ins_node * root) {
	ins_node * iterator;
	ins_node * inode = NULL;
	if(root != NULL) {
		inode = root->next;
		while(inode != NULL) {
			switch(inode->ins) {
				case INS_F2O_0:
				case INS_F2O_1:
				case INS_F2O_2:
				case INS_F2O_3:
				case INS_F2O_4:
				case INS_F2O_5:
				case INS_F2O_6:
				case INS_F2O_7:
				case INS_F2O_8:
				case INS_F2O_9:
				case INS_F2O_10:
				case INS_F2O_11:
				case INS_F2O_12:
				case INS_F2O_13:
				case INS_F2O_14:
				case INS_F2O_15:
					if(inode->rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
						st_jmp_add(inode->rec, inode->ins, _current_offset);
					}
					break;
				case INS_CALL:
					if(inode->rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
						st_jmp_add(inode->rec, INS_CALL, _current_offset);
					}
					break;
				default: break;
			}
			printf("ins : %s\n", _ins_name[inode->ins]);
			_current_offset += is_gencode(_current_offset, inode->ins, inode->rec);
			root->next = inode->next;
			memset(inode, 0, sizeof(ins_node));
			free(inode);
			inode = root->next;
		}
	}
}

void sp_flush_scope_now() {
	ins_node * root = sp_current_iroot();
	ins_node * root_after = sp_right_iroot();
	ins_node * iterator;
	ins_node * inode = NULL;
	//flush current scope
	if(root != NULL) {
		inode = root->next;
		while(inode != NULL) {
			switch(inode->ins) {
				case INS_F2O_0:
				case INS_F2O_1:
				case INS_F2O_2:
				case INS_F2O_3:
				case INS_F2O_4:
				case INS_F2O_5:
				case INS_F2O_6:
				case INS_F2O_7:
				case INS_F2O_8:
				case INS_F2O_9:
				case INS_F2O_10:
				case INS_F2O_11:
				case INS_F2O_12:
				case INS_F2O_13:
				case INS_F2O_14:
				case INS_F2O_15:
					if(inode->rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
						st_jmp_add(inode->rec, inode->ins, _current_offset);
					}
					break;
				case INS_CALL:
					if(inode->rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
						st_jmp_add(inode->rec, INS_CALL, _current_offset);
					}
					break;
				default: break;
			}
			printf("ins : %s\n", _ins_name[inode->ins]);
			_current_offset += is_gencode(_current_offset, inode->ins, inode->rec);
			root->next = inode->next;
			memset(inode, 0, sizeof(ins_node));
			free(inode);
			inode = root->next;
		}
	}
	//flush after scope
	if(root_after == NULL) return;
	inode = root_after->next;
	while(inode != NULL) {
		switch(inode->ins) {
			case INS_F2O_0:
			case INS_F2O_1:
			case INS_F2O_2:
			case INS_F2O_3:
			case INS_F2O_4:
			case INS_F2O_5:
			case INS_F2O_6:
			case INS_F2O_7:
			case INS_F2O_8:
			case INS_F2O_9:
			case INS_F2O_10:
			case INS_F2O_11:
			case INS_F2O_12:
			case INS_F2O_13:
			case INS_F2O_14:
			case INS_F2O_15:
				if(inode->rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
					st_jmp_add(inode->rec, inode->ins, _current_offset);
				}
				break;
			case INS_CALL:
				if(inode->rec->offset == SYM_UNDEFINED) {						//label not found, reserved address for backpatching later
					st_jmp_add(inode->rec, INS_CALL, _current_offset);
				}
				break;
			default: break;
		}
		printf("ins : %s\n", _ins_name[inode->ins]);
		_current_offset += is_gencode(_current_offset, inode->ins, inode->rec);
		root_after->next = inode->next;
		memset(inode, 0, sizeof(ins_node));
		free(inode);
		inode = root_after->next;
	}
}

void sp_flush_scope_s(ins_node * node) {
	ins_node * root = sp_current_iroot();
	ins_node * root_after = sp_right_iroot();
	ins_node * inode = NULL;
	ins_node * iterator;
	sm_printf("sp_flush_scope() : %d\n", _sp_inode_index);
	if(node == NULL) {		//no previous scope, start generating code
		//flush current scope
		sp_flush_scope_now();
		printf("sp_flush_scope_now\n"); //getchar();
	} else {
		//flush current scope to previous scope
		//printf("rerouting instruction nodes\n");
		iterator = node;
		while(iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = root->next;
		while(iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = root_after->next;
		root->next = NULL;
		root_after->next = NULL;
		//if(_sp_inode_index >= 1) _sp_inode_stack[_sp_inode_index - 1] = NULL;
		//if(_sp_inode_index >= 2) _sp_inode_stack[_sp_inode_index - 2] = NULL;
	}
	//printf("end flush scope\n");
}

void sp_flush_scope() {
	sp_flush_scope_s(sp_left_iroot());
}

//for flushing argument when there are chaining method such as x->y(a)->z(b) resulting in push b, push a, push x, xcall y, xcall z
ins_node * _sp_method_scope[SEM_MAX_STACK];
int sp_method_index = 0;
void sp_create_method_scope() {
	_sp_method_scope[sp_method_index++] = sp_create_inode(INS_LBL, sp_create_label(_RECAST(uchar *,"__p")));	//right scope;
}

void sp_destroy_method_scope() {
	ins_node * current = _sp_method_scope[--sp_method_index];
	_sp_method_scope[sp_method_index + 1] = NULL;
	if(current != NULL) {
		printf("--------------sp_flush_scope_now_s\n");
		sp_flush_scope_now_s(current);
		memset(current, 0, sizeof(ins_node)); 
		free(current); 
	}
}

void sp_flush_method_param() {
	ins_node * iterator = NULL;
	ins_node * last_chain = NULL;
	ins_node * current = NULL;// = _sp_method_scope[sp_method_index - 1];
	if(sp_method_index == 0) {
	//	sp_flush_scope_now();
	} else {
		current = _sp_method_scope[sp_method_index - 1];
		/*if(current->next != NULL) {
			last_chain = current->next;
			current->next = NULL;
		}*/
		sp_flush_scope_s(current);
		/*if(last_chain != NULL) {
			iterator = current->next;
			while(iterator->next != NULL) {
				iterator = iterator->next;
			}
			iterator->next = last_chain;
		}*/
	}
}

void sp_merge_scope_left() {
	ins_node * root = sp_pop_iroot();
	ins_node * root_after = sp_pop_iroot();
	ins_node * next_root = sp_current_iroot();
	ins_node * next_root_after = sp_right_iroot();
	ins_node * inode = NULL;
	ins_node * prev_nodes = NULL;
	ins_node * temp;

	if(root != NULL) {
		inode = root->next;
		if(inode != NULL) {
			prev_nodes = inode;
			while(inode->next != NULL) {
				inode = inode->next;
			}
			temp = next_root->next;
			next_root->next = prev_nodes;
			inode->next = temp;
			memset(root, 0, sizeof(ins_node));
			free(root);
		}
	}
	//printf("merge scope\n");
	if(root_after == NULL) return;
	prev_nodes = root_after->next;
	if(prev_nodes == NULL) return;
	inode = next_root_after->next;
	if(inode == NULL) { 
		next_root_after->next = prev_nodes; 
		memset(root_after, 0, sizeof(ins_node));
		free(root_after); 
	} else {
		while(inode->next != NULL) {
			inode = inode->next;
		}
		inode->next = prev_nodes;
		memset(root_after, 0, sizeof(ins_node));
		free(root_after);
	}
}

void sp_merge_scope_right() {
	ins_node * root = sp_pop_iroot();
	ins_node * root_after = sp_pop_iroot();
	ins_node * next_root = sp_current_iroot();
	ins_node * next_root_after = sp_right_iroot();
	ins_node * inode = NULL;
	ins_node * prev_nodes = NULL;
	ins_node * temp;
	//sp_push_iroot(NULL);
	//sp_push_iroot(NULL);
	//printf("index = %d\n", _sp_inode_index);
	//fflush(0);

	if(root == NULL) return;
	prev_nodes = root->next;
	//root->next = NULL;
	if(prev_nodes == NULL) return;
	temp = next_root->next;
	if(inode == NULL) { 
		next_root->next = prev_nodes; 
		memset(root, 0, sizeof(ins_node));
		free(root); 
	} else {
		while(inode->next != NULL) {
			inode = inode->next;
		}
		inode->next = prev_nodes;
		memset(root, 0, sizeof(ins_node));
		free(root);
	}
	inode = temp;
	return;
	if(root_after == NULL) return;
	prev_nodes = root_after->next;
	//root_after->next = NULL;
	if(prev_nodes == NULL) return;
	if(next_root_after == NULL) return;
	inode = next_root_after->next;
	if(inode == NULL) { 
		next_root_after->next = prev_nodes; 
		memset(root, 0, sizeof(ins_node));
		free(root_after); 
	} else {
		while(inode->next != NULL) {
			inode = inode->next;
		}
		inode->next = prev_nodes;
		memset(root, 0, sizeof(ins_node));
		free(root_after);
	}
}

void sp_swap_scope() { 
	ins_node * temp =  _sp_inode_stack[_sp_inode_index - 1]; 
	_sp_inode_stack[_sp_inode_index - 1] = _sp_inode_stack[_sp_inode_index - 2]; 
	_sp_inode_stack[_sp_inode_index - 2] = temp;  
}

void sp_destroy_scope() {
	ins_node * root;
	sp_flush_scope();			//flush current scope
	root = sp_pop_iroot();
	if(root != NULL) { memset(root, 0, sizeof(ins_node)); free(root); }
	//sp_flush_scope();			//flush right scope
	root = sp_pop_iroot();
	if(root != NULL) { memset(root, 0, sizeof(ins_node)); free(root); }
	//sp_flush_scope();			//flush left scope
	//root = sp_pop_iroot();
	//if(root != NULL) { memset(root, 0, sizeof(ins_node)); free(root); }
	sm_printf("sp_destroy_scope() : %d\n", _sp_inode_index);
	//sp_pop_varctx_scope();
}

symrec * sp_set_scope_var(symrec * rec) {
	ins_node * inode = sp_current_iroot();
	printf("inode : %ld\n", inode);
	if(inode != NULL) inode->scope_var = rec;
	return rec;
}

symrec * sp_get_scope_var() {
	symrec * rec = NULL;
	ins_node * inode = sp_current_iroot();
	if(inode != NULL) rec = inode->scope_var;
	return rec;
}

void sp_create_new_scope(symrec * var) {			//use double scoping (current-after)
	//sp_push_varctx_scope();
	//sp_push_iroot(sp_create_inode(INS_LBL, sp_create_label(_RECAST(uchar *,"__0"))));	//left scope
	sp_push_iroot(sp_create_inode(INS_LBL, sp_create_label(_RECAST(uchar *,"__a"))));	//right scope
	sp_push_iroot(sp_create_inode(INS_LBL, sp_create_label(_RECAST(uchar *,"__z"))));	//current scope
	sp_set_scope_var(var);
	sm_printf("sp_create_new_scope() : %d\n", _sp_inode_index);
}

void sp_end_scope() {
	symrec * rec = sp_get_scope_var(); 
	if(rec != NULL && (rec->type & 0x0F) == SYM_TYPE_VAR) { 		//lazy assignment
		sp_clear_current_scope(); sp_destroy_scope(); 
		sp_lz_load_variable(rec->name); 
	} else { 														//lazy operation
		sp_merge_scope_right();
	}
}

//////////////////////////////////third gen support//////////////////////////////////
void sp_import_package(uchar * pkgname) {
	uchar buffer[256];
	uint16 i = 0;
	memset(buffer, 0, sizeof(buffer));
	strcpy(_RECAST(char *,buffer), _RECAST(const char *, pkgname));
	for(i = 0;i<strlen(_RECAST(const char *,buffer)); i++) {
		if(buffer[i] == '.') buffer[i] = '\\';
	}
	sm_printf("import %s\n", buffer);
	lk_import_directory(buffer);
}

symrec * sp_lz_load_class(uchar * classname) {
	symrec * srec; 
	pk_class * classrec = lk_select_class(lk_get_root(), classname);
	if(classrec == NULL) { sp_error(SP_ERR_UNDEF_CLASS, classname); }
	srec = st_sym_select(SYM_TYPE_CONST, classname);
	if(srec == NULL) { sp_push_constant(classname); srec = st_sym_select(SYM_TYPE_CONST, classname); }
	sp_push_inode(sp_create_inode(INS_OBJNEW, srec)); 
	return srec;
}

symrec * sp_start_method_call(uchar * methname) {
	symrec * func ;

	func = st_sym_create(SYM_TYPE_FUNCTION, methname, SYM_UNDEFINED);
	func->pcount = 0;
	//sp_create_new_scope();
	sp_push_varctx_scope();
	return func;
}

uint32 sp_push_method_param(symrec * rec) {
	if(rec != NULL) {
		rec->pcount++;
	}
	sp_pop_varctx_scope();
	sp_push_varctx_scope();
	return _current_offset;
}

symrec * sp_end_method_call_scope(symrec * methrec) {
	uchar methname[256];
	symrec * srec = NULL;
	if((methrec->type & (SYM_TYPE_CONST | SYM_TYPE_FUNCTION)) != (SYM_TYPE_CONST | SYM_TYPE_FUNCTION)) {
		sprintf(_RECAST(char *,methname), "%s?%i", methrec->name, methrec->pcount);
		srec = st_sym_select(SYM_TYPE_CONST | SYM_TYPE_FUNCTION, methname);
	} else {
		printf("type interface call\n"); getchar();
		srec = methrec;
	}
	if(srec == NULL) { 
		sp_push_constant(methname); 
		srec = st_sym_select(SYM_TYPE_CONST, methname); 
		srec->ptotal = methrec->pcount;
		srec->ptotal |= 0x80;						//lock method numargs
	}
	if(srec != NULL) {										//variable not found
		if(((srec->ptotal & 0x80) != 0) && methrec->pcount < (srec->ptotal & 0x7F)) sp_error(SP_ERR_LESS_ARGUMENTS, srec->name);
		if(((srec->ptotal & 0x80) != 0) && methrec->pcount > (srec->ptotal & 0x7F)) sp_error(SP_ERR_OVER_ARGUMENTS, srec->name);

		sp_push_inode(sp_create_inode(INS_EXTCALL + methrec->pcount, srec));
	}
	return srec;
}

symrec * sp_end_method_call(symrec * methrec) {
	uchar classname[256];
	uchar methname[256];
	pk_method * pkm = NULL;
	pk_class * pkc = NULL;
	symrec * srec = NULL;
	//srec = st_sym_select(SYM_TYPE_CONST | SYM_TYPE_FUNCTION, methname);
	if((methrec->type & (SYM_TYPE_CONST | SYM_TYPE_FUNCTION)) != (SYM_TYPE_CONST | SYM_TYPE_FUNCTION)) {
		sprintf(_RECAST(char *,methname), "%s?%i", methrec->name, methrec->pcount);
		srec = st_sym_select(SYM_TYPE_CONST | SYM_TYPE_FUNCTION, methname);
	} else {
		printf("type interface call\n"); getchar();
		srec = methrec;
	}
	if(srec == NULL) { 
		sp_push_constant(methname); 
		srec = st_sym_select(SYM_TYPE_CONST, methname); 
		srec->ptotal = methrec->pcount;
		srec->ptotal |= 0x80;						//lock method numargs
	}
	if(srec != NULL) {										//variable not found
		if(((srec->ptotal & 0x80) != 0) && methrec->pcount < (srec->ptotal & 0x7F)) sp_error(SP_ERR_LESS_ARGUMENTS, srec->name);
		if(((srec->ptotal & 0x80) != 0) && methrec->pcount > (srec->ptotal & 0x7F)) sp_error(SP_ERR_OVER_ARGUMENTS, srec->name);

		sp_push_inode(sp_create_inode(INS_EXTCALL + methrec->pcount, srec));
	}
	sp_pop_varctx_scope();
	//fflush(0);
	st_sym_dispose(methrec);
	return srec;
}

//LAMBDA EXPRESSION (2018.01.08)
symrec * sp_start_lambda_decl(uchar icount) {
	symrec * func;
	uchar funcname[256];
	sprintf(_RECAST(char *, funcname), "__lf%d", _sp_label_index++);		//unlike sp_start_function_call, name is automatically generated, combined with counter
	//sp_create_new_scope();
	//create new function but don't add it to symbol table yet
	func = st_sym_create(SYM_TYPE_FUNCTION, funcname, SYM_UNDEFINED);
	func->pcount = icount;
	func->parent = _sp_current_function;						//assign label parent, for removal
	sp_flush_scope();
	//2: reserve area for goto instruction  (GOTO END_FUNCTION_BODY)
	_sp_skip_function_body = sp_create_label(_RECAST(uchar *,"__lx"));								//register skip function
	sp_skip_lambda_body();
	sp_push_lambda();										//store current function context
	st_push_table();										//create new symbol table
	_sp_current_function = func;
	//printf("create new lambda function\n");

	//sp_create_new_scope();
	
	return func;
}

symrec * sp_create_lambda_param(uchar * varname) {
	uchar lblname[256];
	symrec * newvar = NULL;
	newvar = st_sym_select(SYM_TYPE_VAR, varname);
	if(newvar == NULL) {
		//printf("create variable %s\n", varname);
		newvar = st_sym_insert(SYM_TYPE_VAR, varname, _sp_context_cntr);
		_sp_context_cntr++;
	}
	if(_sp_current_function != NULL) {
		newvar->parent = _sp_current_function;						//assign variable parent, for removal
		if((_sp_current_function->ptotal & 0x80) == 0) {		//check if function count is locked
			_sp_current_function->pcount ++;
		}
	}
	return newvar;
}

symrec * sp_execute_lambda_call(symrec * rec) {
	symrec * frec = NULL;
	uchar funcname[256];
	sprintf(_RECAST(char *, funcname), "%s?%d", rec->name, rec->pcount);		//add argument to function name (overloading)
	//printf("add %s to table\n", rec->name);
	frec = st_sym_add_record(rec);		//add this function to symbol table
	frec->ptotal = frec->pcount;		//set function numargs
	frec->ptotal |= 0x80;				//lock function numargs
	sprintf(_RECAST(char *,frec->name), "%s", funcname);
	frec->length = strlen(_RECAST(const char *, funcname));
	return frec;
}

uint32 sp_skip_lambda_body(void) {
	uint32 offset;
	//used by parent statement (parent body) to skip lambda declaration
	if(_sp_skip_function_body != NULL) {
		offset = sp_jump_to(INS_JMP, _sp_skip_function_body->name);
	}
	return offset;
}

//must stack these vars
//	_sp_current_function = a symbol stored the current function declaration
//	_sp_end_function_body = a label marked the end of function body
//	_sp_context_cntr = number of context allocated for this function
symrec * sp_start_lambda_body(symrec * func) {
	uchar funcname[256];
	jmprec * itr;
	symrec * newfunc = NULL;
	//printf("start lambda body %s\n", func->name);
	//newfunc = st_sym_select(SYM_TYPE_FUNCTION, func->name);			//function prototype already declared
	newfunc = func;
	if(newfunc != NULL) {											//relase temp function record, use function record from symtable
		if(newfunc->offset == SYM_UNDEFINED) {
			//st_sym_update_parent(func, newfunc);
			//st_sym_dispose(func);
			newfunc->offset = _current_offset;
			_sp_current_function = newfunc;									//local variable/label anchor
			//printf("create var %s\n", newfunc->name);
			sp_create_variable(newfunc->name);
			itr = (jmprec *)newfunc->list;								//search queued list for backpatching
			while(itr != NULL) {
				is_gencode(itr->offset, itr->ins, newfunc);
				itr = itr->next;
			}
			st_jmp_clear(newfunc);
		}  else {
			//function already defined
			//printf("function already defined\n");
			sm_printf(_RECAST(char *, SP_ERR_MULTI_FUNCTION), func->name);
			fflush(0);
			sp_error(SP_ERR_MULTI_FUNCTION, func->name);
			return NULL;
		} 
	} else {
		//printf("function not existed\n");
		sm_printf(_RECAST(char *, SP_ERR_INVALID_LAMBDA_DECL), func->name);
		sp_error(SP_ERR_INVALID_LAMBDA_DECL, func->name);
		return NULL;
	}
	//create end body label
	sp_push_varctx_scope();
	_sp_end_function_body = sp_create_label(_RECAST(uchar *,"__ln"));								//register end function
	//generate new label for this function body
	_current_offset += is_gencode(_current_offset, INS_LBL, newfunc);			//generate function label
	newfunc->offset = _current_offset;
	_current_offset += is_gencode(_current_offset, INS_SCTX, 0);
	//printf("end start_lambda_body\n");
	return newfunc;
}

symrec * sp_end_lambda_decl(symrec * func) {
	//printf("end lambda body\n");			//end lambda body
	if(func == NULL) return func;
	if(func != NULL) {
		//printf("backpatch sctx : %d\n", _sp_context_cntr);
		is_gencode(func->offset, INS_SCTX, _sp_context_cntr);
	} else {
		//printf("empty function\n");
	}
	if(_sp_end_function_body != NULL) {
		//printf("create_exit_label\n");
		sp_new_label(_sp_end_function_body->name);
	}
	///printf("destroy scope\n");
	//sp_destroy_scope();
	sp_load_stack(func->name);							//load return value
	sp_flush_scope_now();
	_current_offset += is_gencode(_current_offset, INS_RCTX, _sp_context_cntr);
	_current_offset += is_gencode(_current_offset, INS_RET);
	//end lambda declaration (return previous stacked global vars)
	//_sp_context_cntr = 0;
	//_sp_current_function = NULL;
	//_sp_end_function_body = NULL;
	//2.5: backpatch reserved goto
	//3: call function (might be F2O, convert from function to object)
	//sp_push_inode(sp_create_inode(INS_CALL, func));
	//sp_push_inode(sp_create_inode((INS_F2O + (func->ptotal & 0x0F)), func));
	//printf("clear locals parent:%s\n", func->name);
	st_sym_clear_locals(func);
	sp_pop_varctx_scope();
	//sp_end_scope();
	st_pop_table();											//restore previous symbol table
	sp_pop_lambda();										//store current function context
	if(_sp_skip_function_body != NULL) {
		//printf("backpatch %s\n", _sp_skip_function_body->name);
		sp_new_label(_sp_skip_function_body->name);
		_sp_skip_function_body = NULL;
	} //else printf("skip body is null\n");
	//{ sp_lz_load_lambda(func); }
	//return sp_push_symrec(func);
	return func;
}

symrec * sp_install_menu(uchar * menuname, symrec * methrec) {
	if(_sp_prog_class != NULL) {
		if(pk_register_menu(_sp_prog_class, methrec->name, menuname) == NULL) {
			sp_error(SP_ERR_MUST_PUBLIC, methrec->name);
		}
	}
	return methrec;
}

symrec * sp_install_event(uchar * eventname, symrec * methrec) {
	uchar id = 0;
	if(_sp_prog_class != NULL) {
		id = atoi(_RECAST(const char *,eventname));
		if(pk_register_event(_sp_prog_class, methrec->name, id) == NULL) {
			sp_error(SP_ERR_MUST_PUBLIC, methrec->name);
		}
	}
	return methrec;
}

try_catch_block * tc_block[SEM_MAX_STACK];
int tc_block_index = 0;
void sp_push_try_catch() {
	try_catch_block * tc = (try_catch_block *)malloc(sizeof(try_catch_block));
	tc->start = _current_offset;
	tc->end = tc->start;
	tc->cblock = -1;
	tc->num_args = 0;
	tc_block[tc_block_index++] = tc;
}

void sp_set_cblock() {
	try_catch_block * tc ;
	if (tc_block_index == 0) return;
	tc = tc_block[tc_block_index - 1];
	tc->end = _current_offset;
	tc->cblock = _current_offset;
}

void sp_pop_try_catch(int num_args) {
	try_catch_block * tc;
	if (tc_block_index == 0) return;
	tc = tc_block[--tc_block_index];
	tc->num_args = num_args;
	//push try-catch metadata
	pk_register_tcblock(tc->start, tc->cblock, tc->num_args);
	free(tc);
}

void sp_error_clear() {
	err_record * temp;
	while(_sp_err_head != _sp_err_tail) {
		temp = _sp_err_head;
		_sp_err_head = _sp_err_head->next;
		memset(temp, 0, sizeof(err_record));
		free(temp);
	}
	if(_sp_err_tail != NULL) {
		temp = _sp_err_head;
		_sp_err_head = NULL;
		_sp_err_tail = NULL;
		memset(temp, 0, sizeof(err_record));
		free(temp);
	}
	_sp_err_head = NULL;
}

void sp_error_push(uchar * msg) {
	err_record * rec = (err_record *)malloc(sizeof(err_record));
	rec->next = NULL;
	//printf("error %s\n", msg);
	memset(rec->buffer, 0, 256);
	if(_sp_err_tail == NULL) {
		_sp_err_tail = rec;
		_sp_err_head = rec;
	} else {
		_sp_err_tail->next = rec;
		_sp_err_tail = rec;
	}
	strncpy(_RECAST(char *, rec->buffer), _RECAST(const char*, msg), 255);
}

static void sp_init_global() {
	_sp_err_head = NULL;
	_sp_err_tail = NULL;
	_current_offset = 0;
	_sp_offstack_index = 0;
	_sp_symstack_index = 0;
	_sp_label_index = 0;
	_sp_context_cntr = 0;
	_sp_current_function = NULL;								//currently parsed function
	_sp_end_function_body = NULL;
	_lambda_index = 0;
}

//////////////////////////////////parser public apis//////////////////////////////////
pp_config * sp_init(uchar * filename) {
	sp_error_clear();
	sp_init_global();
	sp_init_parser();
	_current_offset = 0;
	p_config = sp_open(filename);
	p_config->show_assembly = TRUE;
	return lp_push_config(p_config);
}

pp_config * sp_open(uchar * filename) {
	extern FILE * yyin;
	p_config = (pp_config *)malloc(sizeof(pp_config));
	strcpy(_RECAST(char *,p_config->filename), _RECAST(const char *, filename));
	p_config->file = fopen(_RECAST(const char *,filename), "r");
	p_config->line = 0;
	p_config->mode = 0;
	p_config->show_assembly = FALSE;
	//if(p_config->file == NULL) return NULL;
	return p_config;
}

pp_config * sp_clr_init(uchar * codebuffer, int32 size) {
	//static pp_config curConfig;
	sp_error_clear();
	sp_init_global();
	sp_init_parser();
	_current_offset = 0;
	p_config = (pp_config *)malloc(sizeof(pp_config));
	//strcpy(_RECAST(char *,p_config->filename), _RECAST(const char *, filename));
	//p_config->file = fopen(_RECAST(const char *,filename), "r");
	p_config->line = 0;
	p_config->show_assembly = TRUE;
	p_config->codebuffer = codebuffer;
	p_config->length = size;
	p_config->mode = 1;		//direct code buffer
	p_config->index= 0;
	lp_init();
	is_init(NULL);
	as_init(NULL);
	//global constant
	lp_update((uchar *)"true", (uchar *)"true", 4);
	lp_update((uchar *)"false", (uchar *)"false", 5);
	lp_update((uchar *)"null", (uchar *)"", 0);
	//sc_init(NULL);
	return lp_push_config(p_config);
}

int pp_getc(pp_config * pfig) {
	int c;
	//printf("getc");
	if(pfig == NULL) { return EOF; }
	//printf("%d", pfig->mode);
	if(pfig->mode == 1) {
		if(pfig->index >= pfig->length) { return EOF; } 
		c = pfig->codebuffer[pfig->index++];
		//putchar(c);
	} else {
		c = getc(pfig->file);
		//putchar(c);
	}
	return c;
}

void pp_close(pp_config * pfig){
	if(pfig == NULL) return;
	if(pfig->mode == 1) {
		pfig->index = pfig->length;
	} else fclose(pfig->file);	
}

void pp_seek(pp_config * pfig, long offset, int origin) {
	if(pfig == NULL) return;
	if(pfig->mode == 1) {
		switch(origin) {
			case SEEK_CUR:
				//printf("pseek %d %d\n", pfig->index, pfig->index + offset);
				pfig->index += offset;
				break;
			case SEEK_SET:
				pfig->index = offset;
				break;
			default: break;
		}
	} else fseek(pfig->file, offset, origin);
	
}

int pp_error(pp_config * pfig) {
	if(pfig == NULL) return -1;
	if(pfig->mode == 1) {
		if(pfig->index >= pfig->length) return -1;
		return 0;
	} else return ferror(pfig->file);
	
}

err_record * sp_error_get_enumerator() {
	return _sp_err_head;
}

err_record * sp_error_next_record(err_record * rec) {
	return rec->next;
}
		  
