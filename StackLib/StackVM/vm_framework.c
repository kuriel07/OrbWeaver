#include "../defs.h"
#include "../config.h"	
#include "../crypto/inc/cr_apis.h"
#include "vm_stack.h"	
#include "vm_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__GNUC__)
#include <dlfcn.h>
#else
#include "dlfcn.h"
#endif
#ifdef STACK_HOST
#if defined(WIN32) || defined(WIN64)
#include "core/global.h"
#endif
#if defined(__GNUC__)
#include "global.h"
#include <pthread.h>
#endif
#endif

//extern vm_object * g_pVaRetval;
//extern sys_context g_sVaSysc;
//tk_context_p g_pfrmctx = NULL;

static uint8 vm_imemcmp(void * op1, void * op2, uint16 size) _REENTRANT_ {
	uint8 * p_dst = (uint8 *)op1;
	uint8 * p_src = (uint8 *)op2;
	uint8 c, d;
 	while(size-- != 0) {
		c = p_dst[size];
		d = p_src[size];
		if(c >= 'a' && c <='z') c -= 0x20;
		if(d >= 'a' && d <='z') d -= 0x20;
	 	if(c != d) return -1;
	}
	return 0;
}

CONST vm_api_entry g_vaRegisteredApis[] = {
 	//var APIs
	{0, "this", va_this, NULL},
#if STACK_VAR_APIS
	{1, "set_value", va_set_var, NULL},
 	{2, "get_value", va_get_var, NULL},
 	{3, "delete_value", va_delete_var, NULL},
#endif
	//string APIs
	{4, "index_of", va_index_of, NULL},
	{5, "replace", va_replace, NULL},	   
	{6, "substr", va_substr, NULL},
	{7, "to_bytes", va_bytes, NULL},
#if STACK_FILE_APIS || STACK_HOST
	//file APIs
	{8, "fopen", va_fopen, NULL},  
#endif
	{9, "close", va_close, NULL},			//default context close
	{10, "read", va_read, NULL},			//default context read
	{11, "write", va_write, NULL},			//default context write
	{12, "seek", va_seek, NULL},			//default context seek
#if STACK_FILE_APIS 
	{12, va_fpopbytag, NULL},
#endif 
	//array-object APIs (13-28)
	{13, "count", va_arg_count, NULL},
	{14, "arg_create", va_arg_create, NULL},
	{15, "arg_object", va_arg_object, NULL},
	{16, "arg_array", va_arg_array, NULL}, 	 
	{17, "at", va_arg_at, NULL},
	{18, "get", va_arg_get, NULL},
	{19, "to_json", va_arg_serialize, NULL},			// -> to json string
	{20, "from_json", va_arg_deserialize, NULL},			// -> from json string
	{19, "serialize", va_arg_serialize, NULL},			// -> serialize current object
	{20, "deserialize", va_arg_deserialize, NULL },			// -> deserialize from array/string
	{21, "add", va_arg_add, NULL},
	{22, "set", va_arg_set, NULL},
	{23, "remove", va_arg_remove, NULL},
	{24, "dequeue", va_arg_dequeue, NULL },
	{25, "pop", va_arg_pop, NULL },
	{27, "split", va_string_split, NULL},			//string split (added 2018.04.29)
	{28, "first", va_arg_first, NULL },
	{29, "next", va_arg_try_next, NULL},
	//operand-object conversion APIs
	{30, "cast", va_dlcast, NULL},
	//support for lambda expression (2018.01.10)
	{31, "exec", va_sys_exec, NULL},

	//external invocation APIs (32-36)
#if 0
	{32, "ti_load", va_invoke_external, NULL},			//invoke another framework
	{33, "ci_load", va_invoke_card, NULL},			//invoke card framework
#endif
	{36, "dl_load", va_dlopen, NULL},			//invoke external library through interface
	
	//ISO messaging APIs (37-39)
#if 0
	{37, "iso_create", va_iso_create_message, NULL },
	{38, "iso_push", va_iso_push_element, NULL },
	{39, "iso_get", va_iso_get_element, NULL },
#endif

	//data extension APIs (40-47)
	{40, "to_num", va_to_float, NULL },		//convert string to float

	//atomic operation APIs (48-51)
#if 0
	{48, "begin", va_async_begin, NULL},
	{49, "end", va_async_end, NULL},
	{50, "rollback", va_async_rollback, NULL}
	{51, "commit", va_async_commit, NULL},
#endif

#if STACK_BIT_APIS
	//bit APIs
	{128, "check_bit", va_check_bit, NULL},
	{129, "set_bit", va_set_bit, NULL},	 
	{130, "clear_bit", va_clear_bit, NULL},
#endif
#if STACK_CONVERTER_APIS
	//converter APIs			(131-135)
	{131, "to_hex", va_bin2hex, NULL},
	{132, "from_hex", va_hex2bin, NULL}, 
	{133, "to_dec", va_bin2dec, NULL},
	{134, "to_b64", va_b64_encode, NULL},
	{135, "from_b64", va_b64_decode, NULL},
#endif

#if STACK_CRYPTO_APIS	
	//cryptography APIs		(136-142)
	{136, "cr_create", va_crypto_create, NULL},
	{137, "encrypt", va_crypto_encrypt, NULL },	 
	{138, "decrypt", va_crypto_decrypt, NULL },
	{139, "cr_random", va_random, NULL },
	{140, "hash", va_digest, NULL },
#endif
#if STACK_NETWORK_APIS
	//network APIs		(144-148)
	{144, "net_open", va_net_open, NULL },			//create a new connection (net_context), whose behaviour similar with file context
	{145, "net_transmit", va_net_transmit, NULL},		//(param1 = address, param2=payload (OWB array), param3 = method (GET, POST), param4=type(TCP, UDP)  
	{148, "mail_create", va_net_mail_create, NULL},	//param1=server, param2=port, param3=username, param4=password
	{149, "mail_send", va_net_mail_send, NULL},		//param1=handle, param2=to, param3=subject, param4=msg, param5=from
#endif
#if STACK_GUI_APIS	
	//ui APIs (160-191)
	{160, va_ui_alert, NULL },
	{161, va_ui_create_window, NULL },
	{162, va_ui_destroy_window, NULL },
	{163, va_ui_create_label, NULL },
	{164, va_ui_create_button, NULL },
	{165, va_ui_create_textbox, NULL },
	{166, va_ui_create_image, NULL},
	{168, va_ui_wait, NULL },
	//UI framework APIs
	{169, va_display_text, NULL},
	{170, va_get_input, NULL},
	{171, va_select_item, NULL},
	//UI util APIs
	{176, va_ui_get_text, NULL},
	{177, va_ui_set_text, NULL},
	
	{190, va_ui_push_window, NULL},
	{191, va_ui_pop_window, NULL},
#endif
	{169, "print", va_display_text, NULL},
#if STACK_IO_APIS	
	//UART APIs				(192-195)
	{192, "com_open", va_com_open, NULL },			//create a comm connection (com_context), whose behaviour similar with file context
	//{193, va_com_transmit, NULL},		//send bytes and wait for response at specific period
	{194, "com_readline", va_com_readline, NULL},		//read bytes until newline found
		
	//Port APIs (196-199)
	{196, "port_open", va_port_open, NULL },
	//{197, va_io_write, NULL },
 #endif		
#if STACK_PICC_APIS	//(200-204)
	{200, "picc_open", va_picc_open, NULL},
	{201, "picc_auth", va_picc_auth, NULL},
	{202, "picc_transmit", va_picc_transmit, NULL},
#endif
#ifdef STACK_HOST
	//OS APIs (specific)
	{ 240, "sleep", va_sleep, NULL},			//OS APIs specific
	{ 241, "thread", va_create_thread, NULL},
	{ 242, "start", va_thread_start, NULL },
	{ 243, "stop", va_thread_stop, NULL },
#endif
	//dynamic loading
	//{248, "dlopen", va_dlopen, NULL},
	//{249, "dlexec", va_dlexec, NULL},
	//{250, "cast", va_dlcast, NULL},
	{0, "", NULL, NULL}
} ;

//extern vf_handle _vm_file;
uint8 g_vaWaitTag = 0; 

//init once
void va_init_context() {
	int rc;
	//GPIO_InitTypeDef GPIO_InitStructure;       
	//g_pfrmctx = ctx;
	//port init
	//__HAL_RCC_GPIOE_CLK_ENABLE();
   	/* PORT E.0, E.1, E.2 */
  	//GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
  	//GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;	 					//default AF_PP
	//GPIO_InitStructure.Pull = GPIO_PULLUP;								//default NOPULL
  	//GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
  	//HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	//com init
	//va_com_init();
	va_data_init();
}

void vm_init_global(void * ctx) {
	va_init_context();	//init framework context (dev entity)
#if defined(STACK_HOST)
	printf("init va_ext_init\n");
	va_ext_init();
#endif
}

void va_exec_init() {			//called one time before any execution
	//GUI init
	//va_ui_init();
	//va_picc_init();
}

void va_exec_release() {		//called one time after any execution
	//GUI release
	//va_ui_release_all();
	//va_picc_release_all();
}

int32 va_o2f(VM_DEF_ARG, vm_object * obj) _REENTRANT_ {
	uint8 buffer[64];
	vm_object * param;
	uint8 offset = 0;
	uint8 len = obj->len;
	if(len == 0) return 0;		//NULL object

	if (obj->type != VM_OBJ_TYPE_ANY) {
		param = va_data_cast(VM_ARG, obj, VM_OBJ_TYPE_ANY);
		if (param->len > 64) len = 63;
		len = param->len;
		offset = param->len - len;
		vm_memcpy(buffer, param->bytes + offset, len);
		buffer[len] = 0;
		vm_release_object(VM_ARG, param);
	} else {
		if (obj->len > 64) len = 63;
		offset = obj->len - len;
		vm_memcpy(buffer, obj->bytes + offset, len);
		buffer[len] = 0;
	}
	return (int32)atoi((const char *)buffer);
}	  

void va_return_word(VM_DEF_ARG, int32 val) _REENTRANT_ {
	char nbuf[20];
	sprintf(nbuf, "%d", (int32)val);
	vm_set_retval( vm_create_object(VM_ARG, vm_strlen(nbuf), _RECAST(uchar *, nbuf)));
}

#define VA_DISP_TEXT 	STK_CMD_DISPLAY_TEXT 
#define VA_DISP_INKEY	STK_CMD_GET_INKEY
#define VA_DISP_IDLE	STK_CMD_SET_UP_IDLE_TEXT

#define VA_OBJECT_DELIMITER		0x80
#define VA_VAR_SET			1
#define VA_VAR_GET			2
#define VA_VAR_DELETE	3 
 
void va_this(VM_DEF_ARG) _REENTRANT_ {
	vm_context * vctx = &vm_get_context();
	//printf("this() called\n");
	vm_set_retval(vm_create_object(VM_ARG, sizeof(vm_context *), _RECAST(uchar *, &vctx)));
}

#if STACK_VAR_APIS
#if 0
static void va_var_operation(uint8 mode) _REENTRANT_ {
   	vm_object * param;
	//FSHandle fhandle;	
	vf_handle fhandle;
	uint16 i = 0, j;
	uint8 exist = 0;
	BSIterator iterator;
	uint8 tag[3] = { 0xE0, 0x1F, 0x00 };
	uint8 lbuf[VA_OBJECT_MAX_SIZE];
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint16 objLen;     
	uint16 glen = 0;
	uint16 clen;				    
	param = vm_get_argument(VM_ARG, 0);
	if(param->len == 0) return;// g_pVaRetval;
	tk_kernel_data_load(g_pfrmctx, &fhandle);
	memcpy(&iterator, &fhandle, sizeof(BSAllocHandle));
	objLen = bsIteratorInit(&iterator, (BSAllocHandle *)&iterator);
	while(objLen != 0) {
		objLen = bsIteratorRead(&iterator, 0, bbuf, objLen);
		//list variable
		if(param->len == 0) {
			lbuf[glen + 2] = ASN_TAG_OCTSTRING;	
			for(clen=0;clen<objLen;clen++) if(bbuf[clen] == VA_OBJECT_DELIMITER) break;
			lbuf[glen + 3] = clen;
	   		memcpy(lbuf + glen + 4, bbuf, clen); 
			glen += (clen + 2);
		} else if(objLen >= (param->len +1)) {
			if(memcmp(bbuf, param->bytes, param->len) == 0 && bbuf[param->len] == VA_OBJECT_DELIMITER) break;	//matched
		}
		objLen = bsIteratorNext(&iterator);
	}
	switch(mode) {
		case VA_VAR_GET:
			vm_set_retval(VM_NULL_OBJECT); 
			if(param->len == 0) {  
				//list variable
				lbuf[0] = ASN_TAG_SET;
				lbuf[1] = glen;
				g_pVaRetval = vm_create_object(glen + 2, lbuf);
				return;
			}
			if(objLen == 0) return; 
			if(objLen == (param->len + 1)) return;
			vm_set_retval(vm_create_object(objLen - (param->len + 1), bbuf + (param->len + 1)));
			break;	
		default:
		case VA_VAR_DELETE:
	 	case VA_VAR_SET:  
			if(objLen != 0) {
				bsReleaseByOffset((BSAllocHandle *)&iterator, iterator.current);	
			}
			if(mode == VA_VAR_DELETE) break;
			memcpy(bbuf, param->bytes, param->len); 
			j = param->len;
			bbuf[j++] = VA_OBJECT_DELIMITER;
			param = vm_get_argument(VM_ARG, 1);	
			memcpy(bbuf + j, param->bytes, param->len);
			j += param->len;	 
			if(bsErr(iterator.current = bsAllocObject((BSAllocHandle *)&iterator, tag, j))) return;
			bsHandleWriteW(&iterator, iterator.current, bbuf, j);
			break;
	}
}
#endif
		
void va_set_var(VM_DEF_ARG) _REENTRANT_ { 
	//OS_DEBUG_ENTRY(va_set_var);
	vm_context * wctx;
	vm_variable * iterator;
	uint8 wbuf[VA_OBJECT_MAX_SIZE];
	uint16 len;
	//va_var_operation(VA_VAR_SET);	
	vm_object * _this = vm_get_argument(VM_ARG, 0);
	vm_object * vname = vm_get_argument(VM_ARG, 1);
	vm_object * vvalue = vm_get_argument(VM_ARG, 2);
	if(vm_get_argument_count(VM_ARG) < 1) goto exit_set_var;
	//if((_this->mgc_refcount & VM_CTX_MAGIC) != VM_CTX_MAGIC) goto return_set_var;
	if(vm_get_argument_count(VM_ARG) < 3) goto return_set_var;
	//start set variable using current context
	//start check whether variable already exist
	//ctx = (vm_context *)_this->bytes;
	memcpy(&wctx, _this->bytes, sizeof(vm_context *));
	//printf("ctx = %x\n", ctx->vars);
	iterator = wctx->vars[0];
	//printf("iterator = %x\n", iterator);

	len = vname->len;
	memcpy(wbuf, vname->bytes, len);
	wbuf[len++] = VA_OBJECT_DELIMITER;
	while(iterator != NULL) {
		if(memcmp(iterator->bytes, wbuf, len) == 0 && iterator->len > len) {
			//release variable with the same name
			vm_variable_release(wctx->vars, iterator);
			break;
		}
		iterator = iterator->next;
	}
	//create and add variable to the list (doesn't care if it's successful or not), return current context
	memcpy(wbuf + len, vvalue->bytes, vvalue->len);
	len += vvalue->len;
	vm_variable_new(wctx->vars, vvalue->mgc_refcount & VM_MAGIC_MASK, len, wbuf);
	return_set_var:
	vm_set_retval(_this);
	if(_this != VM_NULL_OBJECT) {
		//_this->mgc_refcount = VM_CTX_MAGIC | ((_this->mgc_refcount + 1) & 0x0F);		//copy header bytes, set to object in case didn't
		_this->mgc_refcount = (_this->mgc_refcount & VM_MAGIC_MASK) | ((_this->mgc_refcount + 1) & 0x0F);
	}
	exit_set_var:
	//OS_DEBUG_EXIT();
	return;
} 
 		
void va_get_var(VM_DEF_ARG) _REENTRANT_ { 
	//OS_DEBUG_ENTRY(va_get_var);
	vm_context * wctx;
	vm_variable * iterator;
	//va_var_operation(VA_VAR_GET);
	vm_object * _this = vm_get_argument(VM_ARG, 0);
	vm_object * vname = vm_get_argument(VM_ARG, 1);
	uint8 wbuf[VA_OBJECT_MAX_SIZE];
	uint16 len;
	if(vm_get_argument_count(VM_ARG) < 2) goto exit_get_var;
	//if((_this->mgc_refcount & VM_CTX_MAGIC) != VM_CTX_MAGIC) goto exit_get_var;
	//start get variable using current context
	//ctx = (vm_context *)_this->bytes;
	memcpy(&wctx, _this->bytes, sizeof(vm_context *));
	iterator = wctx->vars[0];
	len = vname->len;
	memcpy(wbuf, vname->bytes, len);
	wbuf[len++] = VA_OBJECT_DELIMITER;
	while(iterator != NULL) {
		if(memcmp(iterator->bytes, wbuf, len) == 0 && iterator->len > len) {
			//matched name and delimiter
			vm_set_retval(vm_create_object(VM_ARG, iterator->len - len, iterator->bytes + len));
			//set magic number and ref_count
			vm_get_retval()->mgc_refcount = (iterator->mgc & VM_MAGIC_MASK) | (vm_get_retval()->mgc_refcount & 0x0F) ;
			break;
		}
		iterator = iterator->next;
	}
	exit_get_var:
	//OS_DEBUG_EXIT();
	return;
}
		
void va_delete_var(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_delete_var);
	//va_var_operation(VA_VAR_DELETE);
	//OS_DEBUG_EXIT();
}
#endif 	//end of var apis

void va_display_text(VM_DEF_ARG) {
	uint8 i;
	vm_object * param;
	uint8 * buffer;
	uint32 len;
	vm_object * temp;
	uint8 arg_count = vm_get_argument_count(VM_ARG);
	//return;
	for(i=0;i<arg_count;i++) {
		param = vm_get_argument(VM_ARG, i);
		if (param->type != VM_OBJ_TYPE_ANY) {
			param = va_data_cast(VM_ARG, param, VM_OBJ_TYPE_ANY);
			//buffer = (uint8 *)malloc(param->len + 1);
			//memcpy(_RECAST(char *, buffer), _RECAST(const char *, param->bytes), param->len);
			//buffer[param->len] = 0;
			//len = param->len;
			//vm_release_object(VM_ARG, temp);
		}
		if(vm_object_get_type(param) == VM_EXT_MAGIC ) {
			buffer = (uint8 *)malloc( 4096);
			len = ((vm_extension *)param->bytes)->apis->text(VM_ARG, param, buffer);
		} else {
			buffer = (uint8 *)malloc(param->len + 1);
			memcpy(_RECAST(char *, buffer), _RECAST(const char *, param->bytes), param->len);
			buffer[param->len] = 0;
			len = param->len;
		}
		if (param != vm_get_argument(VM_ARG, i)) vm_release_object(VM_ARG, param);
		//VM_ARG->stream << buffer;
		if(vm_get_stream() != NULL) vm_stream_write(vm_get_stream(), buffer, len);
		//printf("%s\n", buffer);
		free(buffer);
	}
}

#define VA_STR_INDEX_OF			0
#define VA_STR_LAST_INDEX_OF	1
#define VA_STR_REPLACE			2

static void va_string_operation(VM_DEF_ARG, uint8 mode) _REENTRANT_ {
	vm_object * pattern;
	vm_object * param;
	uint16 index = 0xFFFF;
	uint16 offset = 0;
	uint8 length;
	uint8 wbuf[256];
	//BYTE buffer[10];
	if(vm_get_argument_count(VM_ARG) < 2) return;
	param = vm_get_argument(VM_ARG, 0);
	if(param->len != 0) { 		//source string
	  	vm_memcpy(wbuf, param->bytes, param->len);
		length = param->len;
	}
	param = vm_get_argument(VM_ARG, 1);
	if(param->len != 0) {		//pattern
		if(length < param->len) return;
		pattern = param;
		//length -= param->len;
	}
	param = vm_get_argument(VM_ARG, 2);
	if(param->len != 0) {  		//offset/new pattern
		if(mode < 2) { 		//check if index_of or last_index_of
			if(param->len > 10) return;
			offset = va_o2f(VM_ARG, param);	 
			if(offset > length) return;
		}  
	}
	switch(mode) {
	  	case VA_STR_INDEX_OF:
			for(offset;offset<length;offset++) {
			 	if(vm_memcmp(wbuf + offset, pattern->bytes, pattern->len) == 0) {
					//mmItoa(MM_ITOA_WORD, g_baOrbBuffer + length, (WORD)offset);
					sprintf(_RECAST(char *, wbuf), "%d", (uint16)offset);
					vm_set_retval(vm_create_object(VM_ARG, vm_strlen(_RECAST(const char *, wbuf + length)), wbuf + length));
				 	break;
				}
			}
			break;
		case VA_STR_REPLACE:
			//for(offset;offset<length;offset++) {
			while(offset < length) {
			 	if(vm_memcmp(wbuf + offset, pattern->bytes, pattern->len) == 0) {
					length -= pattern->len;
					vm_memcpy(wbuf + offset + param->len, wbuf + offset + pattern->len, length - offset);
					vm_memcpy(wbuf + offset, param->bytes, param->len);
					length += param->len;
					offset += param->len;
				} else {
					offset ++;
				}
			}
			vm_set_retval(vm_create_object(VM_ARG, offset, wbuf));
			break;
	}
	return;
}

void va_index_of(VM_DEF_ARG) _REENTRANT_ {
	OS_DEBUG_ENTRY(va_index_of);
 	va_string_operation(VM_ARG, VA_STR_INDEX_OF);
	OS_DEBUG_EXIT();
}

void va_replace(VM_DEF_ARG) _REENTRANT_ {
	OS_DEBUG_ENTRY(va_replace);
   	va_string_operation(VM_ARG, VA_STR_REPLACE);
	OS_DEBUG_EXIT();
}

uint8 va_arg_is_numeric(uint8 * str, int len) {
	int i = 0;
	uint8 n;
	if(len == 0) return FALSE;
	for(;len != 0;) {
		n = str[--len];
		if((n == '-') && (len == 0)) break;
		if(n > 0x39 || n < 0x30) { return FALSE; }
	}
	return 1;
}

uint8 va_contain_delimiter(uint8 * str, int len) {
	int i = 0;
	for(i=0;i<len;i++) {
		if(str[i] == VA_OBJECT_DELIMITER) return 1;
	}
	return 0;
}

void va_substr(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_substr);
	uint8 offset; 
	uint8 len;	 
	uint8 * opd1;
	uint8 * opd2;
	vm_object * obj;
	vm_object * op1;
	vm_object * op2;
	if(vm_get_argument_count(VM_ARG) >= 2) {
		obj = vm_get_argument(VM_ARG, 0);  
		op1 = vm_get_argument(VM_ARG, 1);
		op2 = vm_get_argument(VM_ARG, 2);
		opd1 = (uint8 *)mmAllocMemP(op1->len + 1);
		opd2 = (uint8 *)mmAllocMemP(op2->len + 1);
		vm_memcpy(opd1, op1->bytes, op1->len); opd1[op1->len] = 0;		//null terminated string
		vm_memcpy(opd2, op2->bytes, op2->len); opd2[op2->len] = 0;		//null terminated string
		if(va_arg_is_numeric(_RECAST(uchar *, op1->bytes), op1->len) == FALSE) { offset = 0; } else { offset = atoi(_RECAST(const char *, opd1)); }			//
		if(va_arg_is_numeric(_RECAST(uchar *, op2->bytes), op2->len) == FALSE) { len = obj->len; } else { len = atoi(_RECAST(const char *, opd2)); }
		if(len > (obj->len - offset)) len = (obj->len - offset);
		mmFreeMem(opd1);
		mmFreeMem(opd2);
		vm_set_retval(vm_create_object(VM_ARG, len, obj->bytes + offset));
	}
	OS_DEBUG_EXIT();
}

#define VA_FILE_OPEN 	0
#define VA_FILE_READ 	1
#define VA_FILE_WRITE 	2
#define VA_FILE_CLOSE 	4
#define VA_FILE_BY_TAG	0x10

#if STACK_FILE_APIS
static void va_file_operation(VM_DEF_ARG, BYTE mode) _REENTRANT_ {
	vm_object * param;
	vm_object * temp;
	uint8 i,j;
	//FSFileParameters fparam;
	//WORD fid;
	uint8 fidbuf[4];
	uint16 offset, length;
	FSHandle * file;// = vm_get_argument(VM_ARG, 0);
	uint8 isRead = 0;
	//fsGetPropertyB(file, &fparam, sizeof(FSFileParameters));
	switch(mode & 0x0F) {
		case VA_FILE_OPEN:
			if(vm_get_argument_count(VM_ARG) < 1) return;
			param = vm_get_argument(VM_ARG, 0);		//path
			if(param->len == 0) return;
			if((param->len & 0x03) != 0x00) return;//odd path
			//decode hex path
			temp = vm_create_object(sizeof(FSHandle) + 1, NULL);
			if(temp->len == 0) return;
			temp->bytes[0] = 0xF2;		//set handle tag
			file = (temp->bytes + 1);		//handle location pointer
			cmSelectCurrentMFW(cmGetSelectedContext(), file, NULL); 
			for(i=0;i<param->len;i+=4) {
				vm_memcpy(fidbuf, param->bytes + i, 4);
				tkHex2Bin(fidbuf, 4, 4);
				if(fsSelectFileByFidW(file, end_swap16(*((WORD *)fidbuf)), NULL) != FS_OK) {
					vm_release_object(temp);  		//delete allocated object
					return; 		//return
				} 	
			}
			vm_set_retval( temp);				//set return value
			break;
		case VA_FILE_READ: 
			isRead = 1;
		case VA_FILE_WRITE:
			if(vm_get_argument_count(VM_ARG) < 3) return;
			temp = vm_get_argument(VM_ARG, 0);
			if(temp->len != (sizeof(FSHandle) + 1)) return;	//invalid handle
			if(temp->bytes[0] != 0xF2) return;		//invalid handle
			file = (temp->bytes + 1);
			fsGetPropertyB(file, &g_strFileParams);
			offset = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1)); 		//offset
			if(isRead) {
				length = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 2));	//length
				switch(g_strFileParams.desc & 0x07) {
				  	case 0x01:		 //binary
						if(mode & VA_FILE_BY_TAG) {
							if((length = ftPopByTag(file, offset, (BYTE)length, g_baOrbBuffer, 255)) == -1) length = 0;
						} else { 
							length = fsFileReadBinaryW(file, offset, g_baOrbBuffer, length);
						}
						break;
					case 0x02:		 //linfix
					case 0x03:
					case 0x06:		 //cyclic
					case 0x07: 
						if(offset == 0) return;
						if(offset > g_strFileParams.recnum) return;
						if(mode & VA_FILE_BY_TAG) {	
							fsFileReadRecordW(file, offset -1, g_baOrbBuffer, g_strFileParams.reclen);
							offset = 0;
							while(offset < g_strFileParams.reclen) {
							  	if(g_baOrbBuffer[offset++] == (BYTE)length) {
								 	length = g_baOrbBuffer[offset++];
									vm_memcpy(g_baOrbBuffer, g_baOrbBuffer + offset, length);
									offset = g_strFileParams.reclen;
									goto operation_finished;
								} else {
								 	offset += (g_baOrbBuffer[offset] + 1);
								}
							}
							length = 0; 	
						}  else {
							length = fsFileReadRecordW(file, offset -1, g_baOrbBuffer, length);	
						}
						break;
				}
				operation_finished:
				vm_set_retval(vm_create_object((uint8)length, g_baOrbBuffer));  		//set return to data readed
			} else {
			 	param = vm_get_argument(VM_ARG, 2);				//data
				switch(g_strFileParams.desc & 0x07) {
				  	case 0x01:		//binary
						length = fsFileWriteBinaryW(file, offset, param->bytes, param->len);
						break;
					case 0x02:		//linfix
					case 0x03:		  
						if(offset == 0) return;
						if(offset > g_strFileParams.recnum) return;
						length = fsFileWriteRecordW(file, offset - 1, param->bytes, param->len);
						break;
#if FS_API_APPEND_RECORD
					case 0x06:		//cyclic
					case 0x07:
						if(offset != 0) return;
						length = fsFileAppendRecordW(file, 0, param->bytes, param->len);
						break;
#endif
				} 
				mmItoa(MM_ITOA_WORD, g_baOrbBuffer, length);
				length = (BYTE)mmStrLen(g_baOrbBuffer);
				goto operation_finished;
			}
			break;
		case VA_FILE_CLOSE:
			if(vm_get_argument_count(VM_ARG) < 1) return;
			temp = vm_get_argument(VM_ARG, 0);
			if(temp->len != (sizeof(FSHandle) + 1)) return;	//invalid handle
			file = (temp->bytes + 1);
			if(temp->bytes[0] == 0xF2) {	   
				temp->bytes[0] = 0x2F;			//set handle tag to closed handle
			}
			//optional parameter (activate/deactivate)
			param = vm_get_argument(VM_ARG, 1);
			if(param->len != 0) {
				fsChangeStateB(file, va_o2f(VM_ARG, param));
			}
			break;
	}
	return;
}

void va_fopen(VM_DEF_ARG) _REENTRANT_ {
	va_file_operation(VM_ARG, VA_FILE_OPEN);
}

void va_fpopbytag(VM_DEF_ARG) _REENTRANT_ {  
	va_file_operation(VM_ARG, VA_FILE_BY_TAG | VA_FILE_READ);
}

void va_fread(VM_DEF_ARG) _REENTRANT_ {
	va_file_operation(VM_ARG, VA_FILE_READ);
}

void va_fwrite(VM_DEF_ARG) _REENTRANT_ { 
	va_file_operation(VM_ARG, VA_FILE_WRITE);
}

void va_fclose(VM_DEF_ARG) _REENTRANT_ { 
	va_file_operation(VM_ARG, VA_FILE_CLOSE);
} 
#else
//use default file operation

void va_file_seek(VM_DEF_ARG) {
	va_default_context * defctx;
	vm_object * vfile = vm_get_argument(VM_ARG, 0);
	int offset = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	if(vfile->len == 0) return;
	defctx = (va_default_context *)vfile->bytes;
	if(offset < 0) {
		fseek(defctx->ctx, 0, SEEK_END);
		offset = ftell(defctx->ctx);
		va_return_word(VM_ARG, offset);
		return;
	}
	fseek(defctx->ctx, offset, SEEK_SET);
	va_return_word(VM_ARG, offset);
}

void va_file_read(VM_DEF_ARG) {
	va_default_context * defctx;
	char * pbuffer;
	vm_object * vfile = vm_get_argument(VM_ARG, 0);
	int length = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	if(vfile->len == 0) return;
	defctx = (va_default_context *)vfile->bytes;
	pbuffer = malloc(length + 1);
	length = fread(pbuffer, 1, length, defctx->ctx);
	vm_object * ret = vm_create_object (VM_ARG, length, pbuffer);
	vm_set_retval(ret);
	free(pbuffer);
}

void va_file_write(VM_DEF_ARG) {
	va_default_context * defctx;
	char * pbuffer;
	int length;
	vm_object * vfile = vm_get_argument(VM_ARG, 0);
	vm_object * vbuffer = vm_get_argument(VM_ARG, 1);
	if(vfile->len == 0) return;
	defctx = (va_default_context *)vfile->bytes;
	//pbuffer = malloc(length);
	length = fwrite(vbuffer->bytes, 1, vbuffer->len, defctx->ctx);
	va_return_word(VM_ARG, length);
	//free(pbuffer);
}

void va_file_close(VM_DEF_ARG) {
	va_default_context * defctx;
	char * pbuffer;
	int length;
	vm_object * vfile = vm_get_argument(VM_ARG, 0);
	int vbuffer = vm_get_argument(VM_ARG, 1);
	if(vfile->len == 0) return;
	defctx = (va_default_context *)vfile->bytes;
	fclose(defctx->ctx);
}

void va_fopen(VM_DEF_ARG) _REENTRANT_ {
	//dummy
	//printf("dummy function executed\n");
	//vm_invoke_exception(VM_ARG, VX_UNIMPLEMENTED_APIS);
	va_default_context defctx;
	vm_object * vpath = vm_get_argument(VM_ARG, 0);
	defctx.ctx = fopen(vpath->bytes, "rwb");
	defctx.read = va_file_read;
	defctx.write = va_file_write;
	defctx.seek = va_file_seek;
	defctx.close = va_file_close;
	vm_set_retval(vm_create_object (VM_ARG, sizeof(va_default_context), &defctx));
}
#endif	//file APIs

void va_seek(VM_DEF_ARG) _REENTRANT_ {
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	switch (obj->type) {
	case VM_OBJ_TYPE_INTERFACE:
		vm_push_stack_arc(VM_ARG, vm_get_argument(VM_ARG, 1));			//offset
		obj = vm_interface_exec_sta(VM_ARG, 1, obj, _RECAST(uint8 *, "seek"));
		if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
		vm_set_retval(obj);
		break;
	default:
		if (((va_default_context *)obj->bytes)->seek != NULL) {
			((va_default_context *)obj->bytes)->seek(VM_ARG);
		}
		break;
	}
}

void va_read(VM_DEF_ARG) _REENTRANT_ {
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	switch (obj->type) {
	case VM_OBJ_TYPE_INTERFACE:
		vm_push_stack_arc(VM_ARG, vm_get_argument(VM_ARG, 1));			//num of bytes
		obj = vm_interface_exec_sta(VM_ARG, 1, obj, _RECAST(uint8 *, "read"));
		if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
		vm_set_retval(obj);
		break;
	default:
		if (((va_default_context *)obj->bytes)->read != NULL) {
			((va_default_context *)obj->bytes)->read(VM_ARG);
		}
		break;
	}
}

void va_write(VM_DEF_ARG) _REENTRANT_ {
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	switch (obj->type) {
	case VM_OBJ_TYPE_INTERFACE:
		vm_push_stack_arc(VM_ARG, vm_get_argument(VM_ARG, 1));			//data bytes
		obj = vm_interface_exec_sta(VM_ARG, 1, obj, _RECAST(uint8 *, "write"));
		if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
		vm_set_retval(obj);
		break;
	default:
		if (((va_default_context *)obj->bytes)->write != NULL) {
			((va_default_context *)obj->bytes)->write(VM_ARG);
		}
		break;
	}
}

void va_close(VM_DEF_ARG) _REENTRANT_ {
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	switch (obj->type) {
	case VM_OBJ_TYPE_INTERFACE:
		obj = vm_interface_exec_sta(VM_ARG, 0, obj, _RECAST(uint8 *, "close"));
		if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
		vm_set_retval(obj);
		break;
	default:
		if (((va_default_context *)obj->bytes)->close != NULL) {
			((va_default_context *)obj->bytes)->close(VM_ARG);
		}
		break;
	}
}

static const char bin2hexchar[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
uint32 vm_bin2hex(uint8 * bytes, uint32 len, uint8 * hexstring) {
	uint32 j;
	for(j=0;j<len;j++) {
		hexstring[j << 1] = bin2hexchar[bytes[j] >> 4];
		hexstring[(j << 1) + 1] = bin2hexchar[bytes[j] & 0x0F];
	}
	hexstring[len << 1] = 0;
	return len << 1;
}

void va_bin2hex(VM_DEF_ARG) _REENTRANT_ { 
	//OS_DEBUG_ENTRY(va_bin2hex);
	vm_object * param;
	//uint8 i;
	//uint8 bbuf[VA_OBJECT_MAX_SIZE];
	//if((param = vm_get_argument(VM_ARG, 0)) != (vm_object *)VM_NULL_OBJECT) {
	param = vm_get_argument(VM_ARG, 0);
	//printf("len %d\r\n", param->len);
	//if(param->len < (VA_OBJECT_MAX_SIZE / 2)) {		//return;
	vm_set_retval(vm_create_immutable_object(VM_ARG, param->len * 2, NULL));
	//fill hexstring
	vm_bin2hex(param->bytes, param->len, vm_get_retval()->bytes);
	//}	
	//OS_DEBUG_EXIT();
	return;
}

static uint8 tk_hex2byte(uint8 hexchar) {
	if(hexchar >= 'a' && hexchar <= 'f') return (hexchar - 'a') + 10;
	if(hexchar >= 'A' && hexchar <= 'F') return (hexchar - 'A') + 10;
	if(hexchar >= '0' && hexchar <= '9') return hexchar - '0';
	return 0;
}

uint32 tk_hex2bin(uint8 * hexstring, uint8 * bytes, uint32 hexlen) {
	uint32 i = 0;
	uint8 c;
	uint32 len=0;
	while(hexstring[i] != 0 && i < hexlen) {
		if(i & 0x01) {
			c <<= 4;
			c |= tk_hex2byte(hexstring[i]);
			bytes[len] = c;
			len++;
		} else {
			c = tk_hex2byte(hexstring[i]);
		}
		i++;
	}
	return len;
}

void va_hex2bin(VM_DEF_ARG) _REENTRANT_ { 
	//OS_DEBUG_ENTRY(va_hex2bin);
	vm_object * param;
	//uint8 i;
	//uint8 b;
	uint32 len;
	//uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint8 * bbuf;
	param = vm_get_argument(VM_ARG, 0);
	bbuf = (uint8 *)malloc((param->len / 2) + 2);
	if (bbuf == NULL) return;
	//if((param = vm_get_argument(VM_ARG, 0)) != (vm_object *)VM_NULL_OBJECT) {
		//len = param->len / 2;
	//memcpy(g_baOrbBuffer, param->bytes, param->len);
	//tkHex2Bin(g_baOrbBuffer, param->len, param->len);
	len = tk_hex2bin(param->bytes, bbuf, param->len);
	vm_set_retval(vm_create_object(VM_ARG, len, bbuf));
	free(bbuf);
	//}
	//OS_DEBUG_EXIT();
	return;
}

void va_bin2dec(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_bin2dec);
	vm_object * param;
	uint8 buffer[6];
	int32 d = 0;
	uint8 i;
	//if((param = vm_get_argument(VM_ARG, 0)) != (vm_object *)VM_NULL_OBJECT) {
	param = vm_get_argument(VM_ARG, 0);
	if(param->len <= 2) {	//return;
		for(i=param->len;i>0;i--) {
			d *= 0x100;
			d += param->bytes[i-1];
		}
		va_return_word(VM_ARG, (int32)d);
	}
	//OS_DEBUG_EXIT();
	return;
}

void va_b64_encode(VM_DEF_ARG) _REENTRANT_ {	
	//OS_DEBUG_ENTRY(va_b64_encode);
	vm_object * param;
  	uint32 out_len = 0;
  	int i = 0;
  	int j = 0;
  	uint8 char_array_3[3];
  	uint8 char_array_4[4];	 
	uint8 * bytes_to_encode;
	uint32 in_len;
	//uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint8 * bbuf;

	CONST uint8 base64_chars[] = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/"; 
	param = vm_get_argument(VM_ARG, 0);
	bytes_to_encode = param->bytes;
	in_len = param->len;
	bbuf = (uint8 *)malloc(((param->len + 4) * 4) / 3);
	if (bbuf == NULL) return;

  	while (in_len--) {
    	char_array_3[i++] = *(bytes_to_encode++);
    	if (i == 3) {
      	char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      	char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      	char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      	char_array_4[3] = char_array_3[2] & 0x3f;

      	for(i = 0; (i <4) ; i++)
      	  	bbuf[out_len++] = base64_chars[char_array_4[i]];
      	i = 0;
    	}
  	}

  	if (i)
  	{
    	for(j = i; j < 3; j++)
      		char_array_3[j] = '\0'; 
    	char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    	char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    	char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    	char_array_4[3] = char_array_3[2] & 0x3f;

    	for (j = 0; (j < i + 1); j++)
      		bbuf[out_len++] = base64_chars[char_array_4[j]];

    	while((i++ < 3))
      		bbuf[out_len++] = '=';
  	}
  	//g_baOrbBuffer[out_len] = 0;		//EOS
  	vm_set_retval(vm_create_object(VM_ARG, out_len, bbuf));		//return decimal value
	free(bbuf);
	//OS_DEBUG_EXIT();
}

static uint8 va_pr2six(uint8 ch) {
	if ((ch >= 'A') && (ch <= 'Z')) ch = ch - 'A';
	else if ((ch >= 'a') && (ch <= 'z')) ch = ch - 'a' + 26;	
	else if ((ch >= '0') && (ch <= '9')) ch = ch - '0' + 52;
	else if (ch == '+') ch = 62;	
	else if (ch == '=') ch = 64;
	else if (ch == '/') ch = 63;
	else ch = 64;
	return ch;
}

void va_b64_decode(VM_DEF_ARG) _REENTRANT_ {	
	//OS_DEBUG_ENTRY(va_b64_decode);
	vm_object * param;
	uint32 nbytesdecoded;
    register uint8 *bufin;
    register uint8 *bufout;
    register uint8 nprbytes;
	//uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint8 * bbuf;
	param = vm_get_argument(VM_ARG, 0);
	  
    nprbytes = param->len;
	nbytesdecoded = ((nprbytes + 3) / 4) * 3; 
	bbuf = (uint8 *)malloc(nbytesdecoded + 5);
	if (bbuf == NULL) return;

    bufout = (uint8 *) bbuf;
    bufin = (uint8 *) param->bytes;

    while (nprbytes > 4) {
		*(bufout++) = (uint8) (va_pr2six(*bufin) << 2 | va_pr2six(bufin[1]) >> 4);
		*(bufout++) = (uint8) (va_pr2six(bufin[1]) << 4 | va_pr2six(bufin[2]) >> 2);
		*(bufout++) = (uint8) (va_pr2six(bufin[2]) << 6 | va_pr2six(bufin[3]));
		bufin += 4;
		nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) *(bufout++) = (uint8) (va_pr2six(*bufin) << 2 | va_pr2six(bufin[1]) >> 4);
    if (nprbytes > 2) *(bufout++) = (uint8) (va_pr2six(bufin[1]) << 4 | va_pr2six(bufin[2]) >> 2);
    if (nprbytes > 3) *(bufout++) = (uint8) (va_pr2six(bufin[2]) << 6 | va_pr2six(bufin[3]));

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    //return nbytesdecoded;  
  	vm_set_retval(vm_create_object(VM_ARG, nbytesdecoded, bbuf));		//return decimal value
	free(bbuf);
	//OS_DEBUG_EXIT();
}

CONST struct va_crypto_codec {
 	uint8 * name;
	uint8 mode;
	uint8 keylen;
} g_vaRegisteredCodec[] = { 
	{ (uint8 *)"DES", CR_MODE_DES, 8 }, 
	{ (uint8 *)"DES2", CR_MODE_DES2, 16 },
	{ (uint8 *)"DES3", CR_MODE_DES3, 24 },
	{ (uint8 *)"AES", CR_MODE_AES, 16 },	
	{ NULL, CR_MODE_DES, 0 },
};

//uint8 p_cryp_buffer[256];		//use static buffer for enciphering/deciphering
void va_crypto_create(VM_DEF_ARG) _REENTRANT_ {	
	//param[0] = codec 0 = "DES", 1 = "DES2", 2 = "DES3", 3 = "AES"
	//param[1] = mode  "CBC" / null
	//param[2] = key	
	//OS_DEBUG_ENTRY(va_crypto_create);
	vm_object * param;
	vm_object * key;
	uint32 len;	 
	uint8 mode = 0;			//default DES
	uint16 keylen = 8;		//8 bytes per block
	uint32 objLen;
	//VACryptoContext crCtx;
	cr_context_p pctx;
	struct va_crypto_codec * c_iterator = (struct va_crypto_codec *)&g_vaRegisteredCodec[0];  
 	if(vm_get_argument_count(VM_ARG) >= 3) {		//return;	
		//check parameter 0, supported codec
		param = vm_get_argument(VM_ARG, 0);
		while(c_iterator->name != NULL) {	 
			if(vm_imemcmp(param->bytes, c_iterator->name, param->len) == 0) { mode = c_iterator->mode; keylen=c_iterator->keylen; break; }
			c_iterator++;
		}
		if(c_iterator->name == NULL) {
			vm_invoke_exception(VM_ARG, VX_UNIMPLEMENTED_APIS);
			goto exit_crypto_create;
		}
		key = vm_get_argument(VM_ARG, 2);
		keylen = key->len;
		//case of des, could switch to different mode by calculating key length
		if(mode == CR_MODE_DES) {
			if(keylen <=8) {
				mode = CR_MODE_DES;
			} else if(keylen <=16) {
				mode = CR_MODE_DES2;
			} else if(keylen <=24) {
				mode = CR_MODE_DES3;
			} else {
				vm_invoke_exception(VM_ARG, VX_UNIMPLEMENTED_APIS);
				goto exit_crypto_create;
			}
		}
		//check codec mode ECB/CBC
		param = vm_get_argument(VM_ARG, 1);
		if(vm_imemcmp(param->bytes, "CBC", param->len) == 0) mode |= CR_MODE_CBC;
		objLen = sizeof(cr_context) + keylen;
		vm_set_retval(vm_create_object(VM_ARG, objLen + 256, NULL));	  
		if(vm_get_retval() == NULL) goto exit_crypto_create;		//not enough memory, cannot allocate object
		pctx = ((cr_context *)vm_get_retval()->bytes);
		vm_memset(pctx, 0, objLen);					//clear context first before initializing
		cr_init_context(pctx, (uint8 *)vm_get_retval()->bytes + objLen);			//changed to context on 2015.06.14
		pctx->mode = mode;
		//check icv  
		param = vm_get_argument(VM_ARG, 3);
		if(param->len != 0) vm_memcpy(pctx->icv, param->bytes, (param->len > CR_ICV_SIZE)?CR_ICV_SIZE:param->len);
		//set context key pointer
		pctx->key = (BYTE*)pctx + sizeof(cr_context);
		if(key->len < keylen) keylen = key->len;
		vm_memcpy(pctx->key, key->bytes, keylen); 	
	}
	exit_crypto_create:
	//OS_DEBUG_EXIT();
	return;
}

static void va_crypto_do(VM_DEF_ARG, BYTE mode) _REENTRANT_ { 	
	//OS_DEBUG_ENTRY(va_crypto_do);
	vm_object * param;
	vm_object * ctxt;
	uint32 len;
	uint8 * proc_buffer;
	cr_context pctx;
	//CRContext crCtx;	
 	if(vm_get_argument_count(VM_ARG) >= 2) {		//return;	
		//check parameter 0, crypto handler
		param = vm_get_argument(VM_ARG, 0);
		ctxt = vm_get_argument(VM_ARG, 1);
		if(ctxt->len > 224) {
			vm_invoke_exception(VM_ARG, VX_OUT_OF_BOUNDS);
			goto exit_crypto_do;
		}
		proc_buffer = (uint8 *)((cr_context *)param->bytes)->handle;
		if(proc_buffer == NULL) goto exit_crypto_do;
		vm_memset(proc_buffer, 0, (ctxt->len + 16) & 0xF0);
		vm_memcpy(proc_buffer, ctxt->bytes, ctxt->len);
		((cr_context *)param->bytes)->mode = mode | (((cr_context *)param->bytes)->mode & 0x7F);
		vm_memcpy(&pctx, (cr_context *)param->bytes, sizeof(cr_context));
		len = cr_do_crypt(&pctx, 0, ctxt->len);
		vm_set_retval( vm_create_object(VM_ARG, len, proc_buffer));		//return decimal value
	}
	exit_crypto_do:
	//OS_DEBUG_EXIT();
	return;
}

void va_crypto_encrypt(VM_DEF_ARG) _REENTRANT_ {
 	va_crypto_do(VM_ARG, CR_MODE_ENCRYPT);
}

void va_crypto_decrypt(VM_DEF_ARG) _REENTRANT_ { 
 	va_crypto_do(VM_ARG, CR_MODE_DECRYPT);
}

void va_random(VM_DEF_ARG) _REENTRANT_ {	   
	//OS_DEBUG_ENTRY(va_random);
	//uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint32 len = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 0));			//added 2016.06.08
	uint8 * bbuf = (uint8 *)malloc(len);
	if (bbuf == NULL) return;
	if(len != 0) {		//return;
		cr_randomize(bbuf, len);
		vm_set_retval(vm_create_object(VM_ARG, len, bbuf));		//return decimal value
	}
	free(bbuf);
	//OS_DEBUG_EXIT();
}

CONST struct va_crypto_digest {
 	BYTE * name;
	uint8 (* exec)(cr_context_p, uint16, uint16, uint8 *);
	uint8 size;
} g_vaRegisteredDigest[] = { 
	{ (uint8 *)"CRC32", (uint8 (*)(cr_context_p, uint16, uint16, uint8 *))cr_calc_crc, 4 }, 
	{ (uint8 *)"MD5", cr_calc_md5, 16 },
	{ (uint8 *)"SHA1", cr_calc_sha1, 20 },
	{ (uint8 *)"SHA256", cr_calc_sha256, 32 },
	{ (uint8 *)"LRC", cr_calc_lrc, 1 },	
	{ NULL, NULL, 0 },
};

void va_digest(VM_DEF_ARG) _REENTRANT_  {
	//OS_DEBUG_ENTRY(va_digest);
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint8 len = 0;
	vm_object * param;
	vm_object * mode;
	cr_context crx;
	uint8 hashlen;
	uint8 (* exec)(cr_context_p, uint16, uint16, uint8 *);
	struct va_crypto_digest * c_iterator = (struct va_crypto_digest *)&g_vaRegisteredDigest[0]; 
	param = vm_get_argument(VM_ARG, 0);
	mode = vm_get_argument(VM_ARG, 1);
	while(c_iterator->name != NULL) {	 
		if(vm_imemcmp(mode->bytes, c_iterator->name, mode->len) == 0) { 
			exec = c_iterator->exec; 
			hashlen = c_iterator->size; 
			break; 
		}
		c_iterator++;
	}
	if(c_iterator->name == NULL) {
		vm_invoke_exception(VM_ARG, VX_UNIMPLEMENTED_APIS);
		goto exit_digest;
	}
	cr_init_context(&crx, param->bytes);
	exec(&crx, 0, param->len, bbuf);
	//BYTE len = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 0));			//added 2016.06.08
	//if(len != 0) {		//return;
		//cr_randomize(bbuf, len);
	vm_set_retval(vm_create_object(VM_ARG, hashlen, bbuf));		//return decimal value
	//}
	//OS_DEBUG_EXIT();
	exit_digest:
	return;
}

#if 0 		//DEPRECATED APIS
static void va_crypto_operation(VM_DEF_ARG, BYTE mode) _REENTRANT_ {
	vm_object * param;
	vm_object * key;
	uint8 len;	 
	//BYTE plen;
	CRContext crCtx;
	if(vm_get_argument_count(VM_ARG) < 3) return;
	//if((param = vm_get_argument(VM_ARG, 0)) != (vm_object *)VM_NULL_OBJECT) {
	mode |= (va_o2f(VM_ARG, vm_get_argument(VM_ARG, 0)) & 0x3F);		//0=DES,1=DES2key,2=DES3Key,3=AES			
	//}
	//if((key = vm_get_argument(VM_ARG, 1)) != (vm_object *)VM_NULL_OBJECT) {			//get cryptokey
	key = vm_get_argument(VM_ARG, 1);
	//mode = va_o2f(VM_ARG, param);	
	//if((param = vm_get_argument(VM_ARG, 2)) == (vm_object *)VM_NULL_OBJECT) return;
	param = vm_get_argument(VM_ARG, 2);
	//start operation in memory	
	//len = param->len;
	//len = ((param->len + 16) & 0xF0);
	vm_memset(g_baOrbBuffer, 0, ((param->len + 16) & 0xF0));		 
	vm_memcpy(g_baOrbBuffer, param->bytes, param->len);
	crInitContext(&crCtx, g_baOrbBuffer);			   //changed 2015.06.14
	crCtx.mode = mode;
	crCtx.key = key->bytes;
	//len = crMemOperation(mode, key->bytes, param->bytes, param->len, g_baOrbBuffer);
	len = crDoCrypt(&crCtx, 0, param->len);
	vm_set_retval(vm_create_object(len, g_baOrbBuffer));		//return decimal value		
}

//codec
void va_encrypt_ecb(VM_DEF_ARG) _REENTRANT_ {
 	va_crypto_operation(VM_ARG, CR_MODE_ENCRYPT | CR_MODE_ECB);	
}

void va_decrypt_ecb(VM_DEF_ARG) _REENTRANT_ {
	va_crypto_operation(VM_ARG, CR_MODE_DECRYPT | CR_MODE_ECB);
} 

void va_encrypt_cbc(VM_DEF_ARG) _REENTRANT_ {
 	va_crypto_operation(VM_ARG, CR_MODE_ENCRYPT | CR_MODE_CBC);	
}

void va_decrypt_cbc(VM_DEF_ARG) _REENTRANT_ {
	va_crypto_operation(VM_ARG, CR_MODE_DECRYPT | CR_MODE_CBC);
}
#endif			//END DEPRECATED APIS

#if STACK_TOOLKIT_SELECT_APIS
void va_select_item(VM_DEF_ARG) _REENTRANT_ {	   //SELECT ITEM
	vm_object * param;
	uint8 i = 0, j;
	uint8 len;					
	//check if qualifier contain DCS
	//i = tkPrintf("cd", STK_CMD_SELECT_ITEM, 0, STK_DEV_ME);
	i = tkPushHeaderB(STK_CMD_SELECT_ITEM, 0, STK_DEV_ME);
	param = vm_get_argument(VM_ARG, 0);
	if(param->len != 0) {
		len = vrConvertText(param->bytes, param->len, g_baOrbBuffer + i, TK_BUFFER_SIZE - i);
		i = tkPushBufferB(i, STK_TAG_ALPHA, len, g_baOrbBuffer + i);	
	}
	for(j=1;j<vm_get_argument_count(VM_ARG);j++) {
		param = vm_get_argument(VM_ARG, j);	   
		g_baOrbBuffer[i] = j; 	//item id	
		len = 1;
		len += vrConvertText(param->bytes, param->len, g_baOrbBuffer + (i + 1), TK_BUFFER_SIZE - (i + 1));
		i = tkPushBufferB(i, STK_TAG_ITEM, len, g_baOrbBuffer + i);
	} 
	tkDispatchCommandW(NULL, i);
	g_vaWaitTag = STK_TAG_ITEM_ID;
	vm_set_state(VM_STATE_SUSPEND);
}
#endif

#if STACK_TOOLKIT_TIMER_APIS
void va_set_timer(VM_DEF_ARG) _REENTRANT_ {  
	FSHandle temp_fs;	   
	vm_object * param;
	//vm_context vctx; 
	TKApplicationContext actx;
	BYTE buffer[18];
	BYTE l, h;
	//BYTE dbuffer[3];
	BYTE tid;
	BYTE tag, hlen;
	WORD tsize;
	BYTE k;
	WORD index;
	WORD i, codestart;
	WORD tick;
	//set timer initial value  
	mmMemSet(buffer + 2, 0, 3);		//clear timer value to 000000
	if((param = vm_get_argument(VM_ARG, 1))	== VM_NULL_OBJECT) return;
	tick = va_o2f(VM_ARG, param);	 
	for(k=3;k!=1;k--) {
		buffer[k+1] = (tick % 60);
		tick /= 60;
	}
	buffer[2] = tick;
	for(k=3;k!=0;k--) {
		l = buffer[k+1] % 10;
		h = buffer[k+1] / 10;
		buffer[k+1] = (h << 4) | (l & 0x0F);
	}
	//timer id
	if((param = vm_get_argument(VM_ARG, 0))	!= VM_NULL_OBJECT) {
		if(param->len > 2) return;
		index = param->bytes[0];
		index <<= 8;
		index |= param->bytes[1];
	} else 
		return;
	vm_memcpy(&temp_fs, &_vm_file, sizeof(FSHandle));

	//decode ASN.1 structure for package and look for entry point for matching menu_id
	hlen = tkPopFile(&temp_fs, 0, &tag, &tsize);
	if(tag == (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) {		//check for constructed sequence
		codestart = tsize + hlen;										//total sequence length
		//re-initialize vm to execute current selected menu
		vm_memcpy(&actx, &temp_fs, sizeof(FSHandle));		//set handle to current active bytecode where this api executed
		actx.offset = codestart + index;
#if TK_MANAGER_ENABLED
		tid = tkRegisterServiceToTimer(TK_SID_STACK, sizeof(TKApplicationContext), &actx);
#else
		
#endif
		if(tid != 0) {
			i = tkPushHeaderB(STK_CMD_TIMER_MANAGEMENT, 0, STK_DEV_ME);
			i = tkPushBufferB(i, STK_TAG_TIMER_IDENTIFIER, 1, &tid);
			i = tkPushBufferB(i, STK_TAG_TIMER_VALUE, 3, buffer + 2);
			tkDispatchCommandW(NULL, i);	
			g_vaWaitTag = STK_TAG_RESULT;
			vm_set_state(VM_STATE_SUSPEND);	
		}
	}
}
#endif

#define VA_BIT_CHECK		3
#define VA_BIT_SET		   	1
#define VA_BIT_CLR			2

#if STACK_BIT_APIS
static void va_bit_operation(BYTE mode) {
	vm_object * param;
	uint16 bnum;
	uint8 offset, mask;//, res;
	//if(vm_get_argument_count(VM_ARG) <  2) return;
	param = vm_get_argument(VM_ARG, 0);
	bnum = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));		//bit number
	offset = bnum / 8;	//byte number
	mask = bnum % 8;	//mask
	mask = 1 << mask;
	if(param->len <= offset) return;
	switch(mode) {
		case VA_BIT_CHECK:
			bnum = '0';
			if(param->bytes[offset] & mask) {
				bnum = '1';	
			}
			vm_set_retval(vm_create_object(1, &bnum));
			break;
		case VA_BIT_SET:
			param->bytes[offset] |= mask;
			break;
		case VA_BIT_CLR:
			param->bytes[offset] &= ~mask;
			break;
		default: break;
	}
}

void va_check_bit(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_check_bit);
	va_bit_operation(VM_ARG, VA_BIT_CHECK);
	//OS_DEBUG_EXIT();
}

void va_set_bit(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_set_bit);
	va_bit_operation(VM_ARG, VA_BIT_SET);
	//OS_DEBUG_EXIT();
}

void va_clear_bit(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_clear_bit);
	va_bit_operation(VM_ARG, VA_BIT_CLR);
	//OS_DEBUG_EXIT();
} 
#endif

#if 0
void va_arg_findtag(VM_DEF_ARG) _REENTRANT_ {
	int len;		 
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	uint8 tag = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	//len = mmPopTlvByTag(obj->bytes, obj->len, tag, bbuf);
	len = tk_pop_by_tag(obj->bytes, obj->len, tag, bbuf);
	if(len == -1) return;
	vm_set_retval(vm_create_object(len, bbuf));
}
#endif

int va_get_length(uint8 * buffer, int * length) {
	if(buffer[0] < 128) {
		length[0] = buffer[0];
		return 1;
	} else if(buffer[0] == 0x81) {
		length[0] = buffer[1];
		return 2;
	} else if(buffer[0] == 0x82) {
		length[0] = (buffer[1] << 8 | buffer[2]);
		return 3;
	} else if (buffer[0] == 0x83) {
		length[0] = ((int)buffer[1] << 16) | ((int)buffer[2] << 8) | (int)buffer[3];
		return 3;
	}
	return 0;
}

int va_pop_lv(uint8 * buffer, uint8 * dst, int * length) {
	if(buffer[0] < 128) {
		length[0] = buffer[0];
		memcpy(dst, buffer + 1, length[0]);
		return 1 + length[0];
	} else if(buffer[0] == 0x81) {
		length[0] = buffer[1];
		memcpy(dst, buffer + 2, length[0]);
		return 2 + length[0];
	} else if(buffer[0] == 0x82) {
		length[0] = (buffer[1] << 8 | buffer[2]);
		memcpy(dst, buffer + 3, length[0]);
		return 3 + length[0];
	} else if (buffer[0] == 0x83) {
		length[0] = ((int)buffer[1] << 16) | ((int)buffer[2] << 8) | (int)buffer[3];
		memcpy(dst, buffer + 4, length[0]);
		return 4 + length[0];
	}
	return 0;
}

int va_push_lv(uint8 * buffer, uint8 * src, uint16 length) {
	int llen = 0;
	if(length < 128) {
		vm_memcpy(buffer + 1, src, length);
		buffer[0] = length;
		llen = 1 + length;
	} else if(length < 255) {
		vm_memcpy(buffer + 2, src, length);
		buffer[0] = 0x81;
		buffer[1] = length;
		llen = 2 + length;
	} else if(length < 65535) {
		vm_memcpy(buffer + 3, src, length);
		buffer[0] = 0x82;
		buffer[1] = length >> 8;
		buffer[2] = length & 0xFF;
		llen = 3 + length;
	} else {
		vm_memcpy(buffer + 4, src, length);
		buffer[0] = 0x83;
		buffer[1] = length >> 16;		//might loss
		buffer[2] = length >> 8;
		buffer[3] = length & 0xFF;
		llen = 4 + length;
	}
	return llen;
}

typedef struct va_arg_enumerator {
	vm_object * obj;
	uint16 index;
	uint16 length;
} va_arg_enumerator;	

void * va_arg_enumerate(vm_object * obj) {
	int len, llen;
	va_arg_enumerator * enumerator;
	if(obj == NULL) return NULL;
 	if(obj->len == 0) return NULL;
	if(obj->bytes[0] != ASN_TAG_SET && obj->bytes[0] != ASN_TAG_SEQ ) return NULL;
	enumerator = (va_arg_enumerator *)malloc(sizeof(va_arg_enumerator));
	enumerator->obj = obj;
	llen = va_get_length(obj->bytes + 1, &len);
	enumerator->index = llen;
	enumerator->length = len;
	return enumerator;
}

uint8 * va_arg_next(void * enumerator, uint8 * key, uint16 * length, uint8 * value) {
	int len, llen;
	uint8 * ret = NULL;
	uint8 klen = 0;
	uint8 c;
	va_arg_enumerator * iterator = (va_arg_enumerator *)enumerator;
	if(enumerator == NULL) return NULL;
	if(iterator->index >= iterator->length) return NULL;
	if(iterator->obj->bytes[iterator->index] == ASN_TAG_OCTSTRING) {
		llen = va_get_length(iterator->obj->bytes + iterator->index + 1, &len);
		ret = iterator->obj->bytes + iterator->index + llen + 1;
		key[0] = 0;
		length[0] = len;
		iterator->index += len + llen +1;
	} else if(iterator->obj->bytes[iterator->index] == ASN_TAG_OBJDESC) {
		llen = va_get_length(iterator->obj->bytes + iterator->index + 1, &len);
		ret = iterator->obj->bytes + iterator->index + llen + 1;
		length[0] = len;
		while((c = *ret++) != VA_OBJECT_DELIMITER) {
			*key++ = c;
			length[0] --;
		}
		length[0] --;
		key[0] = 0;
		iterator->index += len + llen +1;
	} else {
		llen = va_get_length(iterator->obj->bytes + iterator->index + 1, &len);
		ret = iterator->obj->bytes + iterator->index;
		length[0] = len + llen + 1;
		key[0] = 0;
		iterator->index += len + llen +1;
	}
	len = (length[0] > VA_OBJECT_MAX_SIZE)?(VA_OBJECT_MAX_SIZE - 1): length[0];
	memcpy(value, ret, len);
	value[len] = 0;
	return ret;
}

void va_arg_end(void * enumerator) {
	free(enumerator);
}

int va_arg_count_t(uint8 * bytes, int wlen) {
	uint16 cntr = 0;
	uint8 tag;
	int i, tlen, len, llen;
	if (wlen == 0) return 0;
	if (bytes[0] != ASN_TAG_SET && bytes[0] != ASN_TAG_SEQ) return 0;		//invalid array/object mark	
	llen = va_get_length(bytes + 1, &tlen);
	tlen += (llen + 1);
	for (i = (llen + 1);i<tlen;cntr++) {
		tag = bytes[i];
		llen = va_get_length(bytes + i + 1, &len);
		i += (len + llen + 1);
	}
	return cntr;
}

int va_arg_count_s(vm_object * obj) {
	return va_arg_count_t(obj->bytes, obj->len);
}

void va_arg_count(VM_DEF_ARG) _REENTRANT_ {
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	switch (obj->type) {
	case VM_OBJ_TYPE_INTERFACE:
		obj = vm_interface_exec_sta(VM_ARG, 0, obj, _RECAST(uint8 *, "count"));
		if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
		vm_set_retval(obj);
		break;
	default:
		va_return_word(VM_ARG, va_arg_count_s(obj));
		break;
	}
}

uint16 va_num_text(VM_DEF_ARG, vm_object *, uint8 *);
vm_object * va_num_add(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_num_mul(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_num_div(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_num_sub(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_num_mod(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_num_and(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_num_or(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_num_xor(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_num_shl(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_num_shr(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_num_not(VM_DEF_ARG, vm_object *);

vm_custom_opcode num_op_table = {
	va_num_text, 
	va_num_add, 
	va_num_mul, 
	va_num_div, 
	va_num_sub,
	va_num_mod,
	va_num_and,
	va_num_or,
	va_num_xor,
	va_num_shl,
	va_num_shr,
	va_num_not
};

static void va_object_to_double(VM_DEF_ARG, vm_object * op, double * d) {
	uint8 bbuf[128];
	uint8 * pbuf = bbuf;
	if (vm_object_get_type(op) == VM_EXT_MAGIC && ((vm_extension *)op->bytes)->tag == ASN_TAG_REAL) {
		d[0] = ((double *)(((vm_extension *)op->bytes)->payload))[0];
	}
	else {
		if (op->len > sizeof(bbuf)) pbuf = (uint8 *)malloc(op->len);
		if (vm_is_precision(bbuf, vm_object_get_text(VM_ARG, op, bbuf))) {
			d[0] = atof((const char *)bbuf);
		}
		else {
			d[0] = (double)atol((const char *)bbuf);		//skip precision value
		}
		if (pbuf != bbuf) free(pbuf);
	}
}

uint16 va_num_text(VM_DEF_ARG, vm_object * op, uint8 * text) { 
	if(vm_object_get_type(op) == VM_EXT_MAGIC) {
		sprintf(_RECAST(char *, text), "%f", ((double *)((vm_extension *)op->bytes)->payload)[0]);
  	}
 	return strlen(_RECAST(const char *, text));
}

vm_object * va_num_add(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
	va_object_to_double(VM_ARG, op2, &d2);
 	d1 = d1 + d2;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_num_mul(VM_DEF_ARG, vm_object * op1, vm_object * op2) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
	va_object_to_double(VM_ARG, op2, &d2);
 	d1 = d1 * d2;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_num_div(VM_DEF_ARG, vm_object * op1, vm_object * op2) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
	va_object_to_double(VM_ARG, op2, &d2);
 	d1 = d1 / d2;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_num_mod(VM_DEF_ARG, vm_object * op1, vm_object * op2) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
	va_object_to_double(VM_ARG, op2, &d2);
 	d1 = (int64)d1 % (int64)d2;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_num_sub(VM_DEF_ARG, vm_object * op1, vm_object * op2) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
	va_object_to_double(VM_ARG, op2, &d2);
 	d1 = d1 - d2;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_num_and(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
	va_object_to_double(VM_ARG, op2, &d2);
 	d1 = (int)d1 & (int)d2;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_num_or(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
	va_object_to_double(VM_ARG, op2, &d2);
 	d1 = (int)d1 | (int)d2;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_num_xor(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
	va_object_to_double(VM_ARG, op2, &d2);
 	d1 = (int)d1 ^ (int)d2;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_num_shl(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
	va_object_to_double(VM_ARG, op2, &d2);
	d1 = (int)d1 << (int)d2;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_num_shr(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
	va_object_to_double(VM_ARG, op2, &d2);
	d1 = (int)d1 >> (int)d2;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_num_not(VM_DEF_ARG, vm_object *op1) {
	double d1, d2;
	va_object_to_double(VM_ARG, op1, &d1);
 	d1 = !d1;
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&d1);
}

vm_object * va_create_ext_float(VM_DEF_ARG, uint16 length, uint8 * buffer) {
	double f;
	uint8 bbuf[64];
	if(length > 64) return VM_NULL_OBJECT;
	memcpy(bbuf, buffer, length);
	bbuf[length] = 0;
	f = atof(_RECAST(const char *, bbuf));
	return vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&f);
}

void va_to_float(VM_DEF_ARG) _REENTRANT_ {
	//uint16 len = strlen((const char *)key) + strlen((const char *)value) + 2;
	//OS_DEBUG_ENTRY(va_arg_create);
	double f;
	vm_object * obj;
	vm_object * text = vm_get_argument(VM_ARG, 0);
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint16 len = text->len;
	uint16 llen;
	if(len > (VA_OBJECT_MAX_SIZE - 3)) return ;
	vm_memcpy(bbuf, text->bytes, text->len);
	bbuf[text->len] = 0; 
	f = atof(_RECAST(const char *, bbuf));
	vm_set_retval(vm_create_extension(VM_ARG, ASN_TAG_REAL, &num_op_table, sizeof(double), (uint8 *)&f));

	//OS_DEBUG_EXIT();
}
  
void va_arg_create(VM_DEF_ARG) _REENTRANT_ {
	//uint16 len = strlen((const char *)key) + strlen((const char *)value) + 2;
	//OS_DEBUG_ENTRY(va_arg_create);
	vm_object * obj;
	vm_object * key = vm_get_argument(VM_ARG, 0);
	vm_object * val = vm_get_argument(VM_ARG, 1);
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint16 len = 0;
	uint16 llen;
	if(len > (VA_OBJECT_MAX_SIZE - 3)) return ;

	//bbuf[0] = ASN_TAG_OBJDESC;	  
	//bbuf[1] = len - 2;
	//vm_memcpy(bbuf, key->bytes, key->len);
	len += vm_object_get_text(VM_ARG, val, bbuf);
	bbuf[len++] = VA_OBJECT_DELIMITER; 
	//vm_memcpy(bbuf + key->len + 1, val->bytes, val->len);
	len += vm_object_get_text(VM_ARG, val, bbuf + len);
	llen = va_push_lv(bbuf + 1, bbuf, len);
	bbuf[0] = ASN_TAG_OBJDESC;
	
	vm_set_retval(vm_create_arg(VM_ARG, llen + 1, bbuf));

	//OS_DEBUG_EXIT();
}

void va_arg_object(VM_DEF_ARG) _REENTRANT_ {
	//va_list ap;
	//OS_DEBUG_ENTRY(va_arg_object);
    	uint8 j;
	uint16 llen;
	vm_object * obj, * ibj;
	int len = 0;
	//uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint8 * bbuf;
    //double sum = 0;
	uint8 count =  vm_get_argument_count(VM_ARG);
	//cid = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 0));

    //va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
    for (j = 0; j < count; j++) {
        //obj = va_arg(ap, vm_object *);		/* Increments ap to the next argument. */
		obj = vm_get_argument(VM_ARG, j);
		len += (obj->len + 3);			//estimation size each object
	}
	bbuf = (uint8 *)malloc(len + VA_OBJECT_MAX_SIZE);
	//if(len <= VA_OBJECT_MAX_SIZE) {		//return;
	if(bbuf != NULL) {
		//va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
		//obj = (vm_object *)malloc(len + sizeof(vm_object));
		len = 0;
		for (j = 0; j < count; j++) {
			//ibj = va_arg(ap, vm_object *);   
			ibj = vm_get_argument(VM_ARG, j);
			switch(vm_object_get_type(ibj)) {
				case VM_OBJ_MAGIC:
					//check for OWB object
					vm_memcpy(bbuf + len, ibj->bytes, ibj->len);
					len += ibj->len;
					break;
				case VM_MAGIC:
					//default string object
					llen = va_push_lv(bbuf + len + 1, ibj->bytes, ibj->len);
					bbuf[len] = ASN_TAG_OCTSTRING;
					len += (llen + 1);
					break;
				case VM_EXT_MAGIC:			//added 2018.08.30
					llen = va_push_lv(bbuf + len + 1, bbuf + len + 5, vm_object_get_text(VM_ARG, ibj, bbuf + len + 5));
					bbuf[len] = vm_ext_get_tag(ibj);
					len += (llen + 1);
					break;
			}
		}
		llen = va_push_lv(bbuf + 1, bbuf, len);
		bbuf[0] = ASN_TAG_SEQ;
		len = (llen + 1);
		//va_end(ap);
		vm_set_retval(vm_create_arg(VM_ARG, len, bbuf));
		//if(vm_get_retval()->len != 0) vm_get_retval()->mgc_refcount |= VM_OBJ_MAGIC;
	}
	free(bbuf);
	//OS_DEBUG_EXIT();
}
							   
void va_arg_array(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_array);
    uint8 j;
	vm_object * obj, * ibj;
	int len = 0, llen;
    //double sum = 0;	
	//uint8 bbuf[VA_OBJECT_MAX_SIZE];	
	uint8 * bbuf;
	uint8 count =  vm_get_argument_count(VM_ARG);

    //va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
    for (j = 0; j < count; j++) {
        //obj = va_arg(ap, vm_object *);		/* Increments ap to the next argument. */ 
		obj = vm_get_argument(VM_ARG, j);
		len += (obj->len + 3);			//estimation size each object
	}
	bbuf = (uint8 *)malloc(len + 240);
	//if(len <= VA_OBJECT_MAX_SIZE) {	//return;
	if(bbuf != NULL) {
		//va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
		//obj = (vm_object *)malloc(len + sizeof(vm_object));
		len = 0;
		for (j = 0; j < count; j++) {
			//ibj = va_arg(ap, vm_object *);	  
			ibj = vm_get_argument(VM_ARG, j);
			switch(vm_object_get_type(ibj)) {
				case VM_OBJ_MAGIC:
					//check for OWB object
					vm_memcpy(bbuf + len, ibj->bytes, ibj->len);
					len += ibj->len;
					break;
				case VM_MAGIC:
					//default string object
					llen = va_push_lv(bbuf + len + 1, ibj->bytes, ibj->len);
					bbuf[len] = ASN_TAG_OCTSTRING;
					len += (llen + 1);
					break;
				case VM_EXT_MAGIC:			//added 2018.08.30
					llen = va_push_lv(bbuf + len + 1, bbuf + len + 5, vm_object_get_text(VM_ARG, ibj, bbuf + len + 5));
					bbuf[len] = vm_ext_get_tag(ibj);
					len += (llen + 1);
					break;
			}
		}
		llen = va_push_lv(bbuf + 1, bbuf, len);
		bbuf[0] = ASN_TAG_SET;
		len = (llen + 1);
		//va_end(ap);
		vm_set_retval(vm_create_arg(VM_ARG, len, bbuf));
		//if(vm_get_retval()->len != 0) vm_get_retval()->mgc_refcount |= VM_OBJ_MAGIC;
	}
	free(bbuf);
	//OS_DEBUG_EXIT();
}

void va_arg_at(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_at);
	int i, j;
	int tlen;
	int len, llen, hlen;
	int est_count;	
	uint8 * bbuf;
	uint8 tag;
	int index = -1;
	int total = 1;
	int actual_count = 0;
	vm_object * ibj;	
	int bbuf_len = 0;
	int rlen;
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	vm_object * vindex = vm_get_argument(VM_ARG, 1);
	vm_object * vtotal = vm_get_argument(VM_ARG, 2);
	if (!vm_is_numeric(vindex)) {
		//variable key type access, use get
		va_arg_get(VM_ARG);
		return;
	}
	if (vm_get_argument_count(VM_ARG) > 2) {
		total = va_o2f(VM_ARG, vtotal);
	}
	index = va_o2f(VM_ARG, vindex);
	switch(obj->type) {
		case VM_OBJ_TYPE_INTERFACE:
			vm_push_stack_arc(VM_ARG, vm_get_argument(VM_ARG, 1));
			obj = vm_interface_exec_sta(VM_ARG, 1, obj, _RECAST(uint8 *, "at"));
			//vm_pop_stack_arc(VM_ARG);
			if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
			vm_set_retval(obj);
			break;
		default:
			bbuf = (uint8 *)malloc(obj->len);
			if (obj->len != 0 && (obj->bytes[0] == ASN_TAG_SET || obj->bytes[0] == ASN_TAG_SEQ)) {
				//obj = obj->bytes;
				est_count = va_arg_count_s(obj);
				if (index < 0) index = est_count + index;		//support negative as index from last
				if (index < 0) break;						//cannot access negative index
				if (total < 0) {				//negative count, convert to positive count
					total = (est_count - index) + (total + 1);
					if (total < 0) total = 0;			//invalid total
				}
				if ((index + total) > est_count) total = est_count - index;		//bounded positive count
				if (total == 0) break;					//zero count
				hlen = va_pop_lv(obj->bytes + 1, bbuf, &tlen);
				hlen = (hlen - tlen) + 1;
				for (i = 0, j = 0;i<tlen;j++) {
					tag = obj->bytes[hlen + i++];
					llen = va_pop_lv(obj->bytes + hlen + i, bbuf + bbuf_len, &len);
					if (j >= index && j < (index + total)) {
						switch (tag) {
						case ASN_TAG_OCTSTRING:
							if (total > 1) goto push_object;
							bbuf_len = len;
							vm_set_retval(vm_create_object(VM_ARG, len, bbuf));
							break;
						case ASN_TAG_REAL:		//added 2018.08.30
							if (total > 1) goto push_object;
							bbuf_len = len;
							vm_set_retval(va_create_ext_float(VM_ARG, len, bbuf));
							break;
						default:		//either a key-value, sequence(object) or set(array) 
							push_object:
							rlen = va_push_lv(bbuf + 1 + bbuf_len, bbuf + bbuf_len, len);
							bbuf[bbuf_len] = tag;
							//vm_set_retval(vm_create_arg(VM_ARG, llen + 1, bbuf));
							//if (vm_get_retval() != VM_NULL_OBJECT) vm_get_retval()->mgc_refcount |= VM_OBJ_MAGIC;
							bbuf_len += (rlen + 1);
							actual_count++;
							//printf("push_object : %i of %i\n", actual_count, total);
							break;
						}
					}
					i += llen;
				}
			}
			if (actual_count > 1) {
				rlen = va_push_lv(bbuf + 1, bbuf, bbuf_len);
				bbuf[0] = ASN_TAG_SET;
				vm_set_retval(vm_create_arg(VM_ARG, rlen + 1, bbuf));
			}
			free(bbuf);
			break;
	}
	//OS_DEBUG_EXIT();
	//return VM_NULL_OBJECT;		//index out of bounds
}
			  
void va_arg_get(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_get);
	uint16 i, j;
	int tlen;
	int len, llen;
	uint8 * bbuf;
	uint8 tag;
	vm_object * ibj;		
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	vm_object * key = vm_get_argument(VM_ARG, 1);
	switch (obj->type) {
		case VM_OBJ_TYPE_INTERFACE:
			vm_push_stack_arc(VM_ARG, vm_get_argument(VM_ARG, 1));
			obj = vm_interface_exec_sta(VM_ARG, 1, obj, _RECAST(uint8 *, "get"));
			//vm_pop_stack_arc(VM_ARG);
			if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
			vm_set_retval(obj);
			break;
		default:
			if (obj->len != 0) {
				if (obj->bytes[0] != ASN_TAG_SET && obj->bytes[0] != ASN_TAG_SEQ) goto exit_arg_get;		//invalid array/object mark	  
				//obj = obj->bytes;
				bbuf = (uint8 *)malloc(obj->len);
				llen = va_pop_lv(obj->bytes + 1, bbuf, &tlen);
				for (i = 0, j = 0;i < tlen;j++) {
					tag = bbuf[i++];
					llen = va_pop_lv(bbuf + i, bbuf, &len);
					if (tag == ASN_TAG_OBJDESC &&
						bbuf[key->len] == VA_OBJECT_DELIMITER &&
						vm_memcmp(bbuf, key->bytes, key->len) == 0) {
						len = len - (key->len + 1);
						vm_set_retval(vm_create_arg(VM_ARG, len, bbuf + key->len + 1));
						//if (vm_get_retval() != VM_NULL_OBJECT) vm_get_retval()->mgc_refcount |= VM_OBJ_MAGIC;
						break;
					}
					i += llen;
				}
				free(bbuf);
			}
			break;
	}
	exit_arg_get:
	//OS_DEBUG_EXIT();
	return;
}

static int va_arg_serialize_s(uint8 * buffer, uint8 ** strbuf, int objlen) _REENTRANT_ {
	uint8 c;
	int j = 0;
	int wlen, llen;
	int len = 0;
	uint8 state =0;
	uint8 wc;
	uint8 is_numeric = 0;
	while(j++ < objlen && (c = *(strbuf[0])++)) {
		switch(c) {
			case ASN_TAG_SET:		//array		ASN_TAG_SET
				if(len != 0 && state == 0) buffer[len++]= ',';
				state=0;
				buffer[len++]= '[';
				//c = *(strbuf[0])++;
				//j++;
				llen = va_get_length(strbuf[0], &wlen);
				strbuf[0] += llen;
				j += llen;
				len += va_arg_serialize_s(buffer + len, strbuf, wlen);
				j += wlen;
				buffer[len++]= ']';
				break;
			case ASN_TAG_SEQ:		//object	ASN_TAG_SEQ	
				if(len != 0 && state == 0) buffer[len++]= ',';
				state=0;
				buffer[len++]= '{';
				//c = *(strbuf[0])++;
				//j++;
				llen = va_get_length(strbuf[0], &wlen);
				strbuf[0] += llen;
				j += llen;
				len += va_arg_serialize_s(buffer + len, strbuf, wlen);
				j += wlen;
				//j += c;
				buffer[len++]= '}';
				break;
			case VA_OBJECT_DELIMITER:
				buffer[len++]= ':';
				if(state == 1) {
					state++;
					//buffer[len++]= '\"';
				}
				break;
			case ASN_TAG_OBJDESC:					//ASN_TAG_OBJDESC
				if(state == 0) { 
					if(len != 0) buffer[len++]= ',';
					llen = va_get_length(strbuf[0], &wlen);
					strbuf[0] += llen;
					j += llen;
					len += va_arg_serialize_s(buffer + len, strbuf, wlen);
					j += wlen;
				} 
				break;
			case ASN_TAG_OCTSTRING:
				if(state == 0) { 
					if(len != 0) buffer[len++]= ',';
					llen = va_get_length(strbuf[0], &wlen);
					strbuf[0] += llen;
					j += llen;
					if(va_arg_is_numeric(strbuf[0], wlen)) {
						//numeric doesn't need for quote
						memcpy(buffer + len, strbuf[0], wlen);
						(strbuf[0]) += wlen;
						len += wlen;
						j += wlen;
					} else {
						buffer[len++] = '\"';
						memcpy(buffer + len, strbuf[0], wlen);
						(strbuf[0]) += wlen;
						len += wlen;
						j += wlen;
						buffer[len++] = '\"';
					}
				}
				break;
			case ASN_TAG_REAL:			//added 2018.08.30
				if(state == 0) { 
					if(len != 0) buffer[len++]= ',';
					llen = va_get_length(strbuf[0], &wlen);
					strbuf[0] += llen;
					j += llen;
					memcpy(buffer + len, strbuf[0], wlen);
					(strbuf[0]) += wlen;
					len += wlen;
					j += wlen;
				}
				break;
			case ' ': 
				if(state == 0) break;
				if(state == 4) break;
			default:
				if(state == 2 || state == 0) {
					state++;
					if(state == 2 && c >='0' && c <='9') {
						state=4;		//numeric state
					} else {
						//check for delimiter ahead
						if(!va_contain_delimiter(strbuf[0]-1, objlen-(j-1))) {
							is_numeric = 0;
							if(!va_arg_is_numeric(strbuf[0]-1, objlen-(j-1)))
								buffer[len++]= '\"';
							else 
								is_numeric = 1;
						}
					}
				}
				switch(c) {
					case '\"': wc = '\"'; goto push_spchar;
					case '\\': wc = '\\'; goto push_spchar;
					case '/' : wc = '/'; goto push_spchar;
					case '\b': wc = 'b'; goto push_spchar;
					case '\f': wc = 'f'; goto push_spchar;
					case '\n': wc = 'n'; goto push_spchar;
					case '\r': wc = 'r'; goto push_spchar;
					case '\t': wc = 't'; goto push_spchar;
						push_spchar:
						buffer[len++]= '\\';
						buffer[len++]= wc;
						break;
					default:
						buffer[len++]= c;
						break;
				}
				break;
		}
	}
	if((state & 0x01) != 0 && is_numeric == 0) buffer[len++]= '\"';
	return len;
}

void va_arg_serialize(VM_DEF_ARG) _REENTRANT_ {			// -> to json string
	//OS_DEBUG_ENTRY(va_arg_serialize);
	int dlen;
	//uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint8 * bbuf;
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	uint8 * tptr = obj->bytes;
	bbuf = (uint8 *)malloc(VA_OBJECT_MAX_SIZE + obj->len);		//TODO: need to estimate bbuf size better
	if((obj->mgc_refcount & VM_MAGIC_MASK) == VM_OBJ_MAGIC) {		//return;		//should be OWB object
		dlen = va_arg_serialize_s(bbuf, &tptr, obj->len);	 
		vm_set_retval(vm_create_object(VM_ARG, dlen, bbuf));
	}
	free(bbuf);
	//OS_DEBUG_EXIT();
}

char * vm_object_to_jsonstring(char * content) {
	int dlen;
	uint8 * bbuf = NULL;
	uint8 * tptr = content;
	int len, llen;
	//if((obj->mgc_refcount & VM_MAGIC_MASK) == VM_OBJ_MAGIC) {		//return;		//should be OWB object
	if(tptr[0] == ASN_TAG_SET || tptr[0] == ASN_TAG_SEQ || tptr[0] == ASN_TAG_OBJDESC || tptr[0] == ASN_TAG_OCTSTRING || tptr[0] == ASN_TAG_REAL) {
		llen = va_get_length(tptr + 1, &len);
		bbuf = (uint8 *)malloc(VA_OBJECT_MAX_SIZE + len);		//TODO: need to estimate bbuf size better
		if(bbuf != NULL) {
			dlen = va_arg_serialize_s(bbuf, &tptr, len);	 
			bbuf[dlen] = 0;
		}
	}
	return bbuf;
}

void vm_jsonstring_release(char * json) {
	if(json != NULL) free(json);
}

int va_arg_deserialize_s(uint8 * buffer, uint8 ** strbuf, uint16 slen, int * index) _REENTRANT_ {
	uint8 c;
	int len = 0, llen;
	int ilen;
	uint8 state =0;
	int ldx= 0;
	int klen=0;
	uint8 sv_state;
	uint8 quote=0;
	
	while(index[0]++ < slen && (c = *(strbuf[0])++)) {
		switch(c) {
			case '[':		//start array
                if (quote == 1) goto push_char;
				buffer[len++] = ASN_TAG_SET;
				ilen = va_arg_deserialize_s(buffer + len, strbuf, slen, index);
				llen = va_push_lv(buffer + len, buffer + len, ilen);
				len += llen;
				if(state >= 2) {
					klen += (llen + 1);
					va_push_lv(buffer + ldx, buffer + ldx + 1, klen); 
					state++; 
				}
				state = 0;
				break;
			case '{':		//start object
                if (quote == 1) goto push_char;
				buffer[len++] = ASN_TAG_SEQ;
				ilen = va_arg_deserialize_s(buffer + len, strbuf, slen, index);
				llen = va_push_lv(buffer + len, buffer + len, ilen);
				len += llen;
				if(state >= 2) { 
					klen += (llen + 1);
					va_push_lv(buffer + ldx, buffer + ldx + 1, klen);
					state++; 
				}
				state = 0;
				break;
			case '}':		//end object
			case ']':		//end array
                if (quote == 1) goto push_char;		//inside quote
				if(klen != 0) buffer[ldx] = klen;		//
			case 0:			//end string
				return len;
			case '\"':		//start key-value
				if(quote == 1) {
					if(len != 0 && buffer[len-1] == '\\') goto push_char;
				}
				switch(state) {
					case 0: buffer[len++]=ASN_TAG_OBJDESC; ldx = len++; buffer[ldx] = 0; klen = 0; quote =1; break;
					case 1: buffer[ldx] = klen; quote=0; break;
					case 2: quote = 1; break;
					case 3: buffer[ldx] = klen;  quote=0; break;
					case 4: goto reset_state;
				}
				state++;
				break;
			case ',':
				if(quote == 0) { 
					if(klen == 0) {			//case [,,]	-> double comma
						buffer[len++]=ASN_TAG_OBJDESC; 
						ldx = len++; 
					}
					buffer[ldx] = klen;
					klen = 0; 
					quote=0;
					state = 0;
					goto reset_state;
				}
				if(state != 1) goto push_char;
				break;
			case ':':
				if (state == 1) {
					buffer[ldx] = klen;
					state = 2;
				}
				if(state < 3) { c = VA_OBJECT_DELIMITER; }
			default:
				if(state == 0) { buffer[len++]=ASN_TAG_OBJDESC; ldx = len++; buffer[ldx] = 0; klen = 0; quote =0; state++; }
				if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || (c>='0' && c<='9'))	//start of key/value
				{
					if (state == 2) state++;
				}
			push_char:
				klen++;
				buffer[len++] = c;
				break;
			case ' ': 				//skip whitespace
				//if(state == 1 || state == 3) goto push_char; 		//inside quotation mark
				if(quote) { if(quote) goto push_char; }		//ignore
				if(state == 4) {  
			reset_state:
					state = 0;
				}
				if(state == 1) {  }		//ignore
				break;	//skip white space
		}
	}
	return len;
}

void va_arg_deserialize(VM_DEF_ARG) _REENTRANT_ {	//-> from json string
	//OS_DEBUG_ENTRY(va_arg_deserialize);
	int dlen;
	int t = 0;		
	uint8 bbuf[VA_OBJECT_MAX_SIZE];	
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	uint8 * tptr = obj->bytes;	   
	dlen = va_arg_deserialize_s(bbuf, &tptr, obj->len, &t);
	vm_set_retval(vm_create_arg(VM_ARG, dlen, bbuf));
	//if(vm_get_retval() != VM_NULL_OBJECT) vm_get_retval()->mgc_refcount |= VM_OBJ_MAGIC;
	//OS_DEBUG_EXIT();
}

#define VA_ARG_MODE_SET				0
#define VA_ARG_MODE_DEQUEUE			1
#define VA_ARG_MODE_POP				2
void va_arg_set_operation(VM_DEF_ARG, vm_object * obj, vm_object * key, vm_object * val, int mode) _REENTRANT_  {
    int i = 1;
	int len, llen, tllen;
	int j;   
	int tlen;
	uint8 tag;
	int index = -1;
	int cntr = 0;
	int dlen = 0;			
	int est_count;
	vm_object * retobj = NULL;
	//uint8 bbuf[VA_OBJECT_MAX_SIZE];	
	uint8 * bbuf;
	if(obj->len == 0) goto exit_arg_set;
	switch (obj->type) {
		case VM_OBJ_TYPE_INTERFACE:
			if (val->len == 0) {
				switch (mode) {
					case VA_ARG_MODE_DEQUEUE:
						obj = vm_interface_exec_sta(VM_ARG, 0, obj, _RECAST(uint8 *, "dequeue"));
						break;
					case VA_ARG_MODE_POP:
						obj = vm_interface_exec_sta(VM_ARG, 0, obj, _RECAST(uint8 *, "pop"));
						break;
					default:
						vm_push_stack_arc(VM_ARG, key);
						obj = vm_interface_exec_sta(VM_ARG, 1, obj, _RECAST(uint8 *, "remove"));
						break;
				}
			} else {
				vm_push_stack_arc(VM_ARG, val);
				vm_push_stack_arc(VM_ARG, key);
				obj = vm_interface_exec_sta(VM_ARG, 2, obj, _RECAST(uint8 *, "set"));
			}
			if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
			vm_set_retval(obj);
			break;
		default:
			if (obj->bytes[0] != ASN_TAG_SET && obj->bytes[0] != ASN_TAG_SEQ) goto exit_arg_set;		//invalid array/object mark	 
			if (vm_is_numeric(key)) index = va_o2f(VM_ARG, key);
			est_count = va_arg_count_s(obj);
			switch (mode) {
				case VA_ARG_MODE_DEQUEUE:
					if (est_count == 0) return;		//no object to dequeue
					index = 0; 
					break;
				case VA_ARG_MODE_POP:
					if (est_count == 0) return;		//no object to pop
					index = est_count - 1; 
					break;
				default: 
					break;
			}
			//allocate memory for buffering output variable
			bbuf = (uint8 *)malloc(obj->len + key->len + val->len + ((est_count + 1) * 6));
			if (bbuf == NULL) return;
			llen = va_get_length(obj->bytes + 1, &tlen);
			dlen = 0;
			tlen += (llen + 1);			//total length
			for (i = (llen + 1);i<tlen;cntr++) {
				tag = obj->bytes[i];
				tllen = va_get_length(obj->bytes + i + 1, &len);
				if (index == -1) {			//search by key
					if (tag == ASN_TAG_OBJDESC &&
						obj->bytes[i + llen + 1 + key->len] == VA_OBJECT_DELIMITER  &&
						memcmp(&obj->bytes[i + llen + 1], key->bytes, key->len) == 0) {
						if (val->len == 0) goto skip_operation;		//deletion
						bbuf[dlen++] = tag;
						memcpy(bbuf + dlen, key->bytes, key->len);
						bbuf[dlen + key->len] = VA_OBJECT_DELIMITER;
						memcpy(bbuf + dlen + 1 + key->len, val->bytes, val->len);
						llen = va_push_lv(bbuf + dlen, bbuf + dlen, (key->len + val->len + 1));
						dlen += llen;
					} else {
						//default operation, copy current member to buffer
						memcpy(bbuf + dlen, obj->bytes + i, len + tllen + 1);
						dlen += (len + tllen + 1);
					}
				}
				else {						//searchskip_operation by index
					if (cntr == index) {
						if (val->len == 0 ) {
							if (mode != 0) {
								int wwlen;
								int rrlen;
								rrlen = va_get_length(obj->bytes + i + 1, &wwlen);
								switch (obj->bytes[i]) {
								case ASN_TAG_OCTSTRING:			//string object
									retobj = vm_create_object(VM_ARG, wwlen, obj->bytes + i + rrlen + 1);
									break;
								case ASN_TAG_REAL:		//float object
									retobj = va_create_ext_float(VM_ARG, wwlen, obj->bytes + i + rrlen + 1);
									break;
								default:		//either a key-value, sequence(object) or set(array) 
									retobj = vm_create_arg(VM_ARG, len + tllen + 1, obj->bytes + i);
									//retobj->mgc_refcount = VM_OBJ_MAGIC | 1;
									break;
								}
							}
							goto skip_operation;		//deletion
						}
						switch (vm_object_get_type(val)) {
						case VM_OBJ_MAGIC:		//copy raw data
							memcpy(bbuf + dlen, val->bytes, val->len);
							dlen += val->len;
							break;
						case VM_MAGIC:
							bbuf[dlen++] = ASN_TAG_OCTSTRING;
							llen = va_push_lv(bbuf + dlen, val->bytes, val->len);
							dlen += llen;
							break;
						case VM_EXT_MAGIC:				//added 2018.08.30
							bbuf[dlen++] = vm_ext_get_tag(val);
							llen = va_push_lv(bbuf + dlen, bbuf + dlen + 4, vm_object_get_text(VM_ARG, val, bbuf + dlen + 4));
							dlen += llen;
							break;
						}
					} else {
						//default operation, copy current member to buffer
						memcpy(bbuf + dlen, obj->bytes + i, len + tllen + 1);
						dlen += (len + tllen + 1);
					}
				}
			skip_operation:
				i += (len + tllen + 1);
			}
			dlen = va_push_lv(bbuf + 1, bbuf, dlen);
			bbuf[0] = obj->bytes[0];
			if (retobj != 0) {
				vm_set_retval(retobj);		//return last/first object, update mutator for current object
				retobj = vm_create_arg(VM_ARG, dlen + 1, bbuf);
				if (retobj->len != 0) {
					retobj->mgc_refcount = VM_OBJ_MAGIC | (obj->mgc_refcount & 0x0F);		//copy header bytes, set to object in case didn't
					vm_update_mutator(VM_ARG, obj, retobj);						//update mutator
					obj->mgc_refcount &= VM_MAGIC_MASK;									//clear refcount
					vm_release_object(VM_ARG, obj);										//release header
				}
			} 
			else {							//return current object, increase refcount, also update mutator
				vm_set_retval((retobj = vm_create_arg(VM_ARG, dlen + 1, bbuf)));
				if (retobj->len != 0) {
					retobj->mgc_refcount = VM_OBJ_MAGIC | ((obj->mgc_refcount + 1) & 0x0F);		//copy header bytes, set to object in case didn't
					vm_update_mutator(VM_ARG, obj, retobj);						//update mutator
					obj->mgc_refcount &= VM_MAGIC_MASK;									//clear refcount
					vm_release_object(VM_ARG, obj);										//release header
				}
			}
			free(bbuf);
			break;
	}
exit_arg_set:
	return;
}

void va_arg_add(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_add);
	int i = 1;
	int len, llen;
	int index = -1;
	int cntr = 0;
	int dlen = 0;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];	
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	vm_object * val = vm_get_argument(VM_ARG, 1);	
	if(obj->len == 0) goto exit_arg_add;
	switch (obj->type) {
		case VM_OBJ_TYPE_INTERFACE:
			vm_push_stack_arc(VM_ARG, vm_get_argument(VM_ARG, 1));
			obj = vm_interface_exec_sta(VM_ARG, 1, obj, _RECAST(uint8 *, "add"));
			if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
			vm_set_retval(obj);
			break;
		default:
			if (obj->bytes[0] != ASN_TAG_SET && obj->bytes[0] != ASN_TAG_SEQ) goto exit_arg_add;		//invalid array/object mark
			va_pop_lv(obj->bytes + 1, bbuf, &dlen);
			switch (vm_object_get_type(val)) {
			case VM_OBJ_MAGIC:
				memcpy(bbuf + dlen, val->bytes, val->len);
				dlen += val->len;
				break;
			case VM_MAGIC:
				bbuf[dlen] = ASN_TAG_OCTSTRING;
				llen = va_push_lv(bbuf + dlen + 1, val->bytes, val->len);
				dlen += (llen + 1);
				break;
			case VM_EXT_MAGIC:			//added 2018.08.30
				bbuf[dlen] = vm_ext_get_tag(val);
				llen = va_push_lv(bbuf + dlen + 1, bbuf + dlen + 4, vm_object_get_text(VM_ARG, val, bbuf + dlen + 4));
				dlen += (llen + 1);
				break;
			}
			dlen = va_push_lv(bbuf + 1, bbuf, dlen);
			bbuf[0] = obj->bytes[0];
			vm_set_retval(vm_create_arg(VM_ARG, dlen + 1, bbuf));
			if (vm_get_retval()->len != 0) {
				vm_get_retval()->mgc_refcount = VM_OBJ_MAGIC | ((obj->mgc_refcount + 1) & 0x0F);		//copy header bytes, set to object in case didn't
				vm_update_mutator(VM_ARG, obj, vm_get_retval());						//update mutator
				obj->mgc_refcount &= VM_MAGIC_MASK;									//clear refcount
				vm_release_object(VM_ARG, obj);										//release header
			}
			break;
	}
	exit_arg_add:
	//OS_DEBUG_EXIT();
	return;
}

void va_arg_set(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_set);
 	va_arg_set_operation(VM_ARG, vm_get_argument(VM_ARG, 0), vm_get_argument(VM_ARG, 1), vm_get_argument(VM_ARG, 2), VA_ARG_MODE_SET);
	//OS_DEBUG_EXIT();
}
		
void va_arg_remove(VM_DEF_ARG) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_remove);
 	va_arg_set_operation(VM_ARG, vm_get_argument(VM_ARG, 0), vm_get_argument(VM_ARG, 1), VM_NULL_OBJECT, VA_ARG_MODE_SET);
	//OS_DEBUG_EXIT();
}

void va_arg_dequeue(VM_DEF_ARG) _REENTRANT_ {
	va_arg_set_operation(VM_ARG, vm_get_argument(VM_ARG, 0), VM_NULL_OBJECT, VM_NULL_OBJECT, VA_ARG_MODE_DEQUEUE);

}

void va_arg_pop(VM_DEF_ARG) _REENTRANT_ {
	va_arg_set_operation(VM_ARG, vm_get_argument(VM_ARG, 0), VM_NULL_OBJECT, VM_NULL_OBJECT, VA_ARG_MODE_POP);
}

static void va_enumerator_release(void * ctx, void * obj) {
	va_enumerator * enumerator = (va_enumerator *)obj;
	if (enumerator == NULL) return;
	if (enumerator->object == NULL) return;
	vm_object_unref(enumerator->object);
}

static vm_object * va_create_enumerator(VM_DEF_ARG, vm_object * obj) {
	va_enumerator enumerator;
	vm_object * v_enum;
	if (obj == NULL) return NULL;
	if (obj->len == 0) return NULL;
	if (obj->bytes[0] != ASN_TAG_SET && obj->bytes[0] != ASN_TAG_SEQ) return NULL;		//invalid array/object mark	  

	//if (enumerator != NULL) {														//obj = obj->bytes;
	enumerator.object = obj;
	enumerator.current = 0;
	enumerator.end = 0;
	enumerator.ref_count = (obj->mgc_refcount & 0x0F);
	//obj->mgc_refcount++;
	vm_object_ref(obj);
	//}
	v_enum = vm_create_object(VM_ARG, sizeof(va_enumerator), (uint8 *)&enumerator);
	v_enum->release = va_enumerator_release;
	return v_enum;
}

static vm_object * va_next_enumerator(VM_DEF_ARG, vm_object * obj) {
	uint16 i, j;
	uint16 tlen;
	int len, llen;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint8 tag;
	vm_object * ret = NULL;
	va_enumerator * enumerator;
	if (obj == NULL) return NULL;
	if (obj->len == 0) return NULL;
	enumerator = (va_enumerator *)obj->bytes;
	if (enumerator->current == NULL) return NULL;
	if (enumerator->end == NULL) return NULL;
	if (enumerator->current >= enumerator->end) return NULL;
	if (enumerator->object == NULL)return NULL;
	if ((enumerator->object->mgc_refcount & 0x0F) != enumerator->ref_count) return NULL;
	if (enumerator != NULL) {
		//first object
		tag = enumerator->current[0];
		llen = va_pop_lv(enumerator->current + 1, bbuf, &len);
		switch (tag) {
			case ASN_TAG_OCTSTRING:
				ret = vm_create_object(VM_ARG, len, bbuf);
				break;
			case ASN_TAG_REAL:		//added 2018.08.30
				ret = va_create_ext_float(VM_ARG, len, bbuf);
				break;
			default:		//either a key-value, sequence(object) or set(array) 
				llen = va_push_lv(bbuf + 1, bbuf, len);
				bbuf[0] = tag;
				ret = vm_create_arg(VM_ARG, llen + 1, bbuf);
				//if (ret != VM_NULL_OBJECT) ret->mgc_refcount |= VM_OBJ_MAGIC;
				break;
		}
		enumerator->current += 1 + llen;
	}
	return ret;
}

void va_arg_first(VM_DEF_ARG) _REENTRANT_ {
	uint16 i, j;
	int tlen;
	int len, llen;
	//uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint8 * bbuf;
	uint8 tag;
	vm_object * ibj;
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	va_enumerator * enumerator;
	if (obj == NULL) return;
	if (obj->len == 0) return;
	switch (obj->type) {
		case VM_OBJ_TYPE_INTERFACE:		//native enumerator
			obj = vm_interface_exec_sta(VM_ARG, 0, obj, _RECAST(uint8 *, "first"));
			if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
			vm_set_retval(obj);
			break;
		default:					//array-object enumerator
			ibj = va_create_enumerator(VM_ARG, obj);
			if (ibj == NULL) return;
			if (ibj->len == 0) return;
			enumerator = (va_enumerator *)ibj->bytes;
			//initialize enumerator
			obj = enumerator->object;
			bbuf = (uint8 *)malloc(obj->len);
			llen = va_pop_lv(obj->bytes + 1, bbuf, &tlen);
			enumerator->current = obj->bytes + (1 + va_get_length(obj->bytes + 1, &len));
			enumerator->end = obj->bytes + 1 + llen;
			free(bbuf);
			//if (len == 0) return;
			//run next
			//vm_set_retval(va_next_enumerator(VM_ARG, ibj));
			vm_set_retval(ibj);
			break;
	}
}

void va_arg_try_next(VM_DEF_ARG) _REENTRANT_ {
	vm_object * obj;
	obj = vm_get_argument(VM_ARG, 0);
	switch (obj->type) {
		case VM_OBJ_TYPE_INTERFACE:		//native enumerator
			obj = vm_interface_exec_sta(VM_ARG, 0, obj, _RECAST(uint8 *, "next"));
			if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
			vm_set_retval(obj);
			break;
		default:					//array-object enumerator
			obj = va_next_enumerator(VM_ARG, obj);
			if (obj == NULL) obj = vm_create_object(VM_ARG, 0, NULL);
			vm_set_retval(obj);
			break;
	}
}

void va_string_split(VM_DEF_ARG) _REENTRANT_ {
	uint8 * bbuf;
	uint8 lbuf[6];
	int llen;
	int i,k;
	int j = 0;
	uint8 c;
	int last_index = 0;
	vm_object * val = vm_get_argument(VM_ARG, 0);
	vm_object * pattern = vm_get_argument(VM_ARG, 1);
	bbuf = (uint8 *)malloc(VA_OBJECT_MAX_SIZE + val->len);
	for(i=0;i<val->len;) {
		c = val->bytes[i];
		if(c == pattern->bytes[0]) {
			for(k=0;k<pattern->len;k++) 
				if(val->bytes[i+k] != pattern->bytes[k]) goto push_buffer;
			llen = va_push_lv(bbuf + last_index + 1, bbuf + last_index, j);
			bbuf[last_index]=ASN_TAG_OCTSTRING;		//tag
			last_index += llen + 1;
			j = 0;
			i += pattern->len;
		} else {
			push_buffer:
			bbuf[last_index + j++] = c;
			i++;
		}
	}
	llen = va_push_lv(bbuf + 1, bbuf , last_index);
	bbuf[0] = ASN_TAG_SET;
	vm_set_retval( vm_create_object(VM_ARG, llen + 1, bbuf)); 
	free(bbuf);
}

void va_bytes(VM_DEF_ARG) _REENTRANT_ {	   //SELECT ITEM  
	//OS_DEBUG_ENTRY(va_bytes);
	uint8 i, j, k;
	uint8 len;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	vm_object * param;
	j = vm_get_argument_count(VM_ARG);
	//if(j == 0) return;
	for(i=0,k=0;i<j;i++) { 
		param = vm_get_argument(VM_ARG, i);
		bbuf[k++] = va_o2f(VM_ARG, param);	
	}
	vm_set_retval(vm_create_object(VM_ARG, k, bbuf)); 
	//OS_DEBUG_EXIT();
}


va_buffer * va_create_buffer(uint16 tag, uint32 size, uint8 * content) {
	va_buffer *buffer = (va_buffer *)malloc(sizeof(va_buffer) + size);
	buffer->tag = tag;
	buffer->length = size;
	buffer->next = NULL;
	memcpy(buffer->buffer, content, size);
	return buffer;
}

va_buffer * va_add_buffer(va_buffer * head, uint16 tag, uint32 size, uint8 * content) {
	va_buffer * iterator=head;
	if(iterator == NULL) return NULL;
	while(iterator->next != NULL) {
		iterator= iterator->next;
	}
	iterator->next = va_create_buffer(tag, size, content);
	return head;
}

va_buffer * va_create_array_from_buffer(va_buffer * head) {
	uint16 j;
	vm_object * obj, * ibj;
	uint32 len = 0, llen;
	va_buffer * ret = NULL;
    //double sum = 0;	
	va_buffer * iterator = head;
	uint8 * bbuf = NULL;
	uint16 count = 0;

    //va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
    while(iterator != NULL) {
        len += iterator->length + 5;
		iterator = iterator->next;
		count ++;
	}
	bbuf = (uint8 *)malloc(sizeof(va_buffer) + len);
	len = 0;
	iterator = head;
	for (j = 0; j < count; j++) {
		llen = va_push_lv(bbuf + len + 1, iterator->buffer, iterator->length);
		bbuf[len] = iterator->tag;
		len += (llen + 1);
		iterator = iterator->next;
	}
	llen = va_push_lv(bbuf + 1, bbuf, len);
	bbuf[0] = ASN_TAG_SET;
	len = (llen + 1);
	ret = va_create_buffer(ASN_TAG_OCTSTRING, len, bbuf);
	free(bbuf);
	return ret;
}

void va_release_buffer(va_buffer * head) {
	va_buffer * iterator = head;
	va_buffer * candidate;
	while(iterator != NULL) {
		candidate = iterator;
		iterator= iterator->next;
		free(candidate);
	}
}

#ifdef __cplusplus_cli
void va_dlopen(VM_DEF_ARG) {
	//dummy function
}
#else
void va_dlopen(VM_DEF_ARG) {
	//load specific library into memory, returning instance to the library
	uint8 buffer[256];
	char fullpath[512];
	void * ret;
	vm_object * obj;
	//pk_class * pkc = NULL;
	vm_interface iface;
	vm_object * param = vm_get_argument(VM_ARG, 0);
	if(param->len == 0) return;
	memset(buffer, 0, sizeof(buffer));
	strncpy((char *)buffer, (const char *)param->bytes, param->len);
	iface.ptr = NULL;

#ifdef STACK_HOST
	extern pk_class * stack_port_load_class(VM_DEF_ARG, char * name);
	iface.base = stack_port_load_class(VM_ARG, (char *)buffer);
#else
	iface.base = lk_select_class(vm_get_info(), buffer);
#endif
	//only works if package found and has interface tag
	if(iface.base != NULL && ((pk_object *)iface.base)->tag == PK_TAG_INTERFACE) {
		//try load library
#ifdef STACK_HOST

		sprintf(fullpath, "%s/%s", mod_directory, ((pk_interface *)iface.base)->libname);
#else
		sprintf(fullpath, "%s", ((pk_interface *)iface.base)->libname);
#endif
		//if(((pk_object *)iface.base)->codebase == NULL) {
		//ret = dlopen((const char *)fullpath, RTLD_GLOBAL);
#if defined(WIN32) || defined(WIN64)
		ret = dlopen((const char *)fullpath, RTLD_GLOBAL);
#else
		ret = dlopen((const char *)fullpath, RTLD_LOCAL);
#endif
		if(ret != NULL) {
			//set codebase to ret indicating base memory for loaded library
			((pk_object *)iface.base)->codebase = ret;
		} else {
			//library not loaded, set to null
			//(((pk_object **)obj->bytes)[0])->codebase = 0;
		}
		//}
		obj = vm_create_object(VM_ARG, sizeof(vm_interface), (uchar *)&iface);
		vm_set_retval(obj);
	}
}
#endif

void va_dlexec(VM_DEF_ARG) {
	//execute method after dlcast has been called incase any arguments
}

#define VA_DST_TYPE(x) (x << 8)
#define VA_SRC_TYPE(x) (x)

#ifdef WIN32
#define define_memcpy(t1, t2, src, dst, sz) {\
	##t1 *wsrc = (##t1 *)src;\
	##t2 *wdst = (##t2 *)dst;\
	int32 i;\
	for(i=0;i<sz;i++) {\
		wdst[i] = (##t2)wsrc[i];\
	}\
}
#else
#define define_memcpy(t1, t2, src, dst, sz) {\
	t1 *wsrc = (t1 *)src;\
	t2 *wdst = (t2 *)dst;\
	int32 i;\
	for(i=0;i<sz;i++) {\
		wdst[i] = (t2)wsrc[i];\
	}\
}
#endif

void memcpy_b_b(void * dst, void * src, int32 sz) define_memcpy(int8, int8, src, dst, sz)
void memcpy_b_s(void * dst, void * src, int32 sz) define_memcpy(int8, int16, src, dst, sz)
void memcpy_b_w(void * dst, void * src, int32 sz) define_memcpy(int8, int32, src, dst, sz)
void memcpy_b_l(void * dst, void * src, int32 sz) define_memcpy(int8, int64, src, dst, sz)
void memcpy_b_f(void * dst, void * src, int32 sz) define_memcpy(int8, float, src, dst, sz)
void memcpy_b_d(void * dst, void * src, int32 sz) define_memcpy(int8, double, src, dst, sz)
void memcpy_b_p(void * dst, void * src, int32 sz) define_memcpy(int8, void *, src, dst, sz)

void memcpy_s_b(void * dst, void * src, int32 sz) define_memcpy(int16, int8, src, dst, sz)
void memcpy_s_s(void * dst, void * src, int32 sz) define_memcpy(int16, int16, src, dst, sz)
void memcpy_s_w(void * dst, void * src, int32 sz) define_memcpy(int16, int32, src, dst, sz)
void memcpy_s_l(void * dst, void * src, int32 sz) define_memcpy(int16, int64, src, dst, sz)
void memcpy_s_f(void * dst, void * src, int32 sz) define_memcpy(int16, float, src, dst, sz)
void memcpy_s_d(void * dst, void * src, int32 sz) define_memcpy(int16, double, src, dst, sz)
void memcpy_s_p(void * dst, void * src, int32 sz) define_memcpy(int16, void *, src, dst, sz)

void memcpy_w_b(void * dst, void * src, int32 sz) define_memcpy(int32, int8, src, dst, sz)
void memcpy_w_s(void * dst, void * src, int32 sz) define_memcpy(int32, int16, src, dst, sz)
void memcpy_w_w(void * dst, void * src, int32 sz) define_memcpy(int32, int32, src, dst, sz)
void memcpy_w_l(void * dst, void * src, int32 sz) define_memcpy(int32, int64, src, dst, sz)
void memcpy_w_f(void * dst, void * src, int32 sz) define_memcpy(int32, float, src, dst, sz)
void memcpy_w_d(void * dst, void * src, int32 sz) define_memcpy(int32, double, src, dst, sz)
void memcpy_w_p(void * dst, void * src, int32 sz) define_memcpy(int32, void *, src, dst, sz)

void memcpy_l_b(void * dst, void * src, int32 sz) define_memcpy(int64, int8, src, dst, sz)
void memcpy_l_s(void * dst, void * src, int32 sz) define_memcpy(int64, int16, src, dst, sz)
void memcpy_l_w(void * dst, void * src, int32 sz) define_memcpy(int64, int32, src, dst, sz)
void memcpy_l_l(void * dst, void * src, int32 sz) define_memcpy(int64, int64, src, dst, sz)
void memcpy_l_f(void * dst, void * src, int32 sz) define_memcpy(int64, float, src, dst, sz)
void memcpy_l_d(void * dst, void * src, int32 sz) define_memcpy(int64, double, src, dst, sz)
void memcpy_l_p(void * dst, void * src, int32 sz) define_memcpy(int64, void *, src, dst, sz)

void memcpy_f_b(void * dst, void * src, int32 sz) define_memcpy(float, int8, src, dst, sz)
void memcpy_f_s(void * dst, void * src, int32 sz) define_memcpy(float, int16, src, dst, sz)
void memcpy_f_w(void * dst, void * src, int32 sz) define_memcpy(float, int32, src, dst, sz)
void memcpy_f_l(void * dst, void * src, int32 sz) define_memcpy(float, int64, src, dst, sz)
void memcpy_f_f(void * dst, void * src, int32 sz) define_memcpy(float, float, src, dst, sz)
void memcpy_f_d(void * dst, void * src, int32 sz) define_memcpy(float, double, src, dst, sz)
//void memcpy_f_p(void * dst void * src, int32 sz) define_memcpy(float, void *, src, dst, sz)

void memcpy_d_b(void * dst, void * src, int32 sz) define_memcpy(double, int8, src, dst, sz)
void memcpy_d_s(void * dst, void * src, int32 sz) define_memcpy(double, int16, src, dst, sz)
void memcpy_d_w(void * dst, void * src, int32 sz) define_memcpy(double, int32, src, dst, sz)
void memcpy_d_l(void * dst, void * src, int32 sz) define_memcpy(double, int64, src, dst, sz)
void memcpy_d_f(void * dst, void * src, int32 sz) define_memcpy(double, float, src, dst, sz)
void memcpy_d_d(void * dst, void * src, int32 sz) define_memcpy(double, double, src, dst, sz)
//void memcpy_d_p(void * dst, void * src, int32 sz) define_memcpy(double, void *, src, dst, sz)

void memcpy_p_b(void * dst, void * src, int32 sz) define_memcpy(void *, int8, src, dst, sz)
void memcpy_p_s(void * dst, void * src, int32 sz) define_memcpy(void *, int16, src, dst, sz)
void memcpy_p_w(void * dst, void * src, int32 sz) define_memcpy(void *, int32, src, dst, sz)
void memcpy_p_l(void * dst, void * src, int32 sz) define_memcpy(void *, int64, src, dst, sz)
//void memcpy_p_f(void * dst, void * src, int32 sz) define_memcpy(void *, float, src, dst, sz)
//void memcpy_p_d(void * dst, void * src, int32 sz) define_memcpy(void *, double, src, dst, sz)
void memcpy_p_p(void * dst, void * src, int32 sz) define_memcpy(void *, void *, src, dst, sz)

typedef struct memcpy_function {
	void (* func)(void *, void *, int32);
} memcpy_function; 

static memcpy_function g_memcpy_func[64][64];

void va_data_init() {
	memset(g_memcpy_func, 0, sizeof(g_memcpy_func));

	g_memcpy_func[sizeof(int8)][sizeof(int8)].func = memcpy_b_b;
	g_memcpy_func[sizeof(int8)][sizeof(int16)].func = memcpy_b_s;
	g_memcpy_func[sizeof(int8)][sizeof(int32)].func = memcpy_b_w;
	g_memcpy_func[sizeof(int8)][sizeof(int64)].func = memcpy_b_l;
	g_memcpy_func[sizeof(int8)][VM_FLOAT].func = memcpy_b_f;
	g_memcpy_func[sizeof(int8)][VM_DOUBLE].func = memcpy_b_d;
	//g_memcpy_func[sizeof(int8)][sizeof(void *)].func = memcpy_b_p;

	g_memcpy_func[sizeof(int16)][sizeof(int8)].func = memcpy_s_b;
	g_memcpy_func[sizeof(int16)][sizeof(int16)].func = memcpy_s_s;
	g_memcpy_func[sizeof(int16)][sizeof(int32)].func = memcpy_s_w;
	g_memcpy_func[sizeof(int16)][sizeof(int64)].func = memcpy_s_l;
	g_memcpy_func[sizeof(int16)][VM_FLOAT].func = memcpy_s_f;
	g_memcpy_func[sizeof(int16)][VM_DOUBLE].func = memcpy_s_d;
	//g_memcpy_func[sizeof(int16)][sizeof(void *)].func = memcpy_s_p;

	g_memcpy_func[sizeof(int32)][sizeof(int8)].func = memcpy_w_b;
	g_memcpy_func[sizeof(int32)][sizeof(int16)].func = memcpy_w_s;
	g_memcpy_func[sizeof(int32)][sizeof(int32)].func = memcpy_w_w;
	g_memcpy_func[sizeof(int32)][sizeof(int64)].func = memcpy_w_l;
	g_memcpy_func[sizeof(int32)][VM_FLOAT].func = memcpy_w_f;
	g_memcpy_func[sizeof(int32)][VM_DOUBLE].func = memcpy_w_d;
	//g_memcpy_func[sizeof(int32)][sizeof(void *)].func = memcpy_w_p;

	g_memcpy_func[sizeof(int64)][sizeof(int8)].func = memcpy_l_b;
	g_memcpy_func[sizeof(int64)][sizeof(int16)].func = memcpy_l_s;
	g_memcpy_func[sizeof(int64)][sizeof(int32)].func = memcpy_l_w;
	g_memcpy_func[sizeof(int64)][sizeof(int64)].func = memcpy_l_l;
	g_memcpy_func[sizeof(int64)][VM_FLOAT].func = memcpy_l_f;
	g_memcpy_func[sizeof(int64)][VM_DOUBLE].func = memcpy_l_d;
	//g_memcpy_func[sizeof(int64)][sizeof(void *)].func = memcpy_l_p;

	g_memcpy_func[VM_FLOAT][sizeof(int8)].func = memcpy_f_b;
	g_memcpy_func[VM_FLOAT][sizeof(int16)].func = memcpy_f_s;
	g_memcpy_func[VM_FLOAT][sizeof(int32)].func = memcpy_f_w;
	g_memcpy_func[VM_FLOAT][sizeof(int64)].func = memcpy_f_l;
	g_memcpy_func[VM_FLOAT][VM_FLOAT].func = memcpy_f_f;
	g_memcpy_func[VM_FLOAT][VM_DOUBLE].func = memcpy_f_d;
	
	g_memcpy_func[VM_DOUBLE][sizeof(int8)].func = memcpy_d_b;
	g_memcpy_func[VM_DOUBLE][sizeof(int16)].func = memcpy_d_s;
	g_memcpy_func[VM_DOUBLE][sizeof(int32)].func = memcpy_d_w;
	g_memcpy_func[VM_DOUBLE][sizeof(int64)].func = memcpy_d_l;
	g_memcpy_func[VM_DOUBLE][VM_FLOAT].func = memcpy_d_f;
	g_memcpy_func[VM_DOUBLE][VM_DOUBLE].func = memcpy_d_d;

	//g_memcpy_func[sizeof(void *)][sizeof(int8)].func = memcpy_p_b;
	//g_memcpy_func[sizeof(void *)][sizeof(int16)].func = memcpy_p_s;
	//g_memcpy_func[sizeof(void *)][sizeof(int32)].func = memcpy_p_w;
	//g_memcpy_func[sizeof(void *)][sizeof(int64)].func = memcpy_p_l;
	//g_memcpy_func[sizeof(void *)][sizeof(float)].func = memcpy_f_f;
	//g_memcpy_func[sizeof(void *)][sizeof(double)].func = memcpy_f_d;
	//g_memcpy_func[sizeof(void *)][sizeof(void *)].func = memcpy_p_p;
}

static uint16 va_arg_serialize_b(uint8 * buffer, uint8 ptype, uint8 ** strbuf, uint16 objlen) _REENTRANT_ {
	uint8 c;
	int j = 0;
	int wlen, llen;
	int len = 0;
	uint8 state =0;
	uint8 wc;
	int32 temp32;
	double temp_d;
	uint8 num_buffer[32];
	uint8 is_numeric = 0;
	while(j++ < objlen && (c = *(strbuf[0])++)) {
		switch(c) {
			case ASN_TAG_SET:		//array		ASN_TAG_SET
				state=0;
				llen = va_get_length(strbuf[0], &wlen);
				strbuf[0] += llen;
				j += llen;
				if(len == 0) {
					len = va_arg_serialize_b(buffer + len, ptype, strbuf, wlen);
				}
				j += wlen;
				(strbuf[0]) += wlen;
				break;
			case ASN_TAG_OCTSTRING:
				llen = va_get_length(strbuf[0], &wlen);
				strbuf[0] += llen;
				if(va_arg_is_numeric(strbuf[0], wlen)) {
					//numeric doesn't need for quote
					//memcpy(buffer + len, strbuf[0], wlen);
					//(strbuf[0]) += wlen;
					memcpy(num_buffer, strbuf[0], wlen);
					num_buffer[wlen] = 0;
					temp32 = atoi((const char *)num_buffer);
					if(g_memcpy_func[VM_INT32][(ptype & 0x3F)].func != NULL) {
						g_memcpy_func[VM_INT32][(ptype & 0x3F)].func(buffer + len, &temp32, 1);
						len += (ptype & 0x0F);
					}
					j += wlen;
				} else {
					goto argument_mismatch_excp;
				}
				j += llen;
				(strbuf[0]) += wlen;
				break;
			case ASN_TAG_REAL:			//added 2018.08.30
				llen = va_get_length(strbuf[0], &wlen);
				strbuf[0] += llen;
				j += llen;
				memcpy(num_buffer, strbuf[0], wlen);
				num_buffer[wlen] = 0;
				temp_d = atof((const char *)num_buffer);
				if(g_memcpy_func[VM_DOUBLE][(ptype & 0x3F)].func != NULL) {
					g_memcpy_func[VM_DOUBLE][(ptype & 0x3F)].func(buffer + len, &temp_d, 1);
					len += (ptype & 0x0F);
				}
				j += wlen;
				(strbuf[0]) += wlen;
				break;
			argument_mismatch_excp:		//skip argument mismatch
			case ASN_TAG_SEQ:		//object	ASN_TAG_SEQ	
			case VA_OBJECT_DELIMITER:
			case ASN_TAG_OBJDESC:					//ASN_TAG_OBJDESC
			default:
				llen = va_get_length(strbuf[0], &wlen);
				strbuf[0] += llen;
				j += llen;
				j += wlen;
				(strbuf[0]) += wlen;
				break;
		}
	}
	return len;
}

vm_object * va_data_cast(VM_DEF_ARG, vm_object * param, uint16 ptype) {
	vm_object * ret = VM_NULL_OBJECT;
	uint8 buffer[256] = { 0 };
	uint8 * pbuffer;
	uint32 len;
	uint32 temp_len = 0;
	uint32 actual_len = 0;
	int32 temp32;
	float fp32;
	uint8 c;
	uint8 * strbuf;
	double temp_d;
	uint16 index = 0;
	uint8 * temp = NULL;
	if((ptype & 0x3F) == 0) {						
		//from primitive to object (ANY)
		switch(vm_object_get_type(param)) {
			case VM_OBJ_MAGIC:
			case VM_MAGIC:
			case VM_EXT_MAGIC:
				//retain type, clone object
				temp_len = param->len;
				actual_len = temp_len;
				temp = (uint8 *)malloc(temp_len);
				memcpy(temp, param->bytes, param->len);
				break;
			case VM_OPR_MAGIC:		//from an operand (check type conversion)
				//TODO
				if(ptype & VM_OBJ_TYPE_ARR) {
					//type of primitive is array casted to object
					//not supported because of unknown length during
					//conversion from address to object
					vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_TYPE);
					break;
				}
				len = param->len;
				pbuffer = param->bytes;
				switch(param->type & 0x3F) {
					case VM_INT8:
						sprintf((char *)buffer, "%ld", *(int8 *)pbuffer);
						break;
					case VM_INT16:
						sprintf((char *)buffer, "%ld", *(int16 *)pbuffer);
						break;
					case VM_INT32:
						sprintf((char *)buffer, "%ld", *(int32 *)pbuffer);
						break;
#if defined(__aarch64__) || defined(__x64__) || defined(__x86_64__)
					case VM_INT64:
						sprintf((char *)buffer, "%ld", *(int64 *)pbuffer);
						break;
#endif
					case VM_FLOAT:
						fp32 = *(float *)pbuffer;
						sprintf((char *)buffer, "%f", fp32);
						break;
					case VM_DOUBLE:
						sprintf((char *)buffer, "%f", *(double *)pbuffer);
						break;
					//case sizeof(void *):
					//	sprintf((char *)buffer, "%d", *(long *)pbuffer);
					//	break;
					default:	
						//unsupported type
						vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_TYPE);
						break;
				}
				temp_len = strlen((const char *)buffer);
				actual_len = temp_len;
				if(temp_len != 0) {
					temp = (uint8 *)malloc(temp_len);
					memcpy(temp, buffer, temp_len);
				}
				break;
			default:	
				//unsupported type
				vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_ARGUMENT);
				break;
		}
	} else {
		//from any to primitive (operand)
		switch(vm_object_get_type(param)) {
			case VM_MAGIC:
				len = param->len;
				pbuffer = param->bytes; 
				if(!vm_is_numeric(param)) {	
					//printf("not numeric \n");		
					//convert to array of byte (string)
					if((ptype & VM_OBJ_TYPE_ARR) == 0) {
						//type of object is string casted to non array
						printf("var is not numeric %s %d\n", param->bytes, param->len);
						vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
						break;
					}
					temp_len = (len + 1) * (ptype & 0x0F);
					actual_len = (len * (ptype & 0x0F));
					temp = (uint8 *)malloc(temp_len);
					memset(temp, 0, temp_len);
					///printf("memcpy_func to %d\n", ptype & 0x3F);
					if(g_memcpy_func[VM_INT8][(ptype & 0x3F)].func != NULL) {
						g_memcpy_func[VM_INT8][(ptype & 0x3F)].func(temp, pbuffer, len);
					} else {
						//unsupported type
						vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_ARGUMENT);
					}
				} else {
					//printf("is numeric %s\n", param->bytes);
					temp = (uint8 *)malloc(256);
					temp32 = va_o2f(VM_ARG, param);
					temp_len = (ptype & 0x0F);
					actual_len = temp_len;
					if(g_memcpy_func[VM_INT32][(ptype & 0x3F)].func != NULL) {
						g_memcpy_func[VM_INT32][(ptype & 0x3F)].func(temp, &temp32, 1);
					} else {
						//unsupported type
						vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_ARGUMENT);
					}
				}
				break;
			case VM_OPR_MAGIC:		//already an operand (check type conversion)
				len = param->len;
				temp_len = len * (ptype & 0x0F);
				actual_len = temp_len;
				temp = (uint8 *)malloc(temp_len);
				if(g_memcpy_func[(param->type & 0x3f)][(ptype & 0x3F)].func != NULL) {
					g_memcpy_func[(param->type & 0x3f)][(ptype & 0x3F)].func(temp, param->bytes, 1);
				} else {
					//unsupported type
					vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_ARGUMENT);
				}
				break;
			case VM_EXT_MAGIC:
				len = ((vm_extension *)param->bytes)->apis->text(VM_ARG, param, buffer);
				pbuffer = buffer;		//converted to text first
				if(!vm_is_numeric_s(pbuffer, len)) {		
					//convert to array of byte (string)
					if((ptype & VM_OBJ_TYPE_ARR) == 0) {
						//type of object is string casted to non array
						vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
						break;
					}
					temp_len = (len + 1) * (ptype & 0x0F);
					actual_len = len * (ptype & 0x0F);
					temp = (uint8 *)malloc(temp_len);
					memset(temp, 0, temp_len);
					if(g_memcpy_func[VM_INT8][(ptype & 0x3F)].func != NULL) {
						g_memcpy_func[VM_INT8][(ptype & 0x3F)].func(temp, pbuffer, 1);
					} else {
						//unsupported type
						vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_ARGUMENT);
					}
				} else {
					temp = (uint8 *)malloc(256);
					temp_d = atof((const char *)buffer);
					temp_len = (ptype & 0x0F);
					actual_len = temp_len;
					if(g_memcpy_func[VM_DOUBLE][(ptype & 0x3F)].func != NULL) {
						g_memcpy_func[VM_DOUBLE][(ptype & 0x3F)].func(temp, &temp_d, 1);
					} else {
						//unsupported type
						vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_ARGUMENT);
					}
				}
				break;
			case VM_OBJ_MAGIC:
				//ptype |= VM_ARRAY;		//array type, should be converted to array
				if((ptype & VM_OBJ_TYPE_ARR) == 0) {
					//type of object is array/object casted to non array
					vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
					break;
				}
				strbuf = param->bytes;
				if(ptype == VM_STRING) {
					temp = (uint8 *)malloc(VA_OBJECT_MAX_SIZE);
					memset(temp, 0, VA_OBJECT_MAX_SIZE);
					temp_len = va_arg_serialize_s(temp, &strbuf, param->len) + 1;
					actual_len = temp_len - 1;
				} else {
					temp = (uint8 *)malloc(param->len * (ptype & 0x0F));
					temp_len = va_arg_serialize_b(temp, ptype, &strbuf, param->len);
					actual_len = temp_len;
				}
				break;
			default:	
				//unsupported type
				vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_ARGUMENT);
				break;
		}
	}
	if(temp != NULL) {
		if((ptype & 0x3F) == 0) {		//cast to object (from primitive)
			if((param->mgc_refcount & VM_MAGIC_MASK) == VM_OPR_MAGIC) {
				//casted from operand
				if(param->type & VM_OBJ_TYPE_PRECISION) {
					//create precision type extension
					ret = va_create_ext_float(VM_ARG, temp_len, (uint8 *)temp);
				} else {
					ret = vm_create_object(VM_ARG, temp_len, (uint8 *)temp);
				}
			} else {
				//casted from other than operand (duplicate object)
				ret = vm_create_object(VM_ARG, temp_len, (uint8 *)temp);
				ret->type = ptype;
				ret->mgc_refcount = (param->mgc_refcount & VM_MAGIC_MASK) | 1;
			}
		} else {					//cast to operand (from object)
			ret = vm_create_operand(VM_ARG, ptype, temp, temp_len);
		}
		if (ret != NULL) ret->len = actual_len;
		free(temp);
	}
	return ret;
}

void va_dlcast(VM_DEF_ARG) {
	//flag variable to it's native type, and push the result to current stack
	vm_object * param = vm_get_argument(VM_ARG, 0);
	uint16 ptype = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	vm_set_retval(va_data_cast(VM_ARG, param, ptype));
}

static int va_arg_dump_s(uint8 * buffer, uint8 ** strbuf, int objlen, int indent) {
	uint8 c;
	int j = 0;
	int wlen, llen;
	int len = 0;
	uint8 state = 0;
	uint8 wc;
	uint8 is_numeric = 0;
	while (j++ < objlen && (c = *(strbuf[0])++)) {
		switch (c) {
		case ASN_TAG_SET:		//array		ASN_TAG_SET
			if (len != 0 && state == 0) buffer[len++] = ',';
			state = 0;
			buffer[len++] = '\r';
			buffer[len++] = '\n';
			for (int i = 0;i < (indent<<2);i++) buffer[len++] = ' ';
			buffer[len++] = '[';
			//c = *(strbuf[0])++;
			//j++;
			llen = va_get_length(strbuf[0], &wlen);
			strbuf[0] += llen;
			j += llen;
			len += va_arg_dump_s(buffer + len, strbuf, wlen, indent + 1);
			j += wlen;
			buffer[len++] = '\r';
			buffer[len++] = '\n';
			for (int i = 0;i < (indent << 2);i++) buffer[len++] = ' ';
			buffer[len++] = ']';
			break;
		case ASN_TAG_SEQ:		//object	ASN_TAG_SEQ	
			if (len != 0 && state == 0) buffer[len++] = ',';
			state = 0;
			buffer[len++] = '\r';
			buffer[len++] = '\n';
			for (int i = 0;i < (indent << 2);i++) buffer[len++] = ' ';
			buffer[len++] = '{';
			//c = *(strbuf[0])++;
			//j++;
			llen = va_get_length(strbuf[0], &wlen);
			strbuf[0] += llen;
			j += llen;
			len += va_arg_dump_s(buffer + len, strbuf, wlen, indent + 1);
			j += wlen;
			//j += c;
			buffer[len++] = '\r';
			buffer[len++] = '\n';
			for (int i = 0;i < (indent << 2);i++) buffer[len++] = ' ';
			buffer[len++] = '}';
			break;
		case VA_OBJECT_DELIMITER:
			buffer[len++] = ':';
			if (state == 1) {
				state++;
				//buffer[len++]= '\"';
			}
			break;
		case ASN_TAG_OBJDESC:					//ASN_TAG_OBJDESC
			if (state == 0) {
				if (len != 0) buffer[len++] = ',';
				buffer[len++] = '\r';
				buffer[len++] = '\n';
				for (int i = 0;i < (indent << 2);i++) buffer[len++] = ' ';
				llen = va_get_length(strbuf[0], &wlen);
				strbuf[0] += llen;
				j += llen;
				len += va_arg_dump_s(buffer + len, strbuf, wlen, indent+1);
				j += wlen;
			}
			break;
		case ASN_TAG_OCTSTRING:
			if (state == 0) {
				if (len != 0) buffer[len++] = ',';
				buffer[len++] = '\r';
				buffer[len++] = '\n';
				for (int i = 0;i < (indent << 2);i++) buffer[len++] = ' ';
				llen = va_get_length(strbuf[0], &wlen);
				strbuf[0] += llen;
				j += llen;
				if (va_arg_is_numeric(strbuf[0], wlen)) {
					//numeric doesn't need for quote
					memcpy(buffer + len, strbuf[0], wlen);
					(strbuf[0]) += wlen;
					len += wlen;
					j += wlen;
				}
				else {
					buffer[len++] = '\"';
					memcpy(buffer + len, strbuf[0], wlen);
					(strbuf[0]) += wlen;
					len += wlen;
					j += wlen;
					buffer[len++] = '\"';
				}
			}
			break;
		case ASN_TAG_REAL:			//added 2018.08.30
			if (state == 0) {
				if (len != 0) buffer[len++] = ',';
				buffer[len++] = '\r';
				buffer[len++] = '\n';
				for (int i = 0;i < (indent << 2);i++) buffer[len++] = ' ';
				llen = va_get_length(strbuf[0], &wlen);
				strbuf[0] += llen;
				j += llen;
				memcpy(buffer + len, strbuf[0], wlen);
				(strbuf[0]) += wlen;
				len += wlen;
				j += wlen;
			}
			break;
		case ' ':
			if (state == 0) break;
			if (state == 4) break;
		default:
			if (state == 2 || state == 0) {
				state++;
				if (state == 2 && c >= '0' && c <= '9') {
					state = 4;		//numeric state
				}
				else {
					//check for delimiter ahead
					if (!va_contain_delimiter(strbuf[0] - 1, objlen - (j - 1))) {
						is_numeric = 0;
						if (!va_arg_is_numeric(strbuf[0] - 1, objlen - (j - 1)))
							buffer[len++] = '\"';
						else
							is_numeric = 1;
					}
				}
			}
			switch (c) {
			case '\"': wc = '\"'; goto push_spchar;
			case '\\': wc = '\\'; goto push_spchar;
			case '/': wc = '/'; goto push_spchar;
			case '\b': wc = 'b'; goto push_spchar;
			case '\f': wc = 'f'; goto push_spchar;
			case '\n': wc = 'n'; goto push_spchar;
			case '\r': wc = 'r'; goto push_spchar;
			case '\t': wc = 't'; goto push_spchar;
			push_spchar:
				buffer[len++] = '\\';
				buffer[len++] = wc;
				break;
			default:
				buffer[len++] = c;
				break;
			}
			break;
		}
	}
	if ((state & 0x01) != 0 && is_numeric == 0) buffer[len++] = '\"';
	return len;
}

void va_dump_var(VM_DEF_ARG) {
	uint8 i;
	vm_object * param;
	//uint8 buffer[4096];
	uint8 * buffer;
	uint32 len;
	vm_object * temp;
	uint8 * bptr;
	uint8 arg_count = vm_get_argument_count(VM_ARG);
	//return;
	for (i = 0;i<arg_count;i++) {
		param = vm_get_argument(VM_ARG, i);
		if (param->type != VM_OBJ_TYPE_ANY) {
			param = va_data_cast(VM_ARG, param, VM_OBJ_TYPE_ANY);
		}
		if (vm_object_get_type(param) == VM_EXT_MAGIC) {
			buffer = (uint8 *)malloc(4096);
			len = ((vm_extension *)param->bytes)->apis->text(VM_ARG, param, buffer);
		}
		else if (vm_object_get_type(param) == VM_OBJ_MAGIC) {
			buffer = (uint8 *)malloc(param->len + 4096);
			bptr = param->bytes;
			len = va_arg_dump_s(buffer, &bptr, param->len, 0);
			buffer[len] = 0;
		} else {
			buffer = (uint8 *)malloc(param->len + 1);
			memcpy(_RECAST(char *, buffer), _RECAST(const char *, param->bytes), param->len);
			buffer[param->len] = 0;
			len = param->len;
		}
		if (param != vm_get_argument(VM_ARG, i)) vm_release_object(VM_ARG, param);
#ifdef STACK_HOST
		//VM_ARG->stream << buffer;
		if (vm_get_stream() != NULL) vm_stream_write(vm_get_stream(), buffer, len);
		//printf("%s\n", buffer);
		free(buffer);
#else
		printf("%s<br>", buffer);
#endif
	}
}

void * va_arg_operation_s(VM_DEF_ARG, uint16 opcode, uint8 * m1, uint8 * m2, int * plen) _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_at);
	int i, j;
	int tlen1, tlen2;
	int len1, llen, hlen1, hlen2;
	int len2;
	int llen2;
	int est_count;
	uint8 * bbuf;
	uint8 tag1, tag2;
	vm_object * ibj;
	int bbuf_len = 0;
	double temp_d1, temp_d2;
	int temp_w1, temp_w2;
	uint8 * temp1;
	uint8 * temp2;
	void * ptr;
	int rlen;
	int sublen;
	//len = plen[0];
	llen = va_get_length(m1, &len1);
	llen = va_get_length(m2, &len2);
	bbuf = (uint8 *)malloc((len1 + len2) * 2);
	if (m1[0] == ASN_TAG_SET) {
		//obj = obj->bytes;
		switch (opcode) {
		case INS_ADD:	//array[n] + array[n] = array[n], array[m] + scalar = not allowed
		case INS_SUB:	//array[n] - array[n] = array[n], array[m] + scalar = not allowed
		case INS_MUL:	//array[m] * array[n] = array[m][n], array[m] * scalar = array[m]
		case INS_DIV:	//array[m] / array[n] = array[m][n], array[m] / scalar = array[m]
		case INS_MOD:	//array[m] % array[n] = array[m][n], array[m] % scalar = array[m]
			plen[0] = 0;
			sublen = 0;
			if (m2[0] != ASN_TAG_SET) break;		//not a matrix
			hlen1 = va_get_length(m1 + 1, &tlen1);
			hlen1 = hlen1 + 1;
			hlen2 = va_get_length(m2 + 1, &tlen2);
			hlen2 = hlen2 + 1;
			//should compare number of elements on both array should matched
			if (va_arg_count_t(m1, hlen1 + tlen1) != va_arg_count_t(m2, hlen2 + tlen2)) break;
			for (i = 0, j=0;i < tlen1 && j<tlen2;) {
				//tag = m1[hlen1 + i];
				//llen = va_pop_lv(m1 + hlen1 + i + 1, bbuf + bbuf_len, &len1);
				llen = va_get_length(m1 + hlen1 + i + 1, &len1);
				llen2 = va_get_length(m2 + hlen2 + j + 1, &len2);
				ptr = va_arg_operation_s(VM_ARG, opcode, m1 + hlen1 + i, m2 + hlen2 + j, &rlen);
				memcpy(bbuf + bbuf_len + sublen, ptr, rlen);
				sublen += rlen;
				free(ptr);
				j += (llen2 + len2 + 1);
				i += (llen + len1 + 1);
			}
			//plen[0] = sublen;		//returning length
			//encapsulate elements to array container
			//rlen = va_push_lv(bbuf + bbuf_len + 1, bbuf + bbuf_len, sublen);
			//bbuf[bbuf_len] = ASN_TAG_SET;
			//bbuf_len += (rlen + 1);
			bbuf_len = sublen;
			break;
		case INS_CREQ:
		case INS_CRNE:
		case INS_CRLT:
		case INS_CRGT:
		case INS_CRLTEQ:
		case INS_CRGTEQ:
		default:
			plen[0] = 0;
			hlen1 = va_pop_lv(m1 + 1, bbuf, &tlen1);
			hlen1 = (hlen1 - tlen1) + 1;
			for (i = 0;i < tlen1;) {
				tag1 = m1[hlen1 + i];
				llen = va_pop_lv(m1 + hlen1 + i + 1, bbuf + bbuf_len, &len1);
				switch (m2[0]) {
				case ASN_TAG_SET:
					//matrix 2 is array
					hlen2 = va_get_length(m2 + 1, &tlen2);
					hlen2 = hlen2 + 1;
					sublen = 0;
					//should compare number of elements on both array should matched
					if (va_arg_count_t(m1, hlen1 + tlen1) != va_arg_count_t(m2, hlen2 + tlen2)) break;
					//calculate foreach element
					for (j = 0; j < tlen2;) {
						llen2 = va_get_length(m2 + hlen2 + j + 1, &len2);
						ptr = va_arg_operation_s(VM_ARG, opcode, m1 + hlen1 + i, m2 + hlen2 + j, &rlen);
						memcpy(bbuf + bbuf_len + sublen, ptr, rlen);
						sublen += rlen;
						free(ptr);
						j += (llen2 + len2 + 1);
					}
					//encapsulate elements to array container
					rlen = va_push_lv(bbuf + bbuf_len + 1, bbuf + bbuf_len, sublen);
					bbuf[bbuf_len] = ASN_TAG_SET;
					bbuf_len += (rlen + 1);
					break;
				case ASN_TAG_OCTSTRING:
				case ASN_TAG_REAL:
					//matrix 2 is scalar
					ptr = va_arg_operation_s(VM_ARG, opcode, m1 + hlen1 + i, m2, &rlen);
					memcpy(bbuf + bbuf_len, ptr, rlen);
					bbuf_len += rlen;
					free(ptr);
					break;
				}
				i += (llen + 1);
			}
			break;
		}
		rlen = va_push_lv(bbuf + 1, bbuf, bbuf_len);
		bbuf[0] = ASN_TAG_SET;
		plen[0] = rlen + 1;		//returning length
	}
	//matrix 2 is array but matrix 1 is scalar/(not array)
	else if (m2[0] == ASN_TAG_SET) {
		plen[0] = 0;		//returning length
		//only support multiplication
		if (opcode == INS_MUL) {
			//printf("multiplication with array\n");
			hlen1 = va_get_length(m1 + 1, &tlen1);
			hlen1 = hlen1 + 1;
			hlen2 = va_get_length(m2 + 1, &tlen2);
			hlen2 = hlen2 + 1;
			tag1 = m1[0];
			tag2 = m2[0];
			temp1 = (uint8 *)malloc(tlen1 + 1);
			temp2 = (uint8 *)malloc(tlen2 + 1);
			memcpy(temp1, m1 + hlen1, tlen1); temp1[tlen1] = 0;
			memcpy(temp2, m2 + hlen2, tlen2); temp2[tlen2] = 0;
			switch (tag1) {
			case ASN_TAG_OCTSTRING:
				//memcpy(temp, m1 + hlen1, tlen1);
				temp_w1 = atoi((const char *)temp1);
				//temp_w2 = atoi((const char *)temp2);
				temp_w2 = 0;
				switch (opcode) {
				case INS_MUL:
					//temp_w1 = temp_w1 * temp_w2;
					for (j = 0; j < tlen2;) {
						llen2 = va_pop_lv(m2 + hlen2 + j + 1, temp2, &len2);
						temp2[len2] = 0;
						temp_w2 += (temp_w1 * atoi((const char *)temp2));
						j += (llen2 + 1);
					}
					temp_w1 = temp_w2;
					sprintf((char *)bbuf, "%d", temp_w1);
					bbuf_len = strlen((const char *)bbuf);
					rlen = va_push_lv(bbuf + 1, bbuf, bbuf_len);
					bbuf[0] = tag1;
					rlen = rlen + 1;		//set result length
					break;
				default: tag1 = ASN_TAG_NULL; rlen = 0; break;
				}
				break;
			case ASN_TAG_REAL:
				temp_d1 = atof((const char *)temp1);
				temp_d2 = 0;
				//temp_d2 = atof((const char *)temp2);
				switch (opcode) {
				case INS_MUL: 
					//temp_d1 = temp_d1 * temp_d2;
					for (j = 0; j < tlen2;) {
						llen2 = va_pop_lv(m2 + hlen2 + j + 1, temp2, &len2);
						temp2[len2] = 0;
						temp_d2 += (temp_d1 * atof((const char *)temp2));
						j += (llen2 + 1);
					}
					temp_d1 = temp_d2;
					sprintf((char *)bbuf, "%f", temp_d1);
					bbuf_len = strlen((const char *)bbuf);
					rlen = va_push_lv(bbuf + 1, bbuf, bbuf_len);
					bbuf[0] = tag1;
					rlen = rlen + 1;		//set result length
					break;
				default: tag1 = ASN_TAG_NULL; rlen = 0; break;
				}

				break;
			default: tag1 = ASN_TAG_NULL; rlen = 0; break;
			}
			free(temp1);
			free(temp2);
			plen[0] = rlen;		//returning length
		}
	} else {
		//do element-element operation
		hlen1 = va_get_length(m1 + 1, &tlen1);
		hlen1 = hlen1 + 1;
		hlen2 = va_get_length(m2 + 1, &tlen2);
		hlen2 = hlen2 + 1;
		tag1 = m1[0];
		tag2 = m2[0];
		temp1 = (uint8 *)malloc(tlen1 + 1);
		temp2 = (uint8 *)malloc(tlen2 + 1);
		memcpy(temp1, m1 + hlen1, tlen1); temp1[tlen1] = 0;
		memcpy(temp2, m2 + hlen2, tlen2); temp2[tlen2] = 0;
		switch (tag1) {
		case ASN_TAG_OCTSTRING:
			//memcpy(temp, m1 + hlen1, tlen1);
			temp_w1 = atoi((const char *)temp1);
			temp_w2 = atoi((const char *)temp2);
			switch (opcode) {
				case INS_ADD: temp_w1 = temp_w1 + temp_w2;break;
				case INS_SUB: temp_w1 = temp_w1 - temp_w2;break;
				case INS_MUL: temp_w1 = temp_w1 * temp_w2;break;
				case INS_DIV: temp_w1 = temp_w1 / temp_w2;break;
				case INS_MOD: temp_w1 = temp_w1 % temp_w2;break;
				case INS_CREQ: if (temp_w1 != temp_w2) tag1 = ASN_TAG_NULL; break;
				case INS_CRNE: if (temp_w1 == temp_w2) tag1 = ASN_TAG_NULL; break;
				case INS_CRLT: if (temp_w1 >= temp_w2) tag1 = ASN_TAG_NULL; break;
				case INS_CRGT: if (temp_w1 <= temp_w2) tag1 = ASN_TAG_NULL; break;
				case INS_CRLTEQ: if (temp_w1 > temp_w2) tag1 = ASN_TAG_NULL; break;
				case INS_CRGTEQ: if (temp_w1 < temp_w2) tag1 = ASN_TAG_NULL; break;
				default: tag1 = ASN_TAG_NULL; break;
			}
			break;
		case ASN_TAG_REAL:
			temp_d1 = atof((const char *)temp1);
			temp_d2 = atof((const char *)temp2);
			switch (opcode) {
				case INS_ADD: temp_d1 = temp_d1 + temp_d2;break;
				case INS_SUB: temp_d1 = temp_d1 - temp_d2;break;
				case INS_MUL: temp_d1 = temp_d1 * temp_d2;break;
				case INS_DIV: temp_d1 = temp_d1 / temp_d2;break;
				case INS_MOD: temp_w1 = (int)temp_d1 % (int)temp_d2; tag1 = ASN_TAG_OCTSTRING; break;
				case INS_CREQ: if (temp_d1 != temp_d2) tag1 = ASN_TAG_NULL; break;
				case INS_CRNE: if (temp_d1 == temp_d2) tag1 = ASN_TAG_NULL; break;
				case INS_CRLT: if (temp_d1 >= temp_d2) tag1 = ASN_TAG_NULL; break;
				case INS_CRGT: if (temp_d1 <= temp_d2) tag1 = ASN_TAG_NULL; break;
				case INS_CRLTEQ: if (temp_d1 > temp_d2) tag1 = ASN_TAG_NULL; break;
				case INS_CRGTEQ: if (temp_d1 < temp_d2) tag1 = ASN_TAG_NULL; break;
				default: tag1 = ASN_TAG_NULL; break;
			}

			break;
		}
		switch (tag1) {
			case ASN_TAG_OCTSTRING:
				sprintf((char *)bbuf, "%d", temp_w1);
				bbuf_len = strlen((const char *)bbuf);
				rlen = va_push_lv(bbuf + 1, bbuf, bbuf_len);
				bbuf[0] = tag1;
				rlen = rlen + 1;		//set result length
				break;
			case ASN_TAG_REAL:
				sprintf((char *)bbuf, "%f", temp_d1);
				bbuf_len = strlen((const char *)bbuf);
				rlen = va_push_lv(bbuf + 1, bbuf, bbuf_len);
				bbuf[0] = tag1;
				rlen = rlen + 1;		//set result length
				break;
			case ASN_TAG_NULL:
			default:
				rlen = 0;
				break;
		}

		free(temp1);
		free(temp2);
		plen[0] = rlen;		//returning length
	}
	//free(bbuf);
	return bbuf;			//returning buffer pointer
}

vm_object * va_arg_operation(VM_DEF_ARG, uint8 opcode, vm_object * op1, vm_object * op2) {
	int len = 0;
	vm_object * ret;
	void * ptr = NULL;
	double temp_d, d1, d2;
	int64 temp_w, w1, w2;
	char valbuf2[64];
	uint8 tag;
	vm_extension * obj_ext;
	vm_object * (*func)(VM_DEF_ARG, vm_object *, uint8 *) = NULL;
	if (vm_object_get_type(op1) != VM_OBJ_MAGIC) return vm_create_object(VM_ARG, 0, NULL);	//invalid array operation
	if (vm_object_get_type(op2) == VM_OBJ_MAGIC) {
		//both operand are array/object
		ptr = va_arg_operation_s(VM_ARG, opcode, op1->bytes, op2->bytes, &len);
	}
	else if (vm_object_get_type(op2) == VM_OPR_MAGIC) {
		//convert op2 to scalar representation
		tag = ASN_TAG_OCTSTRING;
		switch (op2->type & 0x3F) {
			case VM_INT8: d2 = w2 = *(int8 *)op2->bytes; sprintf(valbuf2, "%d", w2); break;
			case VM_INT16: d2 = w2 = *(int16 *)op2->bytes; sprintf(valbuf2, "%d", w2); break;
			case VM_INT32: d2 = w2 = *(int32 *)op2->bytes; sprintf(valbuf2, "%d", w2); break;
#if defined(__aarch64__) || defined(__x64__) || defined(__x86_64__)
			case VM_INT64: d2 = w2 = *(int64 *)op2->bytes; sprintf(valbuf2, "%ld", w2); break;
#endif
			case VM_FLOAT: w2 = d2 = *(float *)op2->bytes; sprintf(valbuf2, "%f", d2); tag = ASN_TAG_REAL; break;
			case VM_DOUBLE: w2 = d2 = *(double *)op2->bytes; sprintf(valbuf2, "%f", d2); tag = ASN_TAG_REAL; break;
		}
		va_push_lv((uint8 *)valbuf2 + 1, (uint8 *)valbuf2, strlen(valbuf2));
		valbuf2[0] = tag;
		ptr = va_arg_operation_s(VM_ARG, opcode, op1->bytes, (uint8 *)valbuf2, &len);
	}
	else if (vm_object_get_type(op2) == VM_EXT_MAGIC) {
		obj_ext = (vm_extension *)op2->bytes;		//op1 would be priority
		obj_ext->apis->text(VM_ARG, op2, (uint8 *)valbuf2);
		if (vm_is_numeric_s((uint8 *)valbuf2, strlen(valbuf2)) == TRUE) {
			va_push_lv((uint8 *)valbuf2 + 1, (uint8 *)valbuf2, strlen(valbuf2));
			valbuf2[0] = ASN_TAG_OCTSTRING;
			ptr = va_arg_operation_s(VM_ARG, opcode, op1->bytes, (uint8 *)valbuf2, &len);
		}
	}
	else {
		//default object type (string)
		if (vm_is_numeric(op2) == TRUE) {
			va_push_lv((uint8 *)valbuf2 + 1, (uint8 *)op2->bytes, op2->len);
			valbuf2[0] = ASN_TAG_OCTSTRING;
			ptr = va_arg_operation_s(VM_ARG, opcode, op1->bytes, (uint8 *)valbuf2, &len);
		}
	}
	ret = vm_create_arg(VM_ARG, len, ptr);
	free(ptr);
	return ret;
}

#ifdef STACK_HOST
#if defined(WIN32) || defined(WIN64)
#include "util/pthread.h"
#else
#include <pthread.h>
#endif
void va_sleep(VM_DEF_ARG) {
	uint16 ms = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 0));
#if defined(WIN32) || defined(WIN64)
	//_sleep(ms);
	timespec t;
	t.tv_nsec = ms * 1000;
	t.tv_sec = ms / 1000;
	//pthread_delay_np(&t);
	//while (1);
	_sleep(ms);
#else
	sleep(ms);
#endif

}

static void * va_thread_task(void * arg) {
	vm_instance vcit;
	va_thread_arg * thread_arg = (va_thread_arg *)arg;
	vm_instance * ctx;
	vm_function * func;
	vm_object * varg;
	pthread_t tid;
	int s;
	int ret = 0;
	vm_object * args[16];
	int i;
	int arg_count = 0;
	if (thread_arg == NULL) return NULL;
	if (thread_arg->instance == NULL) return NULL;
	ctx = thread_arg->instance;
	func = thread_arg->func;
	vm_init(&vcit, (pk_object *)vm_get_info(), 20 * 1024);
	vcit.stream = vm_get_stream();
	vm_exec_function(&vcit, thread_arg->func);
	for (i = 0;i < thread_arg->num_args && i < func->arg_count && i<14;i++) {
		varg = thread_arg->arguments[i];
		args[arg_count++] = vm_clone_object(&vcit, varg);
		vm_object_unref(varg);
	}
	for (;i < func->arg_count && i<14;i++) args[arg_count++] = vm_create_object(&vcit, 0, NULL);
	for (i = 0;i < func->arg_count;i++) vm_push_argument_object(&vcit, args[i]);
	thread_arg->ctx = &vcit;
	vm_decode(&vcit, 0, 0);
	vm_close(&vcit);
	s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if (s != 0) {
		printf("pthread_setcancelstate");
	}
	while (thread_arg->stop_requested == FALSE) {

	}
	return NULL;
}

static void va_thread_arg_release(VM_DEF_ARG, void * ptr) {
	//printf("free memory : %ld\n", ptr);
	va_thread_arg * thread_arg;
	if (ptr != NULL) {
		thread_arg = ((va_thread_arg *)ptr);
		if (thread_arg->func != NULL) vm_release_callback(VM_ARG, thread_arg->func);
	}
}

void va_create_thread(VM_DEF_ARG) {
	vm_object * obj;
	vm_object * vfunc = vm_get_argument(VM_ARG, 0);
	va_thread_arg * thread_arg = (va_thread_arg *)malloc(sizeof(va_thread_arg));
	vm_function * func;// = (vm_function *)m_alloc(vm_get_heap(), sizeof(vm_function));
	vm_object * varg;
	thread_arg->thread = NULL;
	thread_arg->instance = VM_ARG;
	thread_arg->stop_requested = FALSE;
	thread_arg->ctx = NULL;		//not initialized, will be initialized by the time thread running
	if (vfunc->len != sizeof(vm_function)) return;
	thread_arg->func = (vm_function *)vm_load_callback(VM_ARG, (vm_function *)vfunc->bytes);
	thread_arg->num_args = thread_arg->func->arg_count;
	vm_set_retval(vm_create_object(VM_ARG, sizeof(va_thread_arg *), &thread_arg) );
}

void va_thread_start(VM_DEF_ARG) {
	pthread_t t;
	vm_object * varg = vm_get_argument(VM_ARG, 0);
	if (varg->len != sizeof(va_thread_arg *)) return;
	va_thread_arg * thread_arg = *((va_thread_arg **)varg->bytes);
	if (thread_arg == NULL) return;
	if (thread_arg->thread == NULL) {
		thread_arg->thread = (pthread_t *)malloc(sizeof(pthread_t));
		for (int i = 1;i < (thread_arg->func->arg_count + 1);i++) {
			varg = vm_get_argument(VM_ARG, i);
			vm_object_ref(varg);			//lock object until it is accessed by thread
			thread_arg->arguments[i] = varg;
		}
		pthread_create((pthread_t *)thread_arg->thread, NULL, va_thread_task, thread_arg);
	}
}

void va_thread_stop(VM_DEF_ARG) {
	pthread_t tid;
	void *res;
	int s;
	vm_object * varg = vm_get_argument(VM_ARG, 0);
	if (varg->len != sizeof(va_thread_arg *)) return;
	va_thread_arg * thread_arg = *((va_thread_arg **)varg->bytes);
	//printf("aborting\n");
	if (thread_arg->ctx != NULL) vm_set_state(thread_arg->ctx, VM_STATE_ABORT);
	varg->len = 0;
	memcpy(&tid, thread_arg->thread, sizeof(pthread_t));

	s = pthread_cancel(tid);
	if (s != 0) {
		//handle_error_en(s, "pthread_cancel");
		printf("error pthread_cancel\n");
	}
	/* Join with thread to see what its exit status was */
	thread_arg->stop_requested = TRUE;
	s = pthread_join(tid, &res);
	if (s != 0) {
		//handle_error_en(s, "pthread_join");
		printf("error pthread_join\n");
	}
	if (res == PTHREAD_CANCELED) {
		//printf("thread was canceled\n");
		vm_release_callback(thread_arg->instance, thread_arg->func);
		if(thread_arg->thread != NULL) free(thread_arg->thread);
		free(thread_arg);
	}
	//_sleep(200);
}

#endif		//STACK_HOST
