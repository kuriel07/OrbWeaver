/*!\file 			vm_framework.h
 * \brief     	Orb-Weaver Framework APIs
 * \details   	Orb-Weaver Framework APIs provide all necessary function for Native API integration to stack machine
 * \author    	AGP
 * \version   	1.4
 * \date      	Created by Agus Purwanto on 06/18/2017.
 * \pre       	
 * \bug       	
 * \warning   	
 * \copyright 	OrbLeaf Technology
\verbatim
1.0
 * initial release, all APIs declared on this header	(2016.06.18)
 * added : va_trap_apis for persistent storage trap allocator, ported from icos_orb	(2016.06.21)
 * added : persistent storage variable APIs (2017.06.21)
 * added : SMTP APIs for sendmail (RFC822) based on http://www.codeproject.com/KB/mcpp/CSmtp.aspx (2017.06.21)
 * changed : added interrupt mechanism for pin event (2017.09.11)
 * added: va_ui_create_image to support image rendering from resource (2017.12.09)
 * added: va_sys_exec_31 to support execution of vm_function object generated by F2O (2018.01.10)
\endverbatim
 */

#include "../defs.h"
#include "../config.h"
#include "vm_stack.h"

#ifndef VM_FRAMEWORK__H

#define VA_OBJECT_DELIMITER			0x80

#define VA_STATE_WAIT						0x08
#define VA_STATE_RUN						0x01
#define VA_STATE_IDLE						0x00

typedef struct va_default_context{
	void * ctx;
	uint32 offset;
	void (* close)(VM_DEF_ARG);
	void (* read)(VM_DEF_ARG);
	void (* write)(VM_DEF_ARG);
	void (* seek)(VM_DEF_ARG);
} va_default_context;

typedef struct va_enumerator {
	vm_object * object;
	uint8 * current;
	uint8 * end;
	int ref_count;
} va_enumerator;

#define VA_PINTYPE_IO						0x03
#define VA_PINTYPE_PWM					0x04
typedef struct va_port_context {
	va_default_context base;
	uint8 state;
	uint8 id;
	uint16 pin;
	void * handle;			//to be released
	uint8 dutycycle;
	uint16 freq;
} va_port_context;

typedef struct va_net_context {
	va_default_context base;
	vm_function callback;
	int shutdown;
	void * sockfd;
	void * thread;
} va_net_context;

typedef struct va_com_context {
	va_default_context base;
	//USART_HandleTypeDef handle;
} va_com_context;

typedef struct va_picc_key {
	void * ctx;
	uint8 block;
	uint8 keyid;
	uint8 key[6];
	struct va_picc_key * next;
} va_picc_key;

typedef struct va_picc_context {
	va_default_context base;
	uint8 state;					//open/close state
} va_picc_context;

typedef struct va_port_config {
	uint16 id;
	void * port;
	uint32 pin;
} va_port_config;

typedef struct va_buffer {
	struct va_buffer * next;
	uint16 tag;
	uint32 length;
	uint8 buffer[1];
} va_buffer;

typedef struct va_thread_arg {
	void * thread;
	vm_instance * instance;
	vm_function * func;
	vm_instance * ctx;
	uint8 num_args;
	vm_object * arguments[16];
	int8 stop_requested;
} va_thread_arg;

#ifndef VM_NULL_OBJECT
#define VM_NULL_OBJECT		(vm_object *)"\xe0\x0\x0\x0\x0\x0\x0\x0\x0"
#endif

extern CONST vm_api_entry g_vaRegisteredApis[];
extern CONST vm_api_entry g_vaRegisteredExtApis[];


#ifdef __cplusplus
extern "C" {
#endif
//global
void va_this(VM_DEF_ARG) _REENTRANT_;
//void va_sys_exec() _REENTRANT_ ;			//execute a function object
void va_set_execution_context() _REENTRANT_ ;
//default context read/write operation
void va_read(VM_DEF_ARG) _REENTRANT_ ;
void va_write(VM_DEF_ARG) _REENTRANT_ ;
void va_close(VM_DEF_ARG) _REENTRANT_ ;
void va_seek(VM_DEF_ARG) _REENTRANT_;
//var
void va_set_var(VM_DEF_ARG) _REENTRANT_ ;
void va_get_var(VM_DEF_ARG) _REENTRANT_ ;
void va_delete_var(VM_DEF_ARG) _REENTRANT_ ; 
//string
void va_index_of(VM_DEF_ARG) _REENTRANT_ ;
void va_last_index_of(VM_DEF_ARG) _REENTRANT_ ;
void va_replace(VM_DEF_ARG) _REENTRANT_ ;
void va_substr(VM_DEF_ARG) _REENTRANT_ ;
void va_string_split(VM_DEF_ARG) _REENTRANT_ ;
//bytes creation
void va_bytes(VM_DEF_ARG) _REENTRANT_ ;
//file
void va_fopen(VM_DEF_ARG) _REENTRANT_ ;  
void va_fread(VM_DEF_ARG) _REENTRANT_ ;
void va_fwrite(VM_DEF_ARG) _REENTRANT_ ;
void va_fclose(VM_DEF_ARG) _REENTRANT_ ;
void va_fpopbytag(VM_DEF_ARG) _REENTRANT_ ;  
//json				
void va_arg_findtag(VM_DEF_ARG) _REENTRANT_ ;	
void va_arg_count(VM_DEF_ARG) _REENTRANT_;			
void va_arg_create(VM_DEF_ARG) _REENTRANT_ ;
void va_arg_object(VM_DEF_ARG) _REENTRANT_ ;
void va_arg_array(VM_DEF_ARG) _REENTRANT_ ; 	 
void va_arg_at(VM_DEF_ARG) _REENTRANT_ ;
void va_arg_get(VM_DEF_ARG) _REENTRANT_ ;
void va_arg_serialize(VM_DEF_ARG) _REENTRANT_ ;			// -> to json string
void va_arg_deserialize(VM_DEF_ARG) _REENTRANT_ ;			// -> from json string
void va_arg_add(VM_DEF_ARG) _REENTRANT_ ;
void va_arg_set(VM_DEF_ARG) _REENTRANT_ ;		
void va_arg_remove(VM_DEF_ARG) _REENTRANT_ ;
void va_arg_dequeue(VM_DEF_ARG) _REENTRANT_;
void va_arg_pop(VM_DEF_ARG) _REENTRANT_;
void va_arg_first(VM_DEF_ARG) _REENTRANT_ ;
void va_arg_try_next(VM_DEF_ARG) _REENTRANT_ ;
void va_dump_var(VM_DEF_ARG);

//toolkit (21-36)
void va_select_item(VM_DEF_ARG) _REENTRANT_;
void va_display_text(VM_DEF_ARG) _REENTRANT_;
void va_get_input(VM_DEF_ARG) _REENTRANT_;
void va_set_timer(VM_DEF_ARG) _REENTRANT_ ; 
//invoke external
void va_invoke_external(VM_DEF_ARG) _REENTRANT_ ;
//picc APIs
void va_picc_open(VM_DEF_ARG) _REENTRANT_ ;
void va_picc_auth(VM_DEF_ARG) _REENTRANT_;
void va_picc_transmit(VM_DEF_ARG);
void va_picc_init(VM_DEF_ARG);				//init global PICC context
void va_picc_release_all(VM_DEF_ARG);		//release global PICC context
//iso8583 (37-39)
void va_iso_create_message(VM_DEF_ARG) _REENTRANT_ ; 
void va_iso_push_element(VM_DEF_ARG) _REENTRANT_ ;
void va_iso_get_element(VM_DEF_ARG) _REENTRANT_ ;

void va_to_float(VM_DEF_ARG) ;
//generic toolkit
void va_toolkit_create(VM_DEF_ARG) _REENTRANT_ ;  
void va_toolkit_push_ext(VM_DEF_ARG) _REENTRANT_ ;
void va_toolkit_push_raw(VM_DEF_ARG) _REENTRANT_ ;
void va_toolkit_dispatch(VM_DEF_ARG) _REENTRANT_ ;
void va_toolkit_get_result(VM_DEF_ARG) _REENTRANT_ ;
//bit operation 
void va_check_bit(VM_DEF_ARG) _REENTRANT_ ; 
void va_set_bit(VM_DEF_ARG) _REENTRANT_ ;
void va_clear_bit(VM_DEF_ARG) _REENTRANT_ ;
//converter
void va_bin2hex(VM_DEF_ARG) _REENTRANT_;
void va_hex2bin(VM_DEF_ARG) _REENTRANT_; 
void va_bin2dec(VM_DEF_ARG) _REENTRANT_;
void va_dec2bin(VM_DEF_ARG) _REENTRANT_;
void va_b64_encode(VM_DEF_ARG) _REENTRANT_ ;
void va_b64_decode(VM_DEF_ARG) _REENTRANT_ ;
//codec
void va_crypto_create(VM_DEF_ARG) _REENTRANT_ ;
void va_crypto_encrypt(VM_DEF_ARG) _REENTRANT_ ; 
void va_crypto_decrypt(VM_DEF_ARG) _REENTRANT_ ;
void va_random(VM_DEF_ARG) _REENTRANT_ ;
void va_digest(VM_DEF_ARG) _REENTRANT_ ;
//security
void va_verify_pin(VM_DEF_ARG) _REENTRANT_ ;
//toolkit manager			  
void va_terminal_profile(VM_DEF_ARG) _REENTRANT_ ; 
//cross APIs
void va_wib_set_return_var(VM_DEF_ARG) _REENTRANT_ ; 
void va_get_info(VM_DEF_ARG) _REENTRANT_ ;
//default syscall return
vm_object * va_syscall_ret(VM_DEF_ARG, uint8 size, uint8 * buffer) _REENTRANT_ ;

void va_delay(VM_DEF_ARG);
void va_ui_init(VM_DEF_ARG);
void va_ui_release_all(VM_DEF_ARG);
void va_ui_alert(VM_DEF_ARG);
void va_ui_create_window(VM_DEF_ARG);
void va_ui_destroy_window(VM_DEF_ARG);
void va_ui_create_label(VM_DEF_ARG);
void va_ui_create_button(VM_DEF_ARG);
void va_ui_create_textbox(VM_DEF_ARG);
void va_ui_create_image(VM_DEF_ARG);
void va_ui_get_text(VM_DEF_ARG);
void va_ui_set_text(VM_DEF_ARG);
void va_ui_wait(VM_DEF_ARG);
//void va_ui_present(tk_context_p ctx, int32 ms) ;
void va_ui_push_window(VM_DEF_ARG);			//non blocking
void va_ui_pop_window(VM_DEF_ARG);				//non blocking
//IO com APIs
void va_com_init(VM_DEF_ARG) ;		//system startup
void va_com_open(VM_DEF_ARG);
void va_com_transmit(VM_DEF_ARG);
void va_com_readline(VM_DEF_ARG);
//IO port APIs
void va_port_open(VM_DEF_ARG);
//network APIs
void va_net_open(VM_DEF_ARG);
void va_net_transmit(VM_DEF_ARG);
void va_net_mail_create(VM_DEF_ARG);
void va_net_mail_send(VM_DEF_ARG);
//dynamic APIs
void va_dlopen(VM_DEF_ARG);
void va_dlexec(VM_DEF_ARG);
void va_dlcast(VM_DEF_ARG);

//OS APIs
void va_sleep(VM_DEF_ARG);
void va_create_thread(VM_DEF_ARG);
void va_thread_start(VM_DEF_ARG);
void va_thread_stop(VM_DEF_ARG);


//data casting global init
void va_data_init();

//default util
int32 va_o2f(VM_DEF_ARG, vm_object * obj) _REENTRANT_ ;
void va_return_word(VM_DEF_ARG, int32 val) _REENTRANT_ ;
int va_get_length(uint8 * buffer, int * length);
int va_pop_lv(uint8 * buffer, uint8 * dst, int * length);
int va_push_lv(uint8 * buffer, uint8 * src, uint16 length);
int va_arg_count_t(uint8 * bytes, int wlen);
uint8 va_arg_is_numeric(uint8 * str, int len);
vm_object * va_arg_operation(VM_DEF_ARG, uint8 opcode, vm_object * op1, vm_object * op2);
uint8 va_contain_delimiter(uint8 * str, int len);
uint32 vm_bin2hex(uint8 * bytes, uint32 len, uint8 * hexstring);

//extension apis
vm_object * va_create_ext_float(VM_DEF_ARG, uint16 length, uchar * bytes);
vm_object * va_data_cast(VM_DEF_ARG, vm_object * param, uint16 ptype) ;

//misc apis for array creation
va_buffer * va_create_buffer(uint16 tag, uint32 size, uint8 * content);
va_buffer * va_add_buffer(va_buffer * head, uint16 tag, uint32 size, uint8 * content);
va_buffer * va_create_array_from_buffer(va_buffer * head);
void va_release_buffer(va_buffer * head);

#ifdef __cplusplus
}
#endif
#define VM_FRAMEWORK__H
#endif
