#include "defs.h"
#include "net.h"
#ifndef HTTP_H

#if defined(USE_OPENSSL)
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

#if defined(__GNUC__)
#include <sys/socket.h>
#include <unistd.h>
#endif

#define NET_MIME_TEXT			0x01
#define NET_MIME_BINARY			0x80
#define NET_MIME_END			0


typedef struct net_exec_arg net_exec_arg;
struct net_exec_arg {
	net_entry * root;
	int ConnectFD;
	net_instance * instance;
    struct sockaddr client;
};

typedef struct http_init_arg http_init_arg;

struct http_init_arg {
	void * root;
	int port;
	void * ctx;
};

typedef struct net_ext_mime net_ext_mime;
struct net_ext_mime {
	int16 type;
	char ext[7];
	char mime[64];
};

typedef struct http_param http_param;

#define HTTP_PARAM_NAME_SIZE 256
#define HTTP_PARAM_FILENAME_SIZE 256
#define HTTP_PARAM_TYPE_SIZE 64
#define HTTP_PARAM_BOUNDARY_SIZE 96
struct http_param {
	struct http_param* next;
	char name[HTTP_PARAM_NAME_SIZE];
	char filename[HTTP_PARAM_FILENAME_SIZE];
	char type[HTTP_PARAM_TYPE_SIZE];
	char boundary[HTTP_PARAM_BOUNDARY_SIZE];
	int size;
	char value[1];
};

//multipart decoder
#define MP_STATE_BOUNDARY 0
#define MP_STATE_HEADER 1
#define MP_STATE_DATA_START 2
#define MP_STATE_DATA_CONTENT 3
#define MP_STATE_END 4

typedef struct mp_context mp_context;

struct mp_context {
	unsigned char state;
	size_t index;
	size_t offset;
	size_t total;
	char* error;
	int nesting;
	char boundary[96];
	unsigned int flags;
};

#ifdef __cplusplus
extern "C" {
#endif

uint32 http_parse(net_instance * instance, char * request, uint32 reqlen);
void * http_decode(net_instance * instance, net_entry * root, char * request, int recv_size, int * resp_len);


int http_read_request(void * conn, char ** buffer);
int https_read_request(void* ssl, char ** buffer) ;
int http_init(http_init_arg * arg) ;
int https_init(http_init_arg * arg);
int kora_init(http_init_arg * arg);


uint16 http_base64_length(uint8 * bufcoded);
uint32 http_base64_encode(uint8 * bytes_to_encode, uint32 in_len, uint8 * outbuf);
uint32 http_base64_decode(uint8 * buffer, uint32 size);
uint16 http_hex2bin(uint8 * hexstring, uint8 * bytes);
uint16 http_bin2hex(uint8 * bytes, uint16 len, uint8 * hexstring);
uint16 http_html_escape(uint8 * src, uint8 * dst);
uint16 http_html_unescape(uint8 * src, uint8 * dst);
int16 http_url_get_param(char * url, uint16 index, char * param) ;

//http payload parameters
http_param* http_param_create(char* key, char* value, int max_size);
http_param* http_param_add(http_param* root, http_param* param);
http_param* http_param_get(http_param* root, char* key);
http_param* http_param_remove(http_param* root, http_param* param);
http_param* http_param_update(http_param* root, http_param* old, http_param* param);
http_param* http_param_dump(http_param* root);
int http_param_print(char* buffer, http_param* param);
void http_param_clear(http_param* root);
void http_param_release(http_param* param);

//multipart-formdata decoder, output http_params (linked list)
mp_context* http_mp_new_context(char * boundary);
size_t http_mp_next_linebreak(char* content);
char* http_mp_str_next_token(char* content, char delim) ;
int http_mp_process_disposition(http_param* param, char* content) ;
void http_mp_decode(mp_context* ctx, http_param** root_param, char* content) ;
void http_mp_cleanup(mp_context * ctx);

#ifdef __cplusplus
}
#endif

#define HTTP_H
#endif
