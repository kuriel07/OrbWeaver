
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "defs.h"
#include "config.h"
#include "StackHost.h"
#include "core/http.h"
//#include "core/https.h"
#include "core/net.h"
#include "core/global.h"

#if defined(WIN32) || defined(WIN64)
#include "util/pthread.h"
#else
#include <pthread.h>
#endif

#include "dal.h"
#include <pthread.h>
#include <stdio.h>
#include "minIni.h"

#if defined(WIN32) || defined(WIN64)
#include <WinSock2.h>
#endif

#if defined(__GNUC__)
#include <sys/socket.h>
#include <unistd.h>
#endif

char hostname[128] = { 'l', 'o', 'c','a','l','h','o','s','t',0 };
char inc_directory[512] = {'i','n','c','l','u','d','e','s',0};
char www_directory[512] = {'w','w','w',0};
char mod_directory[512] = { 'm', 'o', 'd','u','l','e','s', 0, 0, 0 };
char cert_file[512] = {'c','e','r','t','s','/','s','e','r','v','e','r','-','e','c','c','.','p','e','m'};
char privkey_file[512] = {'c','e','r','t','s','/','e','c','c','-','k','e','y','.','p','e','m'};
int http_port = 11000;
int https_port = 22000;

void server_start() {
	http_init_arg plain_arg;
	http_init_arg secure_arg;
	static net_entry * root = net_entry_create(NET_ENTRY_TYPE_NAMESPACE, "");
#ifdef WIN32 || WIN64
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
      perror("winsock not initialiazed");
      exit(EXIT_FAILURE);
	}
#endif
	net_enumerate_directory(inc_directory, 1, root);
	stack_setup_modules();
	net_enumerate_directory(www_directory, 1, root);
	plain_arg.port = http_port;
	plain_arg.root = root;
	http_init(&plain_arg);
	secure_arg.port = https_port;
	secure_arg.root = root;
	https_init(&secure_arg);
	//time_t t;
	//time(&t);
	//t =GetSystemTimeAsUnixTime();
	//printf("time : %ld\n", t);
#if 0
	dal_conn * conn = dal_create_conn("pqsql:host=localhost port=5434 dbname=river user=postgres password=ayanami");
	if(dal_open(conn)) {
		printf("open database success\n");
		dal_result * result = dal_query(conn, "SELECT * FROM m_users");
		if(result != NULL) {
			printf("query success\n");
			dal_array * records = dal_fetch_all(result);
			if(records != NULL) {
				printf("fetched\n");
				dal_dump_result(result);
			}
		}
		printf("closing connection\n");
		dal_close(conn);
	} else {
		printf("open database failed\n");
	}
#endif
	while(1) {
		//Sleep(2000);
#if defined(WIN32) || defined(WIN64)
		_sleep(2000);
#endif
#if defined(__GNUC__)
		usleep(2000000);
#endif

	}
}


int parse_ini_callback(const mTCHAR *section, const mTCHAR *key, const mTCHAR *value, void * ctx) {
	if(strcmp(section, "server") == 0) {
		if(strcmp(key, "www_dir")== 0) {
			strncpy(www_directory, value, sizeof(www_directory));
		} else if(strcmp(key, "http_port")== 0) {
			http_port = atoi(value);
		} else if(strcmp(key, "https_port")== 0) {
			https_port = atoi(value);
		} else if(strcmp(key, "hostname")== 0) {
			strncpy(hostname, value, sizeof(hostname));
		} else if(strcmp(key, "cert_file")== 0) {
			strncpy(cert_file, value, sizeof(cert_file));
		} else if(strcmp(key, "privkey_file")== 0) {
			strncpy(privkey_file, value, sizeof(privkey_file));
		}
	} else if(strcmp(section, "app") == 0) {
		if(strcmp(key, "mod_dir")== 0) {
			//GetCurrentDirectory(sizeof(mod_directory), mod_directory);
			//strncat(mod_directory, "/", sizeof(mod_directory));
			strncat(mod_directory, value, sizeof(mod_directory));
		} else if(strcmp(key, "inc_dir")== 0) {
			strncpy(inc_directory, value, sizeof(inc_directory));
		}
	}
	printf("[%s] %s : %s\n", section, key, value);
	return 1;
}

int parse_config(char * path) {
	ini_browse(parse_ini_callback, NULL, path);
	//getchar();
	return 0;
}

int app_start(int argc, char * argv[]) {
	net_bytecodes * bytecodes;
	net_instance * instance;
	char * response_content;
	char file_ext[32];
	int file_ext_len;
	//net_entry * entry;
	printf("entries create\n");
	static net_entry * root = net_entry_create(NET_ENTRY_TYPE_NAMESPACE, "");
	if(argc == 0) return 0;
	printf("enumerate libraries\n");
	net_enumerate_directory(inc_directory, 1, root);
	printf("setup stack\n");
	stack_setup_modules();
	
	printf("extract url\n");
	file_ext_len = net_url_get_ext(argv[0], file_ext, sizeof(file_ext));
	//if(file_ext_len != 0 && strcmp(file_ext, ".orb") == 0) {
	printf("load file : %s\n", argv[0]);
	bytecodes = stack_load_file(argv[0], strcmp(file_ext, ".orb"));
	//lk_decode_file(inpath)
	if(bytecodes != NULL) {
		strncpy(bytecodes->path, argv[0], 512);
		pk_object * iterator = (pk_object *)bytecodes->root;
		if(iterator == NULL) return -1;
		while(iterator != NULL) {
			//entry = net_entry_create(NET_ENTRY_TYPE_FILE, _RECAST(char *, ((pk_class *)iterator)->name));
			//run setup
			instance = net_instance_create(0, NET_INSTANCE_TYPE_APP);
			instance->root = root;
			instance->bytecodes = bytecodes;
			
#if 0
				init_arg = (net_init_arg *)malloc(sizeof(net_init_arg));
				init_arg->instance = instance;
				init_arg->entry = entry;
				pthread_create(&ptid, NULL, net_init_task, init_arg);
#else
			printf("execute bytecodes %s\n", (char *)((pk_class *)iterator)->name);
			stack_exec_bytecodes(instance, bytecodes, (char *)((pk_class *)iterator)->name, (char *)"main", argc - 1, argv + 1);
#if 1
			if((char *)instance->response_payload != NULL) {
				response_content = (char *)malloc(instance->response_size + 1);
				memcpy(response_content, (char *)instance->response_payload, instance->response_size);
				response_content[instance->response_size] = 0;
				free((char *)instance->response_payload);
				//printf(response_content);
				free(response_content);
			}
#endif
			//cleanup
			net_instance_release(instance);
				//end setup execution
#endif
			iterator = (pk_object *)iterator->next;
		}
	}
}

int parse_args(int argc, char ** argv ) {
	FILE * f;
	int state = 0;
	if(argc != 0) {
		for(int i=1;i<argc;i++) {
			//printf("arg[%i] : %s\n", i, argv[i]);
			if(strcmp(argv[i], "-c") == 0){
				state = 2;
			} else if(strcmp(argv[i], "-h") == 0) {
				state = 0;
			} else {
				if((f = fopen(argv[i], "r")) != NULL) {
					fclose(f);
				} else {
					
				}
				switch(state) {
					case 0: //default run as standalone app
						app_start(argc - i, argv + i);
						break;
					case 2:	//path to server config file
						parse_config(argv[i]);
						server_start();
						state = 0;
						break;
					case 1:	//help file
						break;
				}
			}
		}
	}
	return 0;
}

#if defined(__cplusplus)
extern "C" {
#endif
LIB_API int host_main(int argc, char * argv[])
{
	//pthread_init();
	//printf("init\n");
	x_init();		//initialize global buffer
	//GetCurrentDirectory(sizeof(mod_directory), mod_directory);
	//strncat(mod_directory, "/modules", sizeof(mod_directory));
	//printf("parse args\n");
	parse_args(argc, argv);
	//getchar();
	x_cleanup();
	return EXIT_SUCCESS;
}

#if defined(__cplusplus)
}
#endif

