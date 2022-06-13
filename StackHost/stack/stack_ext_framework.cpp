#include "defs.h"
#include "config.h"	
#include "Stack/sem_proto.h"
#include "Stack/lex_proto.h"
#include "Stack/pkg_encoder.h"
#include "Stack/pkg_linker.h"
#include "Stack/asm_streamer.h"
#include "StackVM/vm_stack.h"	
#include "StackVM/vm_framework.h"
#include "crypto/inc/cr_apis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/http.h"
#include "core/net.h"
#include "dal.h"
#include "core/port.h"
#ifndef PTHREAD_H
#include "pthread.h"
#endif
#include "util.h"
#ifdef WIN32
#include "util/dirent.h"
#else
#include "dirent.h"
#endif
#include <time.h>
#if defined(HAVE_SQLITE)
#include <sqlite3.h>
#endif

void va_net_transmit(VM_DEF_ARG);
void va_net_mail_create(VM_DEF_ARG);
void va_net_mail_send(VM_DEF_ARG);
void va_to_datetime(VM_DEF_ARG);
void va_header(VM_DEF_ARG);
void va_dal_create(VM_DEF_ARG);
void va_dal_query(VM_DEF_ARG);
void va_dal_prepare(VM_DEF_ARG);
void va_dal_execute(VM_DEF_ARG);
void va_dal_close(VM_DEF_ARG);
void va_dal_result_read(VM_DEF_ARG);
void va_dal_result_seek(VM_DEF_ARG);
void va_install_callback(VM_DEF_ARG);
void va_eval(VM_DEF_ARG);
void va_opendir(VM_DEF_ARG);

#if defined(HAVE_SQLITE)
void va_new_session(VM_DEF_ARG);
void va_find_session(VM_DEF_ARG);
void va_destroy_session(VM_DEF_ARG);
#endif

typedef struct va_database_context{
	va_default_context base;
	int status;
	void (* query)(VM_DEF_ARG);
	void (* prepare)(VM_DEF_ARG);
	void (* execute)(VM_DEF_ARG);
} va_database_context;

typedef struct va_database_result {
	va_default_context base;
	int status;
} va_database_result;

CONST vm_api_entry g_vaRegisteredExtApis[] = {
	//network APIs		(144-148)
	{144, "net_open", va_net_open, NULL },			//create a new connection (net_context), whose behaviour similar with file context
	{145, "net_transmit", va_net_transmit, NULL},		//(param1 = address, param2=payload (OWB array), param3 = method (GET, POST), param4=type(TCP, UDP)  
	{148, "mail_create", va_net_mail_create, NULL},	//param1=server, param2=port, param3=username, param4=password
	{149, "mail_send", va_net_mail_send, NULL},		//param1=handle, param2=to, param3=subject, param4=msg, param5=from
	//install callback
	{262, "install", va_install_callback, NULL },
	{263, "eval", va_eval, NULL},				//evaluate statement and add it to current program tree
	{264, "dump", va_dump_var, NULL},
	{270, "time", va_to_datetime, NULL},
	//files/directory enumerator APIs
	{280, "opendir", va_opendir, NULL },
	//database APIs
	{384, "database", va_dal_create, NULL},
	{385, "query", va_dal_query, NULL},
	{386, "prepare", va_dal_prepare, NULL},
	{387, "execute", va_dal_execute, NULL},
	//web application APIs
	{512, "header", va_header, NULL},
#if defined(HAVE_SQLITE)
	//web session APIs
	{516, "new_session", va_new_session, NULL},
	{517, "find_session", va_find_session, NULL},
	{518, "destroy_session", va_destroy_session, NULL},
#endif
	{0, "", NULL, NULL}
};

#if defined(HAVE_SQLITE)
static sqlite3 * g_session_db;
#endif

#ifdef __cplusplus
extern "C" {
#endif
void va_ext_init() {
	int rc;
#if defined(HAVE_SQLITE)
   	rc = sqlite3_open("./cache.db", &g_session_db);
	if( rc ) {
      		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(g_session_db));
      		g_session_db = NULL;
		while(1);
   	} else {
      		fprintf(stdout, "Opened database successfully\n");
   	}
#endif
}

void va_register_system_apis() {
	//BYTE i;
	vm_api_entry * iterator = (vm_api_entry *)&g_vaRegisteredApis;
	while(iterator->entry != NULL) {
		printf("install api %s : %d\r\n", iterator->name, iterator->id);
		sp_install_api((uchar *)iterator->name, iterator->id, 0);
		iterator++;
	} 
#if defined(VM_EXT_APIS)
	iterator = (vm_api_entry *)&g_vaRegisteredExtApis;
	while(iterator->entry != NULL) {
		printf("install api %s : %d\r\n", iterator->name, iterator->id);
		sp_install_api((uchar *)iterator->name, iterator->id, 0);
		iterator++;
	} 
#endif
}
#ifdef __cplusplus
}
#endif

void va_net_mail_create(VM_DEF_ARG) {
	
}

void va_net_mail_send(VM_DEF_ARG) {

}

void va_header(VM_DEF_ARG) {
	uint8 buffer[4096];
	uint16 len;
	char * str;
	net_param * param = NULL;
	net_param * param2 = NULL;
	vm_object * key = vm_get_argument(VM_ARG, 0);
	vm_object * value = vm_get_argument(VM_ARG, 1);
	net_instance * instance = (net_instance *)VM_ARG->data;
	if(instance == NULL) return;
	len = (key->len >= 4096)?4095:key->len;
	memcpy(buffer, key->bytes, len);
	buffer[len] = 0;
	if((str = (char *)strstr((const char *)buffer, ":")) != NULL) {
		str[0] = 0;
		param2 = net_param_get(instance->response_headers, (char *)buffer);
		param = net_param_create((char *)buffer, str + 2, len);
	} else {
		param2 = net_param_get(instance->response_headers, (char *)buffer);
		if(vm_get_argument_count(VM_ARG) < 2) goto exit_set_operation;
		str = (char *)malloc(value->len + 1);
		len = value->len;
		if(str != NULL) {
			memcpy(str, value->bytes, value->len);
			str[len] = 0;
			param = net_param_create((char *)buffer, str, len);
			free(str);
		}
	}
	if(param2 != NULL) {		
		//header field already exist
		param2 = net_response_header_set(instance, param2, param);	
		//release param2
		if(param2 != NULL) net_param_release(param2);
		param2 = param;
	} else {					
		//header field didn't exist
		net_response_header_add(instance, param);
		param2 = param;
	}
exit_set_operation:
	vm_set_retval(vm_create_object(VM_ARG, strlen((const char *)param2->value), (uint8 *)param2->value));
};

void va_dal_create(VM_DEF_ARG) {
	char buffer[4096];
	va_database_context db_ctx;
	dal_conn * conn;
	uint16 len;
	vm_object * strconn = vm_get_argument(VM_ARG, 0);
	memset(&db_ctx, 0, sizeof(va_database_context));
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, strconn->bytes, strconn->len);
	conn = dal_create_conn(buffer);
	if(conn != NULL) {
		((va_default_context *)&db_ctx)->ctx = conn;
		((va_default_context *)&db_ctx)->offset = 0;
		((va_default_context *)&db_ctx)->close = va_dal_close;
		db_ctx.execute = va_dal_execute;
		db_ctx.query = va_dal_query;
		db_ctx.prepare = va_dal_prepare;
		db_ctx.status = dal_open(conn);		//try opening connection
		vm_set_retval(vm_create_object(VM_ARG, sizeof(va_database_context), (uint8 *)&db_ctx));
	}
}

void va_dal_close(VM_DEF_ARG) {
	va_database_context * db_ctx;
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	db_ctx = (va_database_context *)obj->bytes;
	if(obj->len == sizeof(va_database_context)) {
		if(db_ctx->status != 0) {
			dal_close((dal_conn *)((va_default_context *)db_ctx)->ctx);
		}
		db_ctx->status = 0;
	}
}

static void va_dal_return(VM_DEF_ARG, dal_result * result) {
	va_database_result db_res;
	memset(&db_res, 0, sizeof(va_database_result));
	((va_default_context *)&db_res)->ctx = result;
	((va_default_context *)&db_res)->read = va_dal_result_read;
	((va_default_context *)&db_res)->seek = va_dal_result_seek;
	vm_set_retval(vm_create_object(VM_ARG, sizeof(va_database_result), (uint8 *)&db_res));
}

void va_dal_query(VM_DEF_ARG) {
	va_database_context * db_ctx;
	dal_result * result;
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	vm_object * stmt = vm_get_argument(VM_ARG, 1);
	char * str;
	db_ctx = (va_database_context *)obj->bytes;
	if(obj->len == sizeof(va_database_context)) {
		str = (char *)malloc(stmt->len + 1);
		memcpy(str, stmt->bytes, stmt->len);
		str[stmt->len] = 0;
		result = dal_query((dal_conn *)((va_default_context *)db_ctx)->ctx, str);
		if(result != NULL) {
			va_dal_return(VM_ARG, result);
		}
		free(str);
	}
}

void va_dal_prepare(VM_DEF_ARG) {
	//return dal_prepared_statement
	va_database_context * db_ctx;
	dal_prepared_statement * stmt = NULL;
	char * str, *strq;
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	vm_object * name = vm_get_argument(VM_ARG, 1);
	vm_object * query = vm_get_argument(VM_ARG, 2);
	db_ctx = (va_database_context *)obj->bytes;
	if(obj->len == sizeof(va_database_context)) {
		str = (char *)malloc(name->len + 1);
		memcpy(str, name->bytes, name->len);
		str[name->len] = 0;
		strq = (char *)malloc(query->len + 1);
		memcpy(strq, query->bytes, query->len);
		strq[query->len] = 0;
		stmt = dal_prepare((dal_conn *)((va_default_context *)db_ctx)->ctx,
			str, strq);
		if(stmt != NULL) {
			vm_set_retval(vm_create_object(VM_ARG, sizeof(dal_prepared_statement *), (uint8 *)stmt));
		}
		free(strq);
		free(str);
	}
}

void va_dal_execute(VM_DEF_ARG) {
	dal_result * result;
	char * str;
	int count = 0;
	int i;
	char buffer[4096];
	int len;
	va_database_context * db_ctx;
	dal_prepared_statement * stmt;
	char * args[128];
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	vm_object * name = vm_get_argument(VM_ARG, 1);
	vm_object * arg;
	db_ctx = (va_database_context *)obj->bytes;
	if(obj->len == sizeof(va_database_context)) {
		str = (char *)malloc(name->len + 1);
		memcpy(str, name->bytes, name->len);
		str[name->len] = 0;
		stmt = dal_conn_find_statement((dal_conn *)((va_default_context *)db_ctx)->ctx, str);
		if(stmt != NULL) {
			for(count =0; count< (vm_get_argument_count(VM_ARG) - 2); count++) {
				arg = vm_get_argument(VM_ARG, count + 2);
				if(vm_object_get_type(arg) == VM_EXT_MAGIC ) {
					//for extension type convert to text first
					len = ((vm_extension *)arg->bytes)->apis->text(VM_ARG, arg, (uint8 *)buffer);
					args[count] = (char *)malloc(len + 1);
					memcpy(args[count], buffer, len);
					args[count][len] = 0;
				} else {
					args[count] = (char *)malloc(arg->len + 1);
					memcpy(args[count], arg->bytes, arg->len);
					args[count][arg->len] = 0;
				}
			}
			result = dal_execute((dal_conn *)((va_default_context *)db_ctx)->ctx, stmt, count, args);
			if(result != NULL) {
				va_dal_return(VM_ARG, result);
			}
			for(i=0;i<count;i++) {
				free(args[i]);
			}
		}
		free(str);
	}
}

void va_dal_result_read(VM_DEF_ARG) {
	dal_result * result;
	va_database_result * db_res;
	dal_data * iterator;
	va_buffer * head = NULL;
	va_buffer * obj_arr;
	int offset = 0;
	vm_object * retval;
	dal_array * arr;
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	db_res = (va_database_result *)obj->bytes;
	if(obj->len == sizeof(va_database_result)) {
		result = (dal_result *)((va_default_context *)db_res)->ctx;
		if(result == NULL) return;
		if(result->conn == NULL) return;
		offset = ((va_default_context *)db_res)->offset;
		if(offset == 0) arr = dal_fetch_first(result);
		else arr = dal_fetch_next(result);
		if(arr != NULL) {
			((va_default_context *)db_res)->offset = offset + 1;
			iterator = (dal_data *)arr;
			while(iterator != NULL) {
				if(head == NULL) head = va_create_buffer(ASN_TAG_OCTSTRING, iterator->length, iterator->data);
				else va_add_buffer(head, ASN_TAG_OCTSTRING, iterator->length, iterator->data);
				iterator = (dal_data *)((dal_node *)iterator)->next;
			}
			if(head != NULL) {
				obj_arr = va_create_array_from_buffer(head);
				va_release_buffer(head); 
				if(obj_arr != NULL) {
					//printf("array created %d\n", obj_arr->length);
					//for(int i=0;i<obj_arr->length;i++) printf("%02x", obj_arr->buffer[i]);
					vm_set_retval((retval = vm_create_object(VM_ARG, obj_arr->length, (uint8 *)obj_arr->buffer)));
					vm_get_retval()->mgc_refcount |= VM_OBJ_MAGIC;
					va_release_buffer(obj_arr);
				}
			}
		}
	}
}

void va_dal_result_seek(VM_DEF_ARG) {
	//argument 0 = set to start, -1 set to last (count)
	dal_result * result;
	va_database_result * db_res;
	dal_array * iterator;
	va_buffer * head = NULL;
	va_buffer * obj_arr;
	int offset = 0;
	dal_array * arr;
	int count = 0;
	int i;
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	int32 fset = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	db_res = (va_database_result *)obj->bytes;
	if(obj->len == sizeof(va_database_result)) {
		count = dal_num_rows((result = (dal_result *)((va_default_context *)db_res)->ctx));
		if(fset == 0) {
			db_res->base.offset = 0;
		} else if(fset >= count || fset < 0) {		//ask for count
			db_res->base.offset = count-1;
			fset = count;
		} else {
			db_res->base.offset = fset;
		}
		if(result->records == NULL) dal_fetch_all(result);
		iterator = (dal_array *)result->records;
		dal_dump_result(result);
		for(i=0;i<db_res->base.offset && iterator != NULL;i++) {
			iterator = (dal_array *)((dal_node *)iterator)->next_node;
		}
		result->enumerator = iterator;
		va_return_word(VM_ARG, fset);
	}
}

void va_install_callback(VM_DEF_ARG) {
	net_callback * callback;
	vm_object * obj = vm_get_argument(VM_ARG, 0);
	if(obj->len == 0) return;
	callback = (net_callback *)vm_install_callback(VM_ARG, (vm_function *)obj->bytes);
	if(callback != NULL) vm_set_retval(vm_create_object(VM_ARG, strlen(callback->name), (uchar *)callback->name));
}


uint16 va_timetick_text(VM_DEF_ARG, vm_object *, uint8 *);
uint16 va_datetime_text(VM_DEF_ARG, vm_object *, uint8 *);
vm_object * va_datetime_add(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_datetime_mul(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_datetime_div(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_datetime_sub(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_datetime_mod(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_datetime_and(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_datetime_or(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_datetime_xor(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_datetime_shl(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_datetime_shr(VM_DEF_ARG, vm_object *, vm_object *);
vm_object * va_datetime_not(VM_DEF_ARG, vm_object *);

vm_custom_opcode timetick_op_table = {
	va_timetick_text,
	va_datetime_add,
	va_datetime_mul,
	va_datetime_div,
	va_datetime_sub,
	va_datetime_mod,
	va_datetime_and,
	va_datetime_or,
	va_datetime_xor,
	va_datetime_shl,
	va_datetime_shr,
	va_datetime_not
};

vm_custom_opcode datetime_op_table = {
	va_datetime_text, 
	va_datetime_add, 
	va_datetime_mul, 
	va_datetime_div, 
	va_datetime_sub, 
	va_datetime_mod,
	va_datetime_and,
	va_datetime_or,
	va_datetime_xor,
	va_datetime_shl,
	va_datetime_shr,
	va_datetime_not
};

static void va_object_to_ticktime(VM_DEF_ARG, vm_object * op, ticktime * t) {
	uint8 bbuf[128];
	double f;
	uint8 * pbuf = bbuf;
	if (vm_object_get_type(op) == VM_EXT_MAGIC && ((vm_extension *)op->bytes)->tag == ASN_TAG_UTCTIME) {
			t[0] = ((ticktime *)(((vm_extension *)op->bytes)->payload))[0];
	}
	else {
		if (op->len > sizeof(bbuf)) pbuf = (uint8 *)malloc(op->len);
		if (vm_is_precision(bbuf, vm_object_get_text(VM_ARG, op, pbuf))) {
			f = atof((const char *)bbuf);
			t->sec = (long)f;
			t->ns = ((long)(f * 1000000) % 1000000);
		}
		else {
			t->sec = atol((const char *)pbuf);
			t->ns = 0;
		}
		if (pbuf != bbuf) free(pbuf);
	}
}

uint16 va_timetick_text(VM_DEF_ARG, vm_object * op, uint8 * text) {
	ticktime tick;
	datetime t;
	uint8 buffer[256];
	if (vm_object_get_type(op) == VM_EXT_MAGIC) {
		tick = ((ticktime *)((vm_extension *)op->bytes)->payload)[0];
		sprintf(_RECAST(char *, text), "%d.%d", tick.sec, (tick.ns / 1000));
	}
	return strlen(_RECAST(const char *, text));
}

uint16 va_datetime_text(VM_DEF_ARG, vm_object * op, uint8 * text) { 
	long l;
	ticktime tick;
	datetime t;
	uint8 buffer[256];
	if(vm_object_get_type(op) == VM_EXT_MAGIC) {
		tick = ((ticktime *)((vm_extension *)op->bytes)->payload)[0];
		if(util_iso8601_from_filetime(&t, tick.sec)  == 0) {
#if defined(WIN32) || defined(WIN64)
			TIME_ZONE_INFORMATION tz_info;
			GetTimeZoneInformation(&tz_info);
			t.tz = 0 - (tz_info.Bias / 60);
#endif
			util_iso8601_encode(&t, buffer);
			sprintf(_RECAST(char *, text), "%s", buffer);
		}
  	}
 	return strlen(_RECAST(const char *, text));
}

vm_object * va_datetime_add(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	ticktime d1, d2;
	va_object_to_ticktime(VM_ARG, op1, &d1);
	va_object_to_ticktime(VM_ARG, op2, &d2);
 	d1.sec = d1.sec + d2.sec;
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, &datetime_op_table, sizeof(ticktime), (uint8 *)&d1);
}

vm_object * va_datetime_mul(VM_DEF_ARG, vm_object * op1, vm_object * op2) {
	ticktime d1, d2;
	double dd1, dd2;
	va_object_to_ticktime(VM_ARG, op1, &d1);
	va_object_to_ticktime(VM_ARG, op2, &d2);
	dd1 = d1.sec + ((double)(d1.ns) / 1000000);
	dd2 = d2.sec + ((double)(d2.ns) / 1000000);
	dd1 = dd1 * dd2;
	d1.sec = (long)dd1;
	d1.ns = (long)(dd1 * 1000000) % 1000000;
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, ((vm_extension *)op1->bytes)->apis, sizeof(ticktime), (uint8 *)&d1);
}

vm_object * va_datetime_div(VM_DEF_ARG, vm_object * op1, vm_object * op2) {
	ticktime d1, d2;
	double dd1, dd2;
	va_object_to_ticktime(VM_ARG, op1, &d1);
	va_object_to_ticktime(VM_ARG, op2, &d2);
	dd1 = d1.sec + ((double)(d1.ns) / 1000000);
	dd2 = d2.sec + ((double)(d2.ns) / 1000000);
	dd1 = dd1 / dd2;
	d1.sec = (long)dd1;
	d1.ns = (long)(dd1 * 1000000) % 1000000;
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, ((vm_extension *)op1->bytes)->apis, sizeof(ticktime), (uint8 *)&d1);
}

vm_object * va_datetime_mod(VM_DEF_ARG, vm_object * op1, vm_object * op2) {
	ticktime d1, d2;
	va_object_to_ticktime(VM_ARG, op1, &d1);
	va_object_to_ticktime(VM_ARG, op2, &d2);
 	d1.sec = d1.sec % d2.sec;
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, ((vm_extension *)op1->bytes)->apis, sizeof(ticktime), (uint8 *)&d1);
}

vm_object * va_datetime_sub(VM_DEF_ARG, vm_object * op1, vm_object * op2) {
	ticktime d1, d2;
	int borrow = 0;
	va_object_to_ticktime(VM_ARG, op1, &d1);
	va_object_to_ticktime(VM_ARG, op2, &d2);
	if (d1.ns < d2.ns) {
		borrow = 1; 
		d1.ns += 1000000;
	}
	d1.ns = d1.ns - d2.ns;
	d1.sec = d1.sec - (d2.sec + borrow);
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, &timetick_op_table, sizeof(ticktime), (uint8 *)&d1);
}

vm_object * va_datetime_and(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	ticktime d1, d2;
	va_object_to_ticktime(VM_ARG, op1, &d1);
	va_object_to_ticktime(VM_ARG, op2, &d2);
	d1.sec = d1.sec & d2.sec;
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, ((vm_extension *)op1->bytes)->apis, sizeof(ticktime), (uint8 *)&d1);
}

vm_object * va_datetime_or(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	ticktime d1, d2;
	va_object_to_ticktime(VM_ARG, op1, &d1);
	va_object_to_ticktime(VM_ARG, op2, &d2);
 	d1.sec = d1.sec | d2.sec;
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, ((vm_extension *)op1->bytes)->apis, sizeof(ticktime), (uint8 *)&d1);
}

vm_object * va_datetime_xor(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	ticktime d1, d2;
	va_object_to_ticktime(VM_ARG, op1, &d1);
	va_object_to_ticktime(VM_ARG, op2, &d2);
 	d1.sec = d1.sec ^ d2.sec;
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, ((vm_extension *)op1->bytes)->apis, sizeof(ticktime), (uint8 *)&d1);
}

vm_object * va_datetime_shl(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	ticktime d1, d2;
	va_object_to_ticktime(VM_ARG, op1, &d1);
	va_object_to_ticktime(VM_ARG, op2, &d2);
	d1.sec = d1.sec << d2.sec;
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, ((vm_extension *)op1->bytes)->apis, sizeof(ticktime), (uint8 *)&d1);
}

vm_object * va_datetime_shr(VM_DEF_ARG, vm_object *op1, vm_object *op2) {
	ticktime d1, d2;
	va_object_to_ticktime(VM_ARG, op1, &d1);
	va_object_to_ticktime(VM_ARG, op2, &d2);
	d1.sec = d1.sec >> d2.sec;
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, ((vm_extension *)op1->bytes)->apis, sizeof(ticktime), (uint8 *)&d1);
}

vm_object * va_datetime_not(VM_DEF_ARG, vm_object *op1) {
	ticktime d1, d2;
	va_object_to_ticktime(VM_ARG, op1, &d1);
 	d1.sec = !d1.sec;
	return vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, ((vm_extension *)op1->bytes)->apis, sizeof(ticktime), (uint8 *)&d1);
}

timespec va_get_datetime_now() {
	timespec wtime ;
#if defined(WIN32) || defined(WIN64)
	TIME_ZONE_INFORMATION tz_info;
	time_t t;
	datetime dt;
	tm *tp;
	char bbuf[100];
	timespec_get(&wtime, TIME_UTC);
	time(&t);
	tp = localtime(&t);
	dt.hour = tp->tm_hour;
	dt.minute = tp->tm_min;
	dt.second = tp->tm_sec;
	dt.date = tp->tm_mday;
	dt.month = (tp->tm_mon + 1);
	dt.year = (1900 + tp->tm_year) ;
	dt.days = tp->tm_yday;
	dt.day = tp->tm_wday;
	GetTimeZoneInformation(&tz_info);
	dt.tz = 0 - (tz_info.Bias / 60);
	//dt.tz = 0;
	//util_iso8601_encode(&dt, bbuf);
	wtime.tv_sec = util_iso8601_to_filetime(&dt);
	//strftime(bbuf, sizeof(bbuf), "%D %T\n", gmtime(&wtime.tv_sec));
	//printf(bbuf);
#else
	clock_gettime(CLOCK_REALTIME, &wtime);
#endif
	return wtime;
}

void va_to_datetime(VM_DEF_ARG) _REENTRANT_ {
	//uint16 len = strlen((const char *)key) + strlen((const char *)value) + 2;
	//OS_DEBUG_ENTRY(va_arg_create);
	ticktime l;
	uint16 llen;
	vm_object * obj;
	vm_object * text;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint16 len;
	if(vm_get_argument_count(VM_ARG) > 0) {
		text = vm_get_argument(VM_ARG, 0);
		len = text->len;
		if(len > (VA_OBJECT_MAX_SIZE - 3)) return ;
		vm_memcpy(bbuf, text->bytes, text->len);
		bbuf[text->len] = 0; 
		//l = atol(_RECAST(const char *, bbuf));
		l.sec = util_iso8601_decode((char *)bbuf);
		l.ns = 0;
	} else {
		timespec t = va_get_datetime_now();
		l.sec = t.tv_sec;
		l.ns = t.tv_nsec;
	}
	vm_set_retval(vm_create_extension(VM_ARG, ASN_TAG_UTCTIME, &datetime_op_table, sizeof(ticktime), (uint8 *)&l));
	//OS_DEBUG_EXIT();
}

void va_eval(VM_DEF_ARG) _REENTRANT_ {
	vm_object * text;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	int32 len;
	int err;
	pp_config * pconfig;
	net_bytecodes * ret = NULL;
	pk_object * pkg_root;
	int bytecode_size;
	uint32 headersize = 0;
	char * bytecodes;
	pk_object * last_root = pk_get_last_root(pk_get_root());
	pk_object * new_root;
	text = vm_get_argument(VM_ARG, 0);
	len = text->len;
	if(vm_get_argument_count(VM_ARG) > 0 && len != 0) {
		pconfig = sp_clr_init(_RECAST(uchar *, text->bytes), len );
		va_register_system_apis();
		err = sp_parse();
		if(err == 0) {
			bytecode_size = is_link_optimize(0);
			pkg_root = pk_get_root();
			//reload codebase to new memory
			ret = (net_bytecodes *)malloc(sizeof(net_bytecodes) + bytecode_size);
			memset(ret, 0, sizeof(net_bytecodes) + bytecode_size);
			ret->root = (void *)pkg_root;
			ret->callback = NULL;
			strncpy(ret->path, "", 512);
			memcpy(ret->bytecodes, (uint8 *)is_get_codebuffer(), bytecode_size);
			ret->size = bytecode_size;
			if(last_root != NULL) new_root = (pk_object *)last_root->next;
			else new_root = pkg_root;
			pk_set_codebase((pk_object *)new_root, (uint8 *)ret->bytecodes);
			//load first class
			if(pkg_root != NULL) {
				vm_set_retval( vm_load_class_sta(VM_ARG, ((pk_class *)new_root)->name));
			}
		} else {
			headersize = pk_flush_root();
		}
		sp_cleanup_parser();
		free(pconfig);	
	}
}

#define VA_OPENDIR_FILE			0x01
#define VA_OPENDIR_DIRECTORY	0x04
void va_opendir(VM_DEF_ARG) {
	//opendir(v)
	DIR * dir;
	char * strpath;
	int len, llen;
	struct dirent *ent;
	vm_object * vpath = vm_get_argument(VM_ARG, 0);
	int filter = VA_OPENDIR_FILE | VA_OPENDIR_DIRECTORY;
	int bufsize = 4096;
	int last_index = 0 ;
	uint8 * bbuf = (uint8 *)malloc(bufsize);
	if (vpath->len != 0) {
		strpath = (char *)malloc(vpath->len + 1);
		memcpy(strpath, vpath->bytes, vpath->len);
		strpath[vpath->len] = 0;
		if (vm_get_argument_count(VM_ARG) > 1) filter = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
		dir = opendir(strpath);
		if (dir != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				switch (ent->d_type) {
				case DT_DIR:
					if ((filter & VA_OPENDIR_DIRECTORY) == 0) break;
					if (strcmp(ent->d_name, ".") == 0) break;
					if (strcmp(ent->d_name, "..") == 0) break;
					llen = va_push_lv(bbuf + last_index + 1, (uint8 *)ent->d_name, strlen(ent->d_name));
					bbuf[last_index] = ASN_TAG_OCTSTRING;
					last_index += llen + 1;
					break;
				case DT_REG:
					if ((filter & VA_OPENDIR_FILE) == 0) break;
					llen = va_push_lv(bbuf + last_index + 1, (uint8 *)ent->d_name, strlen(ent->d_name));
					bbuf[last_index] = ASN_TAG_OCTSTRING;
					last_index += llen + 1;
					break;
				default:break;
				}
				if (last_index > (bufsize - 256)) {
					bufsize += 4096;
					bbuf = (uint8 *)realloc(bbuf, bufsize);
				}
			}
			llen = va_push_lv(bbuf + 1, bbuf, last_index);
			bbuf[0] = ASN_TAG_SET;
			vm_set_retval(vm_create_arg(VM_ARG, llen + 1, bbuf));
			closedir(dir);
		}
	}
	if (bbuf != NULL) free(bbuf);
}

#if defined(HAVE_SQLITE)

int va_generate_session_id(VM_DEF_ARG, uint8 * buf) {
	static int nonce = 0;
	uint8 temp[32];
	cr_context crx;
	vm_object * varg = vm_get_argument(VM_ARG, 0);
	cr_randomize(temp, 32 - sizeof(int));
	//[random bytes:28][nonce:4]
	memcpy(temp + 32 - sizeof(int), &nonce, sizeof(int));
	nonce++;
	cr_init_context(&crx, temp);
	cr_calc_sha256(&crx, 0, 32, temp);
	return vm_bin2hex(temp, 32, buf);
	//return 64;
}

static int va_sql_callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

//param[0]=content, param[1]=expiration time (second)
void va_new_session(VM_DEF_ARG) {
	uint8 bbuf[128];
	int rc, len;
	char sql[256];
   	char *zErrMsg = 0;
	sqlite3_stmt *stmt;
	int row_count = 0;
	timespec dt;
	int num_cols; //= sqlite3_column_count(stmt);
	char * id; //= sqlite3_column_text(stmt, 0);
	char * content; //= sqlite3_column_text(stmt, 1);
	int expiration;// = sqlite3_column_int(stmt, 2);
	if(g_session_db == NULL) return;
	vm_object * vcontent = vm_get_argument(VM_ARG, 0);
	time_t wtime = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	len = va_generate_session_id(VM_ARG, bbuf);
	bbuf[len] = 0;
	/* Create SQL statement */;
	sprintf(sql, "SELECT name FROM sqlite_master WHERE type='table' AND name='m_session'", bbuf);
	sqlite3_prepare_v2(g_session_db, sql, -1, &stmt, NULL);
	row_count = 0;
	while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) row_count++;
	sqlite3_finalize(stmt);
	
   	//rc = sqlite3_exec(g_session_db, sql, va_sql_callback, 0, &zErrMsg);
	if( row_count == 0 ){
		sprintf(sql, "CREATE TABLE m_session("  \
      			"id CHAR(64) PRIMARY KEY NOT NULL," \
      			"content TEXT NOT NULL," \
      			"expiration INT NOT NULL)");
		sqlite3_prepare_v2(g_session_db, sql, -1, &stmt, NULL);
		row_count = 0;
		while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) row_count++;
		sqlite3_finalize(stmt);
	}
	sprintf(sql, "SELECT id, content, expiration FROM m_session WHERE id='%s'", bbuf);

	//printf("Performing query...\n");
	sqlite3_prepare_v2(g_session_db, sql, -1, &stmt, NULL);
	//printf("Got results:\n");
	row_count = 0;
	while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
		printf("result : %d\n", rc);
		num_cols = sqlite3_column_count(stmt);
		id = (char *)sqlite3_column_text(stmt, 0);
		content = (char *)sqlite3_column_text(stmt, 1);
		int expiration = sqlite3_column_int(stmt, 2);
		row_count++;
	}

	sqlite3_finalize(stmt);
	if(row_count == 0) {
		dt = va_get_datetime_now();
		sprintf(sql, "INSERT INTO m_session VALUES('%s', '%s', %ld) ", bbuf, vcontent->bytes, dt.tv_sec + wtime);
		sqlite3_prepare_v2(g_session_db, sql, -1, &stmt, NULL);
		row_count = 0;
		while (sqlite3_step(stmt) != SQLITE_DONE) {
			row_count++;
		}
		sqlite3_finalize(stmt);
	}
   	/* Execute SQL statement */
   	//rc = sqlite3_exec(db, sql, va_sql_callback, 0, &zErrMsg);
   	vm_set_retval(vm_create_object(VM_ARG, len, bbuf));
}	

void va_find_session(VM_DEF_ARG) {
	uint8 bbuf[128];
	int rc, len;
	char sql[256];
   	char *zErrMsg = 0;
	sqlite3_stmt *stmt;
	int row_count = 0;
	timespec dt;
	int num_cols; //= sqlite3_column_count(stmt);
	char * id; //= sqlite3_column_text(stmt, 0);
	char * content; //= sqlite3_column_text(stmt, 1);
	int expiration;// = sqlite3_column_int(stmt, 2);
	if(g_session_db == NULL) return;
	vm_object * vid = vm_get_argument(VM_ARG, 0);
	//time_t wtime = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	//len = va_generate_session_id(VM_ARG, bbuf);
	//bbuf[len] = 0;
	memcpy(bbuf, vid->bytes, vid->len);
	bbuf[vid->len] = 0;
	sprintf(sql, "SELECT id, content, expiration FROM m_session WHERE id='%s'", bbuf );

	//printf("Performing query...\n");
	sqlite3_prepare_v2(g_session_db, sql, -1, &stmt, NULL);
	//printf("Got results:\n");
	row_count = 0;
	while (sqlite3_step(stmt) != SQLITE_DONE) {
		num_cols = sqlite3_column_count(stmt);
		id = (char *)sqlite3_column_text(stmt, 0);
		content = (char *)sqlite3_column_text(stmt, 1);
		expiration = sqlite3_column_int(stmt, 2);
		row_count++;
		vm_set_retval(vm_create_object(VM_ARG, strlen(content), content));
		break;
	}

	sqlite3_finalize(stmt);
   	/* Execute SQL statement */
   	//rc = sqlite3_exec(db, sql, va_sql_callback, 0, &zErrMsg);
   	//return vm_create_object(VM_ARG, bbuf, len);
	
}

void va_destroy_session(VM_DEF_ARG) {
	uint8 bbuf[128];
	int rc, len;
	char sql[256];
   	char *zErrMsg = 0;
	sqlite3_stmt *stmt;
	int row_count = 0;
	timespec dt;
	int num_cols; //= sqlite3_column_count(stmt);
	char * id; //= sqlite3_column_text(stmt, 0);
	char * content; //= sqlite3_column_text(stmt, 1);
	int expiration;// = sqlite3_column_int(stmt, 2);
	if(g_session_db == NULL) return;
	vm_object * vid = vm_get_argument(VM_ARG, 0);
	//time_t wtime = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	//len = va_generate_session_id(VM_ARG, bbuf);
	//bbuf[len] = 0;
	memcpy(bbuf, vid->bytes, vid->len);
	bbuf[vid->len] = 0;
	sprintf(sql, "DELETE FROM m_session WHERE id='%s'", bbuf );

	//printf("Performing query...\n");
	sqlite3_prepare_v2(g_session_db, sql, -1, &stmt, NULL);
	//printf("Got results:\n");
	row_count = 0;
	while (sqlite3_step(stmt) != SQLITE_DONE) {
		row_count++;
	}
	sqlite3_finalize(stmt);
	
}
#endif		//HAVE_SQLITE
