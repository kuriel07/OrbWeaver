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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
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
	net_exec_arg * thread_arg = (net_exec_arg *)arg;
    //SOCKADDR_IN_T client;
    //int     client_len = sizeof(client);
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
		clientfd = net_accept(SocketFD, NULL, NULL);
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
	net_entry * root;
	char request[32768];
	int recv_size;
	char * arguments = NULL;
	char * response;
	int response_length;
	int status;
	net_exec_arg * thread_arg = (net_exec_arg *)arg;
	//SOCKADDR_IN_T client;
	//int     client_len = sizeof(client);
	//func_args args;
	http_init_arg * init = (http_init_arg *)arg;
#ifdef HAVE_CAVIUM
	int ret = OpenNitroxDevice(CAVIUM_DIRECT, CAVIUM_DEV_ID);
	if (ret != 0)
		err_sys("Cavium OpenNitroxDevice failed");
#endif /* HAVE_CAVIUM */

	OPENSSL_init();
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();

	int SocketFD = net_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SocketFD == -1) {
		perror("https cannot create socket");
		exit(EXIT_FAILURE);
	}

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

#if defined(WOLFSSL_DTLS)
	method = DTLSv1_2_server_method();
#elif  !defined(NO_TLS)
	method = TLSv1_2_server_method();
	method = SSLv23_server_method();
#elif defined(WOLFSSL_ALLOW_SSLV3)
	method = SSLv3_server_method();
#else
#error "no valid server method built in"
#endif

	//SignalReady(args, port);
	for (;;)
	{
		ctx = SSL_CTX_new(method);
		//SSL_SetIORecv(ctx, https_recv);
		//SSL_SetIOSend(ctx, https_send);
		
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
			SSL_CTX_set_ecdh_auto(ctx, 1);
			if ((status = SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM)) <= 0)
				err_sys("can't load server cert file, "
					"Please run from OpenSSL home dir");

			if ((status = SSL_CTX_use_PrivateKey_file(ctx, privkey_file, SSL_FILETYPE_PEM)) <= 0)
				err_sys("can't load server key file, "
					"Please run from OpenSSL home dir");
#elif defined(NO_CERTS)
			/* do nothing, just don't load cert files */
#else
			/* normal */
			if (SSL_CTX_use_certificate_file(ctx, svrCert, SSL_FILETYPE_PEM)
				!= SSL_SUCCESS)
				err_sys("can't load server cert file, "
					"Please run from wolfSSL home dir");

			if (SSL_CTX_use_PrivateKey_file(ctx, svrKey, SSL_FILETYPE_PEM)
				!= SSL_SUCCESS)
				err_sys("can't load server key file, "
					"Please run from wolfSSL home dir");
#endif
		} /* doPSK */
#elif !defined(NO_CERTS)
		if (!doPSK) {
			load_buffer(ctx, svrCert, WOLFSSL_CERT);
			load_buffer(ctx, svrKey, WOLFSSL_KEY);
		}
#endif

#if defined(WOLFSSL_SNIFFER)
		/* don't use EDH, can't sniff tmp keys */
		wolfSSL_CTX_set_cipher_list(ctx, "AES256-SHA");
#endif

		if (doPSK) {
#ifndef NO_PSK
			const char *defaultCipherList;

			SSL_CTX_set_psk_server_callback(ctx, https_psk_callback);
			SSL_CTX_use_psk_identity_hint(ctx, "openssl server");
#ifdef HAVE_NULL_CIPHER
			defaultCipherList = "PSK-NULL-SHA256";
#elif defined(HAVE_AESGCM) && !defined(NO_DH)
			defaultCipherList = "DHE-PSK-AES128-GCM-SHA256";
#else
			defaultCipherList = "PSK-AES128-CBC-SHA256";
#endif
			if (!SSL_CTX_set_cipher_list(ctx, defaultCipherList))
				err_sys("server can't set cipher list 2");
#endif
		}


		while (!shutDown) {
			struct sockaddr_in addr;
#if defined(__GNUC__)
			socklen_t len = sizeof(addr);
#endif
#if defined(WIN32) || defined(WIN64)
			int len = sizeof(addr);
#endif
			
#ifndef WOLFSSL_DTLS
			clientfd = net_accept(SocketFD, (struct sockaddr *)&addr, &len);
			if (clientfd < 0) {
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
			ssl = SSL_new(ctx);
			if (ssl == NULL) err_sys("SSL_new failed");
			//printf("https accept client\n");
			//wolfSSL_set_fd(ssl, clientfd);
			//custom context
			//SSL_SetIOReadCtx(ssl, (void *)clientfd);
			//SSL_SetIOWriteCtx(ssl, (void *)clientfd);
			//SSL_set_bio()
#if defined(WIN32) || defined(WIN64)
			SSL_set1_host(ssl, "localhost");
#endif
			SSL_set_fd(ssl, clientfd);
			//SSL_set_rfd(ssl, clientfd);
			//SSL_set_wfd(ssl, clientfd);
#ifdef WOLFSSL_DTLS
			SSL_dtls_set_peer(ssl, &client, client_len);
#endif
			if ((status = SSL_accept(ssl)) <= 0) {
				status = SSL_get_error(ssl, 0);
				ERR_print_errors_fp(stderr);
				printf("SSL_accept failed %d\n", status);
				SSL_free(ssl);
				net_close(clientfd);
				continue;
			}
			//while ( (echoSz = wolfSSL_read(ssl, command, sizeof(command)-1)) > 0) {

			root = thread_arg->root;
			instance = net_instance_create(clientfd, NET_INSTANCE_TYPE_HOST);
			recv_size = SSL_read(ssl, request, sizeof(request) - 1);
			if (recv_size > 0) {
				request[recv_size] = 0;
				if (recv_size != 0) {
					response = (char *)http_decode(instance, root, (char *)request, recv_size, &response_length);
					if (response != NULL) {
						SSL_write(ssl, response, response_length);
						free(response);
					}
					//cleanup
					net_instance_release(instance);
				}
			}
			else {
				ERR_print_errors_fp(stderr);
			}
#ifndef WOLFSSL_DTLS
			SSL_shutdown(ssl);
#endif
			SSL_free(ssl);
			net_close(clientfd);
#ifdef WOLFSSL_DTLS
			tcp_listen(SocketFD, &port, useAnyAddr, doDTLS);
			SignalReady(args, port);
#endif
		}

		net_close(SocketFD);
		SSL_CTX_free(ctx);
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
	//SSL_Cleanup();

#if defined(WIN32) || defined(WIN64)
	OPENSSL_cleanup();
#endif
	EVP_cleanup();

#ifdef HAVE_CAVIUM
	CspShutdown(CAVIUM_DEV_ID);
#endif
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

#endif
	return ret;
}
