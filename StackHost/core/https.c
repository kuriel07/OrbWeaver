/* echoserver.c
 *
 * Copyright (C) 2006-2015 wolfSSL Inc.
 *
 * This file is part of wolfSSL. (formerly known as CyaSSL)
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */
#include <stdio.h>

#include <config.h>

#include "core/port.h"
#include "core/http.h"
#include "core/global.h"
#ifdef WIN32
#include "util/pthread.h"
#else
#include <pthread.h>
#endif

#if defined(USE_WOLFSSL)
//#include <wolfssl/internal.h> /* name change portability layer */
#include <wolfssl/ssl.h> /* name change portability layer */
#include <wolfssl/wolfcrypt/settings.h>
#ifdef HAVE_ECC
#include <wolfssl/wolfcrypt/ecc.h>   /* ecc_fp_free */
#endif

#if defined(WOLFSSL_MDK_ARM)
        #include <stdio.h>
        #include <string.h>

        #if defined(WOLFSSL_MDK5)
            #include "cmsis_os.h"
            #include "rl_fs.h" 
            #include "rl_net.h" 
        #else
            #include "rtl.h"
        #endif

        #include "wolfssl_MDK_ARM.h"
#endif


#ifndef NO_MAIN_DRIVER
    #define ECHO_OUT
#endif

//#include "examples/echoserver/echoserver.h"

#define SVR_COMMAND_SIZE 256

static void SignalReady(void* args, int16 port)
{
#if defined(_POSIX_THREADS) && defined(NO_MAIN_DRIVER) && !defined(__MINGW32__)
    /* signal ready to tcp_accept */
    func_args* server_args = (func_args*)args;
    tcp_ready* ready = server_args->signal;
    pthread_mutex_lock(&ready->mutex);
    ready->ready = 1;
    ready->port = port;
    pthread_cond_signal(&ready->cond);
    pthread_mutex_unlock(&ready->mutex);
#endif
    (void)args;
    (void)port;
}

static void err_sys(char * c) {
	printf("%s\n", c);
}

#if defined(NO_FILESYSTEM) && !defined(NO_CERTS)

enum {
    WOLFSSL_CA   = 1,
    WOLFSSL_CERT = 2,
    WOLFSSL_KEY  = 3
};

static INLINE void load_buffer(WOLFSSL_CTX* ctx, const char* fname, int type)
{
    /* test buffer load */
    long  sz = 0;
    byte  buff[10000];
    FILE* file = fopen(fname, "rb");

    if (!file)
        err_sys("can't open file for buffer load "
                "Please run from wolfSSL home directory if not");
    fseek(file, 0, SEEK_END);
    sz = ftell(file);
    rewind(file);
    fread(buff, sizeof(buff), 1, file);

    if (type == WOLFSSL_CA) {
        if (wolfSSL_CTX_load_verify_buffer(ctx, buff, sz, SSL_FILETYPE_PEM)
                                          != SSL_SUCCESS)
            err_sys("can't load buffer ca file");
    }
    else if (type == WOLFSSL_CERT) {
        if (wolfSSL_CTX_use_certificate_buffer(ctx, buff, sz,
                    SSL_FILETYPE_PEM) != SSL_SUCCESS)
            err_sys("can't load buffer cert file");
    }
    else if (type == WOLFSSL_KEY) {
        if (wolfSSL_CTX_use_PrivateKey_buffer(ctx, buff, sz,
                    SSL_FILETYPE_PEM) != SSL_SUCCESS)
            err_sys("can't load buffer key file");
    }
    fclose(file);
}

#endif /* NO_FILESYSTEM */

static int https_send(WOLFSSL * wctx, char * buf, int sz, void * ictx) {
	int err = net_send((int)ictx, buf, sz, 0);
	printf("send : %d\n", err);
	return err;
}

static int https_recv(WOLFSSL * wctx, char * buf, int sz, void * ictx) {
	
	int err = net_recv((int)ictx, buf, sz, 0);
	//printf("%d\n",err);
	if(err == 0) return WOLFSSL_CBIO_ERR_TIMEOUT;
	return err;
}

static unsigned int https_psk_callback(WOLFSSL* ctx, const char* buf, unsigned char* buf2, unsigned int sz) {
	printf("PSK callback\n");
	return 0;
}

void * https_task(void * arg)
{
    struct sockaddr_in sa;
    WOLFSSL_METHOD* method = 0;
    WOLFSSL_CTX*    ctx    = 0;

    int    doDTLS = 0;
    int    doPSK = 0;
    int    outCreated = 0;
    int    shutDown = 0;
    int    useAnyAddr = 0;
    WOLFSSL* ssl = 0;
    int     echoSz = 0;
    int     clientfd;
    int     firstRead = 1;
    int     gotFirstG = 0;
	net_instance * instance;
	net_entry * root;
	char request[32768];
	int recv_size;
	char * arguments = NULL;
	char * response;
	int response_length;
    struct sockaddr client;
    int     client_len = sizeof(client);
    //func_args args;
	http_init_arg * init = (http_init_arg *)arg;
#ifdef HAVE_CAVIUM
    int ret = OpenNitroxDevice(CAVIUM_DIRECT, CAVIUM_DEV_ID);
    if (ret != 0)
        err_sys("Cavium OpenNitroxDevice failed");
#endif /* HAVE_CAVIUM */

    wolfSSL_Init();
#if defined(DEBUG_WOLFSSL) && !defined(WOLFSSL_MDK_SHELL)
    wolfSSL_Debugging_ON();
#endif	

#ifdef WOLFSSL_DTLS
    doDTLS  = 1;
#endif

#ifdef WOLFSSL_LEANPSK
    doPSK = 1;
#endif

#if defined(NO_RSA) && !defined(HAVE_ECC)
    doPSK = 1;
#endif

#if defined(NO_MAIN_DRIVER) && !defined(USE_WINDOWS_API) && \
        !defined(WOLFSSL_SNIFFER) && !defined(WOLFSSL_MDK_SHELL) && \
        !defined(WOLFSSL_TIRTOS)
        port = 0;
#endif
#if defined(USE_ANY_ADDR)
        useAnyAddr = 1;
#endif

#ifdef WOLFSSL_TIRTOS
    fdOpenSession(Task_self());
#endif
    int SocketFD = net_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SocketFD == -1) {
      perror("https cannot create socket");
      exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);
  
    sa.sin_family = AF_INET;
    sa.sin_port = htons(init->port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
  
    if (net_bind(SocketFD,(struct sockaddr *)&sa, sizeof sa) == -1) {
      perror("https bind failed");
      net_close(SocketFD);
      exit(EXIT_FAILURE);
    }

    if (net_listen(SocketFD, 11) == -1) {
      perror("https listen failed");
      net_close(SocketFD);
	  return NULL;
    }

#if defined(WOLFSSL_DTLS)
    method  = wolfDTLSv1_2_server_method();
#elif  !defined(NO_TLS)
    method = wolfTLSv1_2_server_method();
#elif defined(WOLFSSL_ALLOW_SSLV3)
    method = wolfSSLv3_server_method();
#else
    #error "no valid server method built in"
#endif

    //SignalReady(args, port);
	for(;;) 
	{
		ctx    = wolfSSL_CTX_new(method);
		wolfSSL_SetIORecv(ctx, https_recv);
		wolfSSL_SetIOSend(ctx, https_send);
		
		/* CyaSSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF); */

#if defined(OPENSSL_EXTRA) || defined(HAVE_WEBSERVER)
		wolfSSL_CTX_set_default_passwd_cb(ctx, PasswordCallBack);
#endif

#if defined(HAVE_SESSION_TICKET) && defined(HAVE_CHACHA) && \
                                    defined(HAVE_POLY1305)
		if (TicketInit() != 0)
			err_sys("unable to setup Session Ticket Key context");
		wolfSSL_CTX_set_TicketEncCb(ctx, myTicketEncCb);
#endif

#ifndef NO_FILESYSTEM
		if (doPSK == 0) {
    #ifdef HAVE_NTRU
			/* ntru */
			if (wolfSSL_CTX_use_certificate_file(ctx, ntruCert, SSL_FILETYPE_PEM)
					!= SSL_SUCCESS)
				err_sys("can't load ntru cert file, "
						"Please run from wolfSSL home dir");

			if (wolfSSL_CTX_use_NTRUPrivateKey_file(ctx, ntruKey)
					!= SSL_SUCCESS)
				err_sys("can't load ntru key file, "
						"Please run from wolfSSL home dir");
#elif defined(HAVE_ECC) && !defined(WOLFSSL_SNIFFER)
			/* ecc */
			if (wolfSSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM)
					!= SSL_SUCCESS)
				err_sys("can't load server cert file, "
						"Please run from wolfSSL home dir");

			if (wolfSSL_CTX_use_PrivateKey_file(ctx, privkey_file, SSL_FILETYPE_PEM)
					!= SSL_SUCCESS)
				err_sys("can't load server key file, "
						"Please run from wolfSSL home dir");
#elif defined(NO_CERTS)
			/* do nothing, just don't load cert files */
#else
			/* normal */
			if (wolfSSL_CTX_use_certificate_file(ctx, svrCert, SSL_FILETYPE_PEM)
					!= SSL_SUCCESS)
				err_sys("can't load server cert file, "
						"Please run from wolfSSL home dir");

			if (wolfSSL_CTX_use_PrivateKey_file(ctx, svrKey, SSL_FILETYPE_PEM)
					!= SSL_SUCCESS)
				err_sys("can't load server key file, "
						"Please run from wolfSSL home dir");
#endif
		} /* doPSK */
#elif !defined(NO_CERTS)
		if (!doPSK) {
			load_buffer(ctx, svrCert, WOLFSSL_CERT);
			load_buffer(ctx, svrKey,  WOLFSSL_KEY);
		}
#endif

#if defined(WOLFSSL_SNIFFER)
		/* don't use EDH, can't sniff tmp keys */
		wolfSSL_CTX_set_cipher_list(ctx, "AES256-SHA");
#endif

		if (doPSK) {
#ifndef NO_PSK
			const char *defaultCipherList;

			wolfSSL_CTX_set_psk_server_callback(ctx, https_psk_callback);
			wolfSSL_CTX_use_psk_identity_hint(ctx, "wolfssl server");
			#ifdef HAVE_NULL_CIPHER
				defaultCipherList = "PSK-NULL-SHA256";
			#elif defined(HAVE_AESGCM) && !defined(NO_DH)
				defaultCipherList = "DHE-PSK-AES128-GCM-SHA256";
			#else
				defaultCipherList = "PSK-AES128-CBC-SHA256";
			#endif
			if (wolfSSL_CTX_set_cipher_list(ctx, defaultCipherList) != SSL_SUCCESS)
				err_sys("server can't set cipher list 2");
#endif
		}
		
	
		while (!shutDown) {
#ifndef WOLFSSL_DTLS
		clientfd = net_accept(SocketFD, &client, &client_len);
		if (0 > clientfd) {
			perror("accept failed");
			net_close(SocketFD);
			//exit(EXIT_FAILURE);
		}
  
#else
		clientfd = SocketFD;
		{
			/* For DTLS, peek at the next datagram so we can get the client's
			 * address and set it into the ssl object later to generate the
			 * cookie. */
			int n;
			byte b[1500];
			n = (int)recvfrom(clientfd, (char*)b, sizeof(b), MSG_PEEK,
							  (struct sockaddr*)&client, &client_len);
			if (n <= 0)
				err_sys("recvfrom failed");
		}
#endif
//if (CYASSL_SOCKET_IS_INVALID(clientfd)) err_sys("tcp accept failed");
		ssl = wolfSSL_new(ctx);
		if (ssl == NULL) err_sys("SSL_new failed");
		printf("https accept client\n");
		wolfSSL_set_fd(ssl, clientfd);
		//custom context
		wolfSSL_SetIOReadCtx(ssl, (void *)clientfd);
		wolfSSL_SetIOWriteCtx(ssl, (void *)clientfd);
#ifdef WOLFSSL_DTLS
			wolfSSL_dtls_set_peer(ssl, &client, client_len);
#endif
#if !defined(NO_FILESYSTEM) && !defined(NO_DH) && !defined(NO_ASN)
			wolfSSL_SetTmpDH_file(ssl, "./dh.temp", SSL_FILETYPE_PEM);
#elif !defined(NO_DH)
		//	SetDH(ssl);  /* will repick suites with DHE, higher than PSK */
#endif
		if (wolfSSL_accept(ssl) != SSL_SUCCESS) {
			printf("SSL_accept failed\n");
			wolfSSL_free(ssl);
			net_close(clientfd);
			continue;
		}
#if defined(PEER_INFO)
		showPeer(ssl);
#endif
			//while ( (echoSz = wolfSSL_read(ssl, command, sizeof(command)-1)) > 0) {
			
			root = thread_arg->root;
			instance = net_instance_create(clientfd, NET_INSTANCE_TYPE_HOST);
			instance->client_iptype = client.sa_family;
			switch(client.sa_family) {
				case AF_INET:		//IPv4
					memcpy(instance->client_ip4, client.sa_data + 2, 4);
					instance->port = ((unsigned)client.sa_data[0] * 256) + (unsigned)client.sa_data[1];
					break;
				case AF_INET6:		//IPv6
					memcpy(instance->client_ip6, client.sa_data + 2, 12);
					instance->port = ((unsigned)client.sa_data[0] * 256) + (unsigned)client.sa_data[1];
					break;
				default:
					break;
			}
			recv_size = wolfSSL_read(ssl, request, sizeof(request)-1);
			request[recv_size] = 0;
			if(recv_size != 0) {
				response = (char *)http_decode(instance, root, (char *)request, recv_size, &response_length);
				if(response != NULL) {
					wolfSSL_write(ssl, response, response_length);
					free(response);
				}
				//cleanup
				net_instance_release(instance);

			}
#ifndef WOLFSSL_DTLS
			wolfSSL_shutdown(ssl);
#endif
			wolfSSL_free(ssl);
			net_close(clientfd);
#ifdef WOLFSSL_DTLS
			tcp_listen(SocketFD, &port, useAnyAddr, doDTLS);
			SignalReady(args, port);
#endif
		}

		net_close(SocketFD);
		wolfSSL_CTX_free(ctx);
		//((func_args*)args)->return_code = 0;

#if defined(NO_MAIN_DRIVER) && defined(HAVE_ECC) && defined(FP_ECC) \
								&& defined(HAVE_THREAD_LS)
		ecc_fp_free();  /* free per thread cache */
#endif

#ifdef WOLFSSL_TIRTOS
		fdCloseSession(Task_self());
#endif

#if defined(HAVE_SESSION_TICKET) && defined(HAVE_CHACHA) && \
										defined(HAVE_POLY1305)
		TicketCleanup();
#endif
	}
    wolfSSL_Cleanup();

#ifdef HAVE_CAVIUM
    CspShutdown(CAVIUM_DEV_ID);
#endif
    return 0;	//args.return_code;
}
#endif

#if defined(USE_OPENSSL)
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

static void err_sys(char * c) {
	printf("%s\n", c);
}

static unsigned int https_psk_callback(SSL* ctx, const char* buf,
	unsigned char* buf2, unsigned int sz) {
	printf("PSK callback\n");
	return 0;
}

int https_read_request(void * ssl, char ** buffer) {
	char request[32768];
	char * header;
	char * temp;
	char * value;
	char * buf = NULL;
	unsigned content_length;
	size_t len;
	int consume_len = 0;
	int needed_len = 0;
	int recv_size = SSL_read((SSL *)ssl, request, sizeof(request));
	if(recv_size < sizeof(request)) {
		//buf = malloc(recv_size + 1);
		//memcpy(buf, request, recv_size);
	//} else {
		//look ahead operation
		request[recv_size] = 0;
		header = istrstr(request, "Content-Length");
		if(header== NULL) {
			//printf("no content-length\n");
			buf = malloc(recv_size + 1);
			memcpy(buf, request, recv_size);
			buf[recv_size] = 0;
			buffer[0] = buf;
			return recv_size;		//unable to read content-length, might be invalid http payload
		}
		//printf("looking for linebreak\n");
		len = http_mp_next_linebreak(header);
		temp = malloc(len + 1);
		if(temp != NULL) {
			memcpy(temp, header, len);
			temp[len] = 0;
			//printf("http_mp_str_next_token\n");
			value = http_mp_str_next_token(temp, ':');
			if(value != NULL) {
				value += 1;		//skip delimiter
				//printf("content length : %s\n", value);
				content_length = atoi(value);
				if(content_length < recv_size) {
					printf("singlepart request %d\n", recv_size);
					buf = malloc(recv_size + 1);
					memcpy(buf, request, recv_size);
					buf[recv_size] = 0;
				} else {
					printf("multipart request %d\n", recv_size);
					needed_len = recv_size + content_length;
					buf = malloc(needed_len + 1);
					memcpy(buf, request, recv_size);
					//if(content_length > recv_size) {
					while(recv_size < needed_len) {
						recv_size += SSL_read(ssl, buf + recv_size, content_length);
						printf("recv_size : %d\n", recv_size);
						buf[recv_size] = 0;
						if(recv_size >= needed_len) break;
					} 
				}
				//printf("total received : %d\n", recv_size);
			}
			free(temp);
		}
	}
	buffer[0] = buf;
	return recv_size;
}

void * https_task(void * arg)
{
	struct sockaddr_in sa;
	const SSL_METHOD* method = 0;
	SSL_CTX*    ctx = 0;

	int    doDTLS = 0;
	int    doPSK = 0;
	int    outCreated = 0;
	int    shutDown = 0;
	int    useAnyAddr = 0;
	SSL* ssl = 0;
	int     echoSz = 0;
	int     clientfd;
	int     firstRead = 1;
	int     gotFirstG = 0;
	net_instance * instance;
	//char request[32768];
	char * request;
	int recv_size;
	char * arguments = NULL;
	char * response;
	int response_length;
	int status;
	net_exec_arg * thread_arg = (net_exec_arg *)arg;
	struct sockaddr client ;
	socklen_t client_len=0;
	int i,j,k;
	int reuse = 1;	//enable reuse of socket during transaction failure happened when request a lot
	//SOCKADDR_IN_T client;
	//socklen_t  client_len = sizeof(struct sockaddr);
	//func_args args;
	http_init_arg * init = (http_init_arg *)arg;
	int SocketFD;
	OPENSSL_init();
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();

	restart_ssl:
	method = TLSv1_2_server_method();

	//SignalReady(args, port);
	for (;;)
	{
		//create_context
		ctx = SSL_CTX_new(TLS_server_method());
		//SSL_SetIORecv(ctx, https_recv);
		//SSL_SetIOSend(ctx, https_send);
		
		/* CyaSSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF); */

		//configure_context
		printf("cert file : %s\n", cert_file);
		printf("key file : %s\n", privkey_file);
		if (doPSK == 0) {
			/* ecc */
			SSL_CTX_set_ecdh_auto(ctx, 1);
			//if ((status = SSL_CTX_use_certificate_chain_file(ctx, "/mnt/d/Git/OrbWeaver/bin/certs/localhost.csr")) <= 0)		//ca certificate
			//	err_sys("can't load server ca file, "
			//		"Please run from OpenSSL home dir");
			
			if ((status = SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM)) <= 0)
				err_sys("can't load server cert file, "
					"Please run from OpenSSL home dir");

			if ((status = SSL_CTX_use_PrivateKey_file(ctx, privkey_file, SSL_FILETYPE_PEM)) <= 0)
				err_sys("can't load server key file, "
					"Please run from OpenSSL home dir");
		} /* doPSK */


		if (doPSK) {
			const char *defaultCipherList;

			SSL_CTX_set_psk_server_callback(ctx, https_psk_callback);
			SSL_CTX_use_psk_identity_hint(ctx, "openssl server");
			defaultCipherList = "PSK-NULL-SHA256";
			//defaultCipherList = "DHE-PSK-AES128-GCM-SHA256";
			if (!SSL_CTX_set_cipher_list(ctx, defaultCipherList))
				err_sys("server can't set cipher list 2");
		}

		
	restart_recv:
		SocketFD = net_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (SocketFD == -1) {
			perror("https cannot create socket");
			exit(EXIT_FAILURE);
		}
		if (setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
			perror("setsockopt(SO_REUSEADDR) failed");

	#ifdef SO_REUSEPORT
		//if (setsockopt(SocketFD, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
		//	perror("setsockopt(SO_REUSEPORT) failed");
	#endif

		memset(&sa, 0, sizeof sa);

		sa.sin_family = AF_INET;
		sa.sin_port = htons(init->port);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);

		if (net_bind(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1) {
			perror("https bind failed");
			net_close(SocketFD);
			exit(EXIT_FAILURE);
		}

		if (net_listen(SocketFD, 1) == -1) {
			perror("https listen failed");
			net_close(SocketFD);
			return NULL;
		}


		restart_accept:
		while (!shutDown) {
			struct sockaddr_in addr;
#if defined(__GNUC__)
			socklen_t len = sizeof(addr);
#endif
#if defined(WIN32) || defined(WIN64)
			int len = sizeof(addr);
#endif
			client_len = sizeof(struct sockaddr);
			//client_len = 0;		//need to reset client_len otherwise segmentation fault
			//kora_print_heap();
			//response = malloc(128);
			//for(i=0;i<64;i++) printf("%02x ", (unsigned char)(response-64)[i]);
			//printf("\n");
			//printf("allocated : %ldbytes\n", (((size_t *) response) - 1)[0]);
			//printf("accept %d\n", SocketFD);
			clientfd = net_accept(SocketFD, (struct sockaddr *)&client, &client_len);
			if (clientfd < 0) {
				perror("accept failed");
				//net_close(SocketFD);
				//exit(EXIT_FAILURE);
			}

			printf("new ssl context\n");
			//if (CYASSL_SOCKET_IS_INVALID(clientfd)) err_sys("tcp accept failed");
			ssl = SSL_new(ctx);
			if (ssl == NULL) err_sys("SSL_new failed");
			//printf("https accept client\n");
			//wolfSSL_set_fd(ssl, clientfd);
			//custom context
			//SSL_SetIOReadCtx(ssl, (void *)clientfd);
			//SSL_SetIOWriteCtx(ssl, (void *)clientfd);
			//SSL_set_bio();
			SSL_set1_host(ssl, "vectorc.art");
			SSL_set_fd(ssl, clientfd);
			//SSL_do_handshake(ssl);

			//SSL_set_rfd(ssl, clientfd);
			//SSL_set_wfd(ssl, clientfd);
			if ((status = SSL_accept(ssl)) <= 0) {
				status = SSL_get_error(ssl, status);
				ERR_print_errors_fp(stderr);
				printf("SSL_accept failed %d\n", status);
			} else {
			//while ( (echoSz = wolfSSL_read(ssl, command, sizeof(command)-1)) > 0) {

				instance = net_instance_create(clientfd, NET_INSTANCE_TYPE_HOST);
				//printf("client : %s\n", client);
				//printf("family : %d\n", client_len);
				//for(i=0;i<client_len-2;i++) printf("%d ", client.sa_data[i] );
				instance->client_iptype = client.sa_family;
				switch(client.sa_family) {
					case 2:		//IPv4
						memcpy(instance->client_ip4, client.sa_data + 2, 4);
						instance->port = ((unsigned)client.sa_data[0] * 256) + (unsigned)client.sa_data[1];
						break;
					default:
						break;
				}
				//recv_size = SSL_read(ssl, request, sizeof(request) - 1);
				recv_size = https_read_request(ssl, &request);
				if (recv_size > 0 && request != NULL) {
					request[recv_size] = 0	;
					//response = (char *)http_decode(instance, root, (char *)request, recv_size, &response_length);
					response = (char *)http_decode(instance, thread_arg->root, (char *)request, recv_size, &response_length);
					//response = "abc";
					//response_length = 3;
					printf("response : %ld\n", response);
					if (response != NULL) {
						SSL_write(ssl, response, response_length);
						free(response);
					}
					free(request);
				}
				else {
					ERR_print_errors_fp(stderr);
				}
				//cleanup
				net_instance_release(instance);

			}
			printf("close connection\n");
			SSL_shutdown(ssl);
			SSL_free(ssl);
			//net_shutdown(clientfd, SHUTDOWN_RDWR);
			net_close(clientfd);
			printf("connection closed\n");
			
		}

		net_close(SocketFD);
		SSL_CTX_free(ctx);
		//((func_args*)args)->return_code = 0;
	}

#if defined(WIN32) || defined(WIN64)
	OPENSSL_cleanup();
#endif
	EVP_cleanup();
	return 0;	//args.return_code;
}
#endif

int https_init(http_init_arg * arg) {
    int ret = -1;
    pthread_t thread;
#if defined(USE_WOLFSSL) || defined(USE_OPENSSL)
	if(pthread_create(&thread, NULL, https_task, arg) == 0) {
		//thread executed
		  ret = 0;
	}
#else
	if(pthread_create(&thread, NULL, https_task, arg) == 0) {
		//thread executed
		  ret = 0;
	}

#endif
	return ret;
}
