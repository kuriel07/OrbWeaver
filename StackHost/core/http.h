#include "defs.h"
#include "net.h"
#ifndef HTTP_H

#define NET_MIME_TEXT			0x01
#define NET_MIME_BINARY			0x80
#define NET_MIME_END			0


typedef struct net_exec_arg net_exec_arg;
struct net_exec_arg {
	net_entry * root;
	int ConnectFD;
	net_instance * instance;
};

typedef struct http_init_arg http_init_arg;

struct http_init_arg {
	void * root;
	int port;
};

typedef struct net_ext_mime net_ext_mime;
struct net_ext_mime {
	int16 type;
	char ext[7];
	char mime[64];
};

#ifdef __cplusplus
extern "C" {
#endif

uint32 http_parse(net_instance * instance, char * request, uint32 reqlen);
void * http_decode(net_instance * instance, net_entry * root, char * request, int recv_size, int * resp_len);
int http_init(http_init_arg * arg) ;
int https_init(http_init_arg * arg);


uint16 http_base64_length(uint8 * bufcoded);
uint16 http_base64_encode(uint8 * bytes_to_encode, uint16 in_len, uint8 * outbuf);
uint16 http_base64_decode(uint8 * buffer, uint16 size);
uint16 http_hex2bin(uint8 * hexstring, uint8 * bytes);
uint16 http_bin2hex(uint8 * bytes, uint16 len, uint8 * hexstring);
uint16 http_html_escape(uint8 * src, uint8 * dst);
uint16 http_html_unescape(uint8 * src, uint8 * dst);

#ifdef __cplusplus
}
#endif

#define HTTP_H
#endif
