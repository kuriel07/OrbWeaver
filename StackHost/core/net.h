#ifndef NET__H
#include "defs.h"
#include "config.h"
#include "http_parser.h"
#include "StackVM/vm_stack.h"

#define OWS_MAX_VARIABLE_NAME	64
#define NET_URL_MAXSIZE			256

typedef struct net_param net_param;
typedef struct net_instance net_instance;
typedef struct net_bytecodes net_bytecodes;
typedef struct net_entry net_entry;
typedef struct net_function net_function;
typedef struct net_callback net_callback;

struct net_param {
	struct net_param * next;
	char key[OWS_MAX_VARIABLE_NAME];
	int length;
	char value[1];
};

#define NET_INSTANCE_TYPE_HOST			2
#define NET_INSTANCE_TYPE_APP			1
#define NET_INSTANCE_URL_SIZE			512
struct net_instance {
	uint16 type;			//type of instance
	uint16 client_iptype;
	uint8 client_ip4[4];		//client ipv4
	uint16 client_ip6[6];		//client ipv6
	uint16 port;		//client port
	int conn;				//current connection id
	void * ssl;			//ssl context (in-case secure communication)
	char url[512];			//current executed uri
	char current_field[OWS_MAX_VARIABLE_NAME];
	http_parser * parser;			//current http parser
	
	net_entry * root;				//top root entry
	net_bytecodes * bytecodes;		//current bytecode executed

	void * context;		//virtual machine instance
	uint16 method;		//POST/GET/DELETE/PUT
	net_param * request_headers;		//linked-list of request headers
	void * request_payload;		//current payload
	uint32 request_size;	//payload size

	uint16 status;			//current status
	char * message;
	net_param * response_headers;		//linked-list of response header
	void * response_payload;
	uint32 response_size;
	void (* response_release)(void *);		//function to release response
	//std::string response_string;

	void (* complete)(net_instance *);
};

struct net_callback {
	vm_function base;
	net_callback * next;
	char name[320];
};

struct net_bytecodes {
	char path[512];		//path of current bytecodes
	void * root;		//pk_object for pk_class, use it with lk_select_class
	void * callback;
	uint32 size;		//size of bytecodes
	pk_tcblock * tc_entries;
	char bytecodes[1];	//content of bytecodes
};

struct net_function {
	char url[NET_URL_MAXSIZE];
	void (* execute)(net_instance * );
};

#define NET_ENTRY_TYPE_NAMESPACE		0x8000
#define NET_ENTRY_TYPE_FILE				0x0001

struct net_entry {
	uint16 type;
	net_entry * parent;
	net_entry * sibling;
	net_entry * child;
	net_bytecodes * bytecodes;
	char name[256];
};

#ifdef __cplusplus
extern "C" {
#endif

int istrncmp(char const *a, char const *b, int max_len);
char* istrstr(char* text, char* pattern);

net_param * net_param_create(char * key, char * value, int max_size);
net_param * net_param_add(net_param * root, net_param * param);
net_param * net_param_get(net_param * root, char * key);
net_param * net_param_remove(net_param * root, net_param * param);
net_param * net_param_update(net_param * root, net_param * old, net_param * param);
net_param * net_request_header_add(net_instance *instance, net_param * param);
net_param * net_response_header_add(net_instance * instance, net_param * param);
net_param * net_request_header_remove(net_instance * instance, net_param * param);
net_param * net_response_header_remove(net_instance * instance, net_param * param);
net_param * net_request_header_set(net_instance * instance, net_param * old, net_param * param);
net_param * net_response_header_set(net_instance * instance, net_param * old, net_param * param);
void net_param_clear(net_param * root);
void net_param_release(net_param * param);

net_instance * net_instance_create(int conn, int type);
void net_instance_release(net_instance * instance);
net_callback * net_callback_create(char * name, int16 arg_count, uint32 offset);
net_callback * net_bytecodes_add_callback(net_bytecodes * bytecodes, net_callback *);
net_callback * net_bytecodes_find_callback(net_bytecodes * bytecodes, char * name);
void net_bytecodes_remove_callback(net_bytecodes * bytecodes, net_callback *);

net_entry * net_entry_create (uint16 type, char * name);
net_entry * net_entry_select(net_entry * parent, char * name);
net_entry * net_entry_add(net_entry * parent, net_entry * entry);
net_param * net_param_dump(net_param * root);
int net_param_print(char * buffer, net_param * param);
net_entry * net_entry_remove(net_entry * entry); 

int net_url_get_ext(char * url, char * ext, int maxlen);
net_entry * net_enumerate_directory(
    char *dirname, int is_root, net_entry * parent);



#ifdef __cplusplus
}
#endif
#define NET__H
#endif
