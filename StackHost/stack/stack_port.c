#include "defs.h"
#include "config.h"
#include "Stack/sem_proto.h"
#include "Stack/lex_proto.h"
#include "Stack/pkg_encoder.h"
#include "Stack/pkg_linker.h"
#include "Stack/asm_streamer.h"

#include "StackVM/vm_stack.h"
#include "StackVM/vm_framework.h"
#include "StackHost.h"
#include "core/global.h"
#include "core/http.h"
#include <stdio.h>
#include <stdarg.h>
#include "jansson.h"

/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY	0x00001	/* Lazy function call binding.  */
#define RTLD_NOW	0x00002	/* Immediate function call binding.  */
#define	RTLD_BINDING_MASK   0x3	/* Mask of binding time value.  */
#define RTLD_NOLOAD	0x00004	/* Do not load the object.  */
#define RTLD_DEEPBIND	0x00008	/* Use deep binding.  */

/* If the following bit is set in the MODE argument to `dlopen',
   the symbols of the loaded object and its dependencies are made
   visible as if the object were linked directly into the program.  */
#define RTLD_GLOBAL	0x00100

/* Unix98 demands the following flag which is the inverse to RTLD_GLOBAL.
   The implementation does this by default and so we can define the
   value to zero.  */
#define RTLD_LOCAL	0

/* Do not delete object when closed.  */
#define RTLD_NODELETE	0x01000

void * op_dummy(void * ctx, void * vmat, float f) {
	return vm_create_object(ctx, 0, NULL);
}

uint32 vm_fetch(VM_DEF_ARG, uint32 offset, uchar * buffer, uint32 size) {
	uint32 readed = 0;
	memcpy(buffer, _RECAST(uchar *, vm_get_package()) + offset, size);
	return size;
}

pk_class * stack_port_load_class_pkg(net_entry * entry, char * name) {
	pk_class * pkc;
	net_entry * iterator;
	net_bytecodes * bytecodes;
	if(entry == NULL) return NULL;
	if(entry->bytecodes == NULL) return NULL;
	pkc = lk_select_class((pk_object *)entry->bytecodes->root, (uchar *)name);
	if(pkc == NULL) {
		iterator = entry;
		while(iterator != NULL) {
			if(iterator->child != NULL) pkc = stack_port_load_class_pkg(iterator->child, name);
			if(pkc != NULL) break;
			iterator = iterator->sibling;
		}
	}
	return pkc;
}

pk_class * stack_port_load_class(VM_DEF_ARG, char * name) {
	net_instance * instance = NULL;
	net_entry * iterator;
	instance = (net_instance *)VM_ARG->data;
	pk_class * pkc = lk_select_class(vm_get_info(), (uchar *)name);
	if(pkc == NULL) {
		iterator = instance->root;
		while(iterator != NULL) {
			if(iterator->child != NULL) pkc = stack_port_load_class_pkg(iterator->child, name);
			if(pkc != NULL) break;
			iterator = iterator->sibling;
		}
	}
	return pkc;
}

net_bytecodes * stack_load_file(char * filename, int is_script) {
	uint32 err;
	uint32 ssize, n;
	char * source = NULL;
	uint32 headersize = 0;
	int bytecode_size;
	net_bytecodes * ret = NULL;
	FILE * infile;
	char * bytecodes;
	pp_config * pconfig;
	pk_object * pkg_root;
	as_record * iterator;
	pk_object * first_root = pk_get_root();
	pk_object * last_root = pk_get_last_root(pk_get_root());
	pk_object * new_root;
	//pk_object * temp_root = pk_get_root();
	//pk_set_root(NULL);
	if(is_script != 0) {
		as_init(NULL);
		//pk_init(NULL);
		va_register_system_apis();
		//printf("file : %s\n", filename);
		infile = fopen(filename, "r");
		if(infile == NULL) return 0;
		fseek(infile, 0, SEEK_END);
		ssize = ftell(infile);
		//printf("file size : %d\n", ssize);
		source = (char *)malloc(ssize);
		n = 0;
		fseek(infile, 0, SEEK_SET);
		for(n=0;n<ssize;n++) source[n] = fgetc(infile);
		//printf(source);
		pconfig = sp_clr_init(_RECAST(uchar *, source), ssize );
		err = sp_parse();
		if(err == 0) {
			//printf("link and optimize\n");
			//fflush(0);
			bytecode_size = is_link_optimize(0);
			pkg_root = pk_get_root();
			//reload codebase to new memory
			ret = (net_bytecodes *)malloc(sizeof(net_bytecodes) + bytecode_size);
			memset(ret, 0, sizeof(net_bytecodes) + bytecode_size);
			ret->root = (void *)pkg_root;
			ret->callback = NULL;
			ret->tc_entries = NULL;
			strncpy(ret->path, filename, 512);
			memcpy(ret->bytecodes, (uint8 *)is_get_codebuffer(), bytecode_size);
			ret->size = bytecode_size;
			if (last_root != NULL) {
				//last_root = pk_get_last_root(pk_get_root());
				//last_root->next = new_root;
				//printf("ADD NEW ROOT\n");
				new_root = (pk_object *)last_root->next;
				//new_root = pkg_root;
			}
			else new_root = pkg_root;
			pk_set_codebase((pk_object *)new_root, (uint8 *)ret->bytecodes);
			//pk_dispatch_tc_entries(&new_root, &ret->tc_entries);
			
		} else {
			headersize = pk_flush_root();
		}
		sp_cleanup_parser();
		//lk_set_root(NULL);
		//lk_clear_entries();
			
		free(pconfig);
	} else {
		//pk_init(NULL);
		bytecodes = (char *)lk_decode_file((uchar *)filename, &bytecode_size);
		pkg_root = pk_get_root();
		ret = (net_bytecodes *)malloc(sizeof(net_bytecodes) + bytecode_size);
		memset(ret, 0, sizeof(net_bytecodes) + bytecode_size);
		ret->root = (void *)pkg_root;
		ret->callback = NULL;
		ret->tc_entries = NULL;
		strncpy(ret->path, filename, 512);
		memcpy(ret->bytecodes, (uint8 *)bytecodes, bytecode_size);
		ret->size = bytecode_size;
		if(last_root != NULL) new_root = (pk_object *)last_root->next;
		else new_root = pkg_root;
		//pk_dispatch_tc_entries(&new_root, &ret->tc_entries);
		pk_set_codebase((pk_object *)new_root, (uint8 *)ret->bytecodes);
	}
	//if (first_root != NULL) pk_set_root(first_root);
	return ret;
}

int stack_json_push_argument(vm_instance * instance, json_t * root) {
	size_t i;
	size_t size ;
	char * str;
	int ival;
	float fval;
	char * strbuf[128];
	int num_arg = 0;
	json_t * element;
	size = json_array_size(root);
	for(i=0;i<size;i++) {
		element = json_array_get(root, i);
		switch (json_typeof(element)) {
			case JSON_OBJECT:
				str = (char *)json_string_value(element);
				vm_push_argument(instance, strlen((const char *)str), (uint8 *)str);
				num_arg++;
				break;
			case JSON_ARRAY:
				str = (char *)json_string_value(element);
				vm_push_argument(instance, strlen((const char *)str), (uint8 *)str);
				num_arg++;
				//for (i = 0; i < size; i++) {
				//	stack_json_push_argument(instance, json_array_get(element, i));
				//}
				break;
			case JSON_STRING:
				str = (char *)json_string_value(element);
				vm_push_argument(instance, strlen((const char *)str), (uint8 *)str);
				num_arg++;
				break;
			case JSON_INTEGER:
				ival = json_integer_value(element);
				sprintf((char *)strbuf, "%d", ival);
				vm_push_argument(instance, strlen((const char *)strbuf), (uint8 *)strbuf);
				num_arg++;
				break;
			case JSON_REAL:
				fval = json_real_value(element);
				sprintf((char *)strbuf, "%f", fval);
				vm_push_argument_object(instance, va_create_ext_float(instance, strlen((const char *)strbuf), (uint8 *)strbuf));
				num_arg++;
				break;
			case JSON_TRUE:
				vm_push_argument_object(instance, vm_load_bool(instance, TRUE));
				num_arg++;
				break;
			case JSON_FALSE:
				vm_push_argument_object(instance, vm_load_bool(instance, FALSE));
				num_arg++;
				break;
			case JSON_NULL:
				vm_push_argument_object(instance, vm_create_object(instance, 0, NULL));
				num_arg++;
				break;
			default:
				//fprintf(stderr, "unrecognized JSON type %d\n", json_typeof(element));
				break;
		}
	}
	return num_arg;
}

uint32 stack_exec_bytecodes_json(net_instance * instance, net_bytecodes * b, char * name, char * method, char * arguments) {
	vm_instance * vcit = (vm_instance *)malloc(sizeof(vm_instance));
	char reconstructed_method[512];
	net_callback * callback = NULL;
	int num_arg = 0;
	char * pch;
	int len ;
	json_t * json;
    json_error_t jerror;
	//vm_object * obj;
#if defined(WIN32) || defined(_WIN32) || defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) \
 || defined(__OpenBSD__) || defined(__APPLE__)  || defined(__sun) || defined(__hpux) \
 || defined(__osf__) || defined(__unix__) || defined(LINUX) || defined(_AIX)
	vm_init(vcit, (pk_object *)b->root, 4096 * 1024);		//4MB
#else
	vm_init(vcit, (pk_object *)b->root, MEM_HEAP_SIZE);
#endif
	vcit->stream = vm_stream_open(STREAM_TYPE_MEMORY);
	vcit->data = instance;
	if(instance->parser != NULL) {
		switch(instance->parser->method) {
			case HTTP_DELETE:
			case HTTP_GET:
				if(arguments != NULL && strlen(arguments) != 0) {
					pch = strtok (arguments, "/");
					while (pch != NULL && strlen(pch) != 0)
					{
						//printf("argument %d : %s\n", num_arg, pch);
						char * unescape = (char *)malloc(strlen(pch) + 1);
						if(unescape != NULL) {
							memset(unescape, 0, strlen(pch) + 1);
							len = http_html_unescape((uint8 *)pch, (uint8 *)unescape);
							//if(num_arg == 0) {
							//	callback = net_bytecodes_find_callback(b, unescape);
							//}
							//if(num_arg != 0 || callback == NULL) {
							vm_push_argument(vcit, len, (uint8 *)unescape);
							num_arg++;
							//}
							free(unescape);
						}
						pch = strtok (NULL, "/");
					}
				}
				break;
			case HTTP_POST:
			case HTTP_PUT:
			case HTTP_PATCH:
				//get callback function
				if(arguments != NULL && strlen(arguments) != 0) {
					pch = strtok (arguments, "/");
					if (pch != NULL && strlen(pch) != 0)
					{
						char * unescape = (char *)malloc(strlen(pch) + 1);
						if(unescape != NULL) {
							memset(unescape, 0, strlen(pch) + 1);
							len = http_html_unescape((uint8 *)pch, (uint8 *)unescape);
							callback = net_bytecodes_find_callback(b, unescape);
							free(unescape);
						}
						pch = strtok (NULL, "/");
					}
				}
				if(instance->request_size == 0) break;
				if(instance->request_payload == NULL) break;
				json = json_loads((const char *)instance->request_payload, JSON_DECODE_ANY, &jerror);
				if(json != NULL) {
					if(json_typeof(json) == JSON_ARRAY) {
						num_arg = stack_json_push_argument(vcit, json);
					} else {
						vm_push_argument(vcit, instance->request_size, (uint8 *)instance->request_payload);
						num_arg++;
					}
				}
				break;
			default: break;
		}
	}
	
	sprintf(reconstructed_method, "%s?%d", method, num_arg);
	if(vm_init_exec(vcit, name, reconstructed_method) == 0) {
		if(callback != NULL) {
			//printf("execute callback : %s\n", callback->name);
			vm_exec_function(vcit, (vm_function *)callback);
		}
		//printf("ctx : %x, info : %x\n", vcit, vcit->info);
		vm_decode(vcit, 0, 0);
		vm_close(vcit);
	}
	instance->response_size = vm_stream_get_count(vcit->stream);
	instance->response_payload = malloc(instance->response_size);
	vm_stream_get_bytes(vcit->stream, (uint8 *)instance->response_payload);
	//release stream
	vm_stream_close(vcit->stream);
	free(vcit);
	return instance->response_size;
}

uint32 stack_exec_bytecodes(net_instance * instance, net_bytecodes * b, char * name, char * method, int argc, char * argv[]) {
	vm_instance * vcit = (vm_instance *)malloc(sizeof(vm_instance));
	char reconstructed_method[512];
	net_callback * callback = NULL;
	int num_arg = 0;
	char * pch;
	int len ;
	//vm_object * obj;
#if defined(WIN32) || defined(_WIN32) || defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) \
 || defined(__OpenBSD__) || defined(__APPLE__)  || defined(__sun) || defined(__hpux) \
 || defined(__osf__) || defined(__unix__) || defined(LINUX) || defined(_AIX)
	vm_init(vcit, (pk_object *)b->root, 4096 * 1024);		//4MB
#else
	vm_init(vcit, (pk_object *)b->root, MEM_HEAP_SIZE);
#endif
	if(instance->type == NET_INSTANCE_TYPE_APP) vcit->stream = vm_stream_open(STREAM_TYPE_CONSOLE);
	else vcit->stream = vm_stream_open(STREAM_TYPE_MEMORY);
	vcit->data = instance;
	//push arguments
	for(int i=0;i<argc;i++) {
		vm_push_argument(vcit, strlen(argv[i]), (uint8 *)argv[i]);
	}
	num_arg = argc;
	
	sprintf(reconstructed_method, "%s?%d", method, num_arg);
	printf("method : %s\n", reconstructed_method);
	if(vm_init_exec(vcit, name, reconstructed_method) == 0) {
		if(callback != NULL) {
			printf("execute callback : %s\n", callback->name);
			vm_exec_function(vcit, (vm_function *)callback);
		}
		printf("start decode\n");
		vm_decode(vcit, 0, 0);
		vm_close(vcit);
	} else {
		printf("not found\n");

	}
	instance->response_size = vm_stream_get_count(vcit->stream);
	instance->response_payload = malloc(instance->response_size);
	vm_stream_get_bytes(vcit->stream, (uint8 *)instance->response_payload);
	//release stream
	vm_stream_close(vcit->stream);
	free(vcit);
	return instance->response_size;
}

#ifdef __cplusplus_cli
void stack_setup_modules(net_entry * parent) {
	//dummy function
}
#else
#include "StackVM/dlfcn.h"
void stack_setup_modules() {
	void * iface;
	pk_object * iterator = pk_get_root();
	pk_interface * pki;
	vm_instance * vcit = (vm_instance *)malloc(sizeof(vm_instance));
	char fullpath[512];
	//net_instance * instance;
	void * ret;
	//printf("try registering\n");
	//iface = vm_register_interface(vcit, "XWH");
	//printf("xwh registered\n");
	//vm_interface_add_method(vcit, iface, "+", VM_VAR, 1, CAST_FUNC(op_dummy), VM_FLOAT);
	//printf("added interface method\n");
	lk_dump_classes();
	while(iterator != NULL) {
		//if(iterator->tag 
		if(iterator->tag == PK_TAG_INTERFACE) {
			pki = (pk_interface *)iterator;
#if defined(WIN32) || defined(_WIN32) || defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) \
 || defined(__OpenBSD__) || defined(__APPLE__)  || defined(__sun) || defined(__hpux) \
 || defined(__osf__) || defined(__unix__) || defined(LINUX) || defined(_AIX)
			vm_init(vcit, (pk_object *)pk_get_root(), 4096 * 1024);		//4MB
#else
			vm_init(vcit, (pk_object *)pk_get_root(), MEM_HEAP_SIZE);
#endif
			//instance = net_instance_create(0);
			vcit->stream = vm_stream_open(STREAM_TYPE_CONSOLE);	
			//try load library
			//printf("load library %s\n", fullpath);
#if defined(WIN32) || defined(WIN64)
			//WINDOWS load library
#ifdef STACK_HOST
			sprintf(fullpath, "%s/%s", mod_directory, ((pk_interface *)pki)->libname);
#else
			sprintf(fullpath, "%s", ((pk_interface *)pki)->libname);
#endif
			ret = dlopen((const char *)fullpath, RTLD_GLOBAL);
#else
			//UNIX load library
			//strncpy(mod_directory, "/home/kouryuu/StackLib/bin/modules", 256);
			sprintf(mod_directory, "./modules");
			sprintf(fullpath, "%s/%s", mod_directory, ((pk_interface *)pki)->libname);
			//sprintf(fullpath, "%s", ((pk_interface *)pki)->libname);
			printf("load library : %s\n", (char *)((pk_class *)iterator)->name);
			///ret = dlopen((const char *)fullpath, RTLD_GLOBAL);
			//if(strcmp((char *)((pk_class *)iterator)->name, "libgstreamer_lib.so") == 0) {
			//	ret = dlopen((const char *)fullpath, RTLD_LOCAL);

			//} else {
			ret = dlopen((const char *)fullpath, RTLD_NOW | RTLD_GLOBAL);
			//}
			//ret = dlmopen(-1, (const char *)fullpath, RTLD_NOW);
#endif
			if(ret != NULL) {
				//set codebase to ret indicating base memory for loaded library
				printf("library %s found\n", fullpath);
				((pk_object *)iterator)->codebase = ret;
			} else {
				//library not loaded, set to null

               			fprintf(stderr, "%s\n", dlerror());
				printf("library %s not found\n", fullpath);
				((pk_object *)iterator)->codebase = 0;
				//exit(EXIT_FAILURE);
				//return;
			}
			printf("create interface\n");
			vm_object * iface = vm_create_interface(vcit, (pk_class *)iterator, NULL, NULL);
			vm_interface_exec_sta(vcit, 0, iface, (uint8 *)"setup");
			vm_release_object(vcit, iface);
			//vcit.data = instance;
			vm_stream_close(vcit->stream);
			//net_instance_release(instance);
		}
		iterator = (pk_object *)iterator->next;
	}
	free(vcit);
}
#endif

#if 0
void cgi_about() {
	printf("Orb-Weaver %d.%d.%d CGI Interpreter <br>", IS_MAJOR_VERSION, IS_MINOR_VERSION, BUILD_NUMBER);
	printf("Copyright 2018 @ Orbleaf Technology<br>");
}

void int_about(char * name) {
	vm_api_entry * iterator = (vm_api_entry *)&g_vaRegisteredApis;
	printf("Orb-Weaver %d.%d.%d Interpreter\n", IS_MAJOR_VERSION, IS_MINOR_VERSION, BUILD_NUMBER);
	printf("Copyright 2018 @ Orbleaf Technology\n");
	printf("\nUsage : \n");
	printf("%s [filename]\n", name);
	printf("\nAvailable System APIs : \n");
	while(iterator->entry != NULL) {
		printf("%s()\r\n", iterator->name);
		//sp_install_api(iterator->name, iterator->id, 0);
		iterator++;
	} 
}

int main(int argc, char * argv[]) {
	uint32 err;
	uint32 ssize, n;
	char * source = NULL;
	uint32 headersize = 0;
	FILE * infile;
	pp_config * pconfig;
	as_record * iterator;
	vm_instance vcit;
	as_init(NULL);
	pk_init(NULL);
	vi_register_system_apis();
#ifdef CGI_APPLICATION
	cgi_init();
	cgi_process_form();	
	cgi_init_headers();
	//_istream_code_size = 0;
	if(cgi_param("script")) {
		source = cgi_param("script");
	} else {
		cgi_about();
		cgi_end();
		return 0;
	}
	pconfig = sp_clr_init(_RECAST(uchar *, source), strlen(source));
#else
	if(argc > 1 ) {
		printf("file : %s\n", argv[1]);
		infile = fopen(argv[1], "r");
		if(infile == NULL) return 0;
		fseek(infile, 0, SEEK_END);
		ssize = ftell(infile);
		printf("file size : %d\n", ssize);
		source = (char *)malloc(ssize);
		n = 0;
		fseek(infile, 0, SEEK_SET);
		for(n=0;n<ssize;n++) source[n] = fgetc(infile);
	} else {
		int_about(argv[0]);
		return 0;
	}
	printf(source);
	pconfig = sp_clr_init(_RECAST(uchar *, source), ssize );
#endif
	err = sp_parse();
	//printf("error : %d\n", err);
	if(err == 0) {
		//printf("link and optimize\n");
		//fflush(0);
		is_link_optimize(0);
		headersize = pk_flush_root();
		//lk_clear_entries();
		//bytecodes = gcnew array<System::Byte>(headersize + _istream_code_size);
		//pin_ptr<System::Byte> ptr = &bytecodes[0];
		//memcpy(_RECAST(uchar *, ptr), _pkg_buffer, headersize);
		//memcpy(_RECAST(uchar *, ptr) + headersize, _istream_code_buffer, _istream_code_size);
		//_istream_code_size = 0;
		lk_set_root(pk_get_root());
#ifndef CGI_APPLICATION
		lk_dump_classes();
#endif
		printf("header size : %d\r\n", headersize);
		//iterator = as_get_enumerator();
		//while(iterator != NULL) {
			//lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
		//	printf("%s\n", iterator->buffer);
		//	iterator = as_get_next_record(iterator);
		//}
		vm_init(&vcit, 0, 0);
		vm_decode(&vcit, 0, 0);
		vm_close(&vcit);
	} else {
		
		headersize = pk_flush_root();
		//_istream_code_size = 0;
	}
	lk_clear_entries();
		
	free(pconfig);
	//return bytecodes; 
#ifdef CGI_APPLICATION
	cgi_end();
#endif
	return 0;
}
#endif
