#include "defs.h"
#include "config.h"	
#include "StackVM/vm_stack.h"	
#include "StackVM/vm_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/http.h"
#include "core/net.h"
#include "dal.h"
#include "core/port.h"
#include "util.h"

#if defined(WIN32) || defined(WIN64)
#include <winsock.h>
#include "pthread.h"
#endif

#ifdef __GNUC__
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>
#endif

#ifdef USE_WOLFSSL
//#include <wolfssl/internal.h> /* name change portability layer */
#include <wolfssl/ssl.h> /* name change portability layer */
#include <wolfssl/wolfcrypt/settings.h>
#ifdef HAVE_ECC
    #include <wolfssl/wolfcrypt/ecc.h>   /* ecc_fp_free */
#endif
#endif

#if defined(USE_OPENSSL)
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

typedef struct va_net_thread_arg {
	vm_instance * instance;
	va_net_context * netctx;
	SOCKET sockfd;
	void * wctx;
} va_net_thread_arg;


static void err_sys(char * c) {
	printf("%s\n", c);
}

#ifdef USE_WOLFSSL
static unsigned int va_ssl_psk_callback(WOLFSSL* wctx, const char* buf,
	unsigned char* buf2, unsigned int siz) {
	printf("PSK callback\n");
	return 0;
}

static int va_ssl_send_s(WOLFSSL * wctx, char * buf, int sz, void * ictx) {
	int err = net_send((int)ictx, buf, sz, 0);
	printf("send : %d\n", err);
	return err;
}

static int va_ssl_recv_s(WOLFSSL * wctx, char * buf, int sz, void * ictx) {

	int err = net_recv((int)ictx, buf, sz, 0);
	//printf("%d\n",err);
	if (err == 0) return WOLFSSL_CBIO_ERR_TIMEOUT;
	return err;
}

void * va_ssl_accept_task(void * vctx) {
    WOLFSSL* ssl = 0;
	va_net_context defctx;
	vm_function * func = NULL;
	net_instance * instance;
	vm_instance vcit;
	va_net_thread_arg * thread_arg = (va_net_thread_arg *)vctx;
	vm_instance * vm_inst = (vm_instance *)thread_arg->instance;
	va_net_context * ctx = (va_net_context *)thread_arg->netctx;
	SOCKET clientfd = (SOCKET)thread_arg->sockfd;
	if(ctx == NULL) return NULL;
	func = &ctx->callback;
	if(func == NULL) return NULL;
	instance = (net_instance *)vm_inst->data;
	memcpy(&defctx, ctx, sizeof(va_net_context));

	vm_init(&vcit, vm_inst->info, 2048 * 1024);		//set current PC offset to -1
	vcit.base_address = vm_inst->base_address;
	vcit.stream = vm_stream_open(STREAM_TYPE_MEMORY);
	vcit.data = instance;
	//if (CYASSL_SOCKET_IS_INVALID(clientfd)) err_sys("tcp accept failed");
	ssl = wolfSSL_new((WOLFSSL_CTX *)ctx);
	if (ssl == NULL) err_sys("SSL_new failed");
	printf("ssl accept client\n");
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
	if (wolfSSL_accept(ssl) == SSL_SUCCESS) {
#if defined(PEER_INFO)
		showPeer(ssl);
#endif
		//if(vm_init_exec(&vcit, name, reconstructed_method) == 0) {
		if(func != NULL) {
			vm_exec_function(&vcit, (vm_function *)func);
			defctx.base.ctx = ssl;		
			if(func->arg_count > 0) vm_push_argument(&vcit, sizeof(va_net_context), (uint8 *)&defctx);
			vm_decode(&vcit, 0, 0);
			vm_close(&vcit);
		}
		//}
		instance->response_size = vm_stream_get_count(vcit.stream);
		instance->response_payload = malloc(instance->response_size);
		vm_stream_get_bytes(vcit.stream, (uint8 *)instance->response_payload);
		//release stream
		vm_stream_close(vcit.stream);
		
#ifndef WOLFSSL_DTLS
		wolfSSL_shutdown(ssl);
#endif
	}
	wolfSSL_free(ssl);
	net_close(clientfd);
#ifdef WOLFSSL_DTLS
	tcp_listen(SocketFD, &port, useAnyAddr, doDTLS);
	SignalReady(args, port);
#endif

	//net_close(SocketFD);
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
    //wolfSSL_Cleanup();

#ifdef HAVE_CAVIUM
    CspShutdown(CAVIUM_DEV_ID);
#endif
	if(thread_arg->instance->data != NULL) free(thread_arg->instance->data);
	if(thread_arg->instance != NULL) free(thread_arg->instance);
	free(thread_arg);
	if(ctx->sockfd != NULL) free(ctx->sockfd);
	if(ctx->thread != NULL) free(ctx->thread);
	free(ctx);
	//os_resume(os_find_task_by_name("app"));
	pthread_exit(NULL);
    return 0;	//args.return_code;
}

void * va_ssl_listen_task(void * vctx)
{
	SOCKET clientfd;
	va_net_context defctx;
	vm_function * func = NULL;
	net_instance * instance;
	va_net_thread_arg * accept_arg;
	va_net_thread_arg * thread_arg = (va_net_thread_arg *)vctx;
	vm_instance * vm_inst = (vm_instance *)thread_arg->instance;
	va_net_context * ctx = (va_net_context *)thread_arg->netctx;
	SOCKET SocketFD = (SOCKET)thread_arg->sockfd;
	//while (!shutDown) {

	if (net_listen(SocketFD, 10) != -1) {
		for(;;) {
			//start executing
			//if(vm_is_running()) return;		//cancel operation if vm currently running
			
#ifndef WOLFSSL_DTLS
			clientfd = net_accept(SocketFD, NULL, NULL);
			if (0 > clientfd) {
				perror("accept failed");
				//net_close(SocketFD);
				break;
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
			accept_arg = (va_net_thread_arg *)malloc(sizeof(va_net_thread_arg));
			if(accept_arg != NULL) {
				accept_arg->instance = (vm_instance *)malloc(sizeof(vm_instance));
				memcpy(accept_arg->instance, thread_arg->instance, sizeof(vm_instance));
				accept_arg->instance->data = (net_instance *)malloc(sizeof(net_instance));
				memcpy(accept_arg->instance->data, thread_arg->instance->data, sizeof(net_instance));
				accept_arg->netctx = (va_net_context *)malloc(sizeof(va_net_context));
				memcpy(accept_arg->netctx, thread_arg->netctx, sizeof(va_net_context));
				((va_default_context *)accept_arg->netctx)->ctx = (void *)clientfd;
				accept_arg->sockfd = clientfd;
				accept_arg->netctx->thread = (pthread_t *)malloc(sizeof(pthread_t));
				accept_arg->netctx->sockfd = (SOCKET *)malloc(sizeof(SOCKET));
				((SOCKET *)accept_arg->netctx->sockfd)[0] = clientfd;
				pthread_create((pthread_t *)accept_arg->netctx->thread, NULL, va_ssl_accept_task, accept_arg);
				//memcpy((pthread_t *)thread_arg->netctx->thread, &ptid, sizeof(pthread_t)); 
			}
			
		}
	}
	net_close(SocketFD);
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
    //wolfSSL_Cleanup();

#ifdef HAVE_CAVIUM
    CspShutdown(CAVIUM_DEV_ID);
#endif
	if(thread_arg->instance->data != NULL) free(thread_arg->instance->data);
	if(thread_arg->instance != NULL) free(thread_arg->instance);
	free(thread_arg);
	if(ctx->sockfd != NULL) free(ctx->sockfd);
	if(ctx->thread != NULL) free(ctx->thread);
	free(ctx);
	//os_resume(os_find_task_by_name("app"));
	pthread_exit(NULL);
    return 0;	//args.return_code;
}


static void va_ssl_flush(VM_DEF_ARG) {			//param1 = context
	//OS_DEBUG_ENTRY(va_net_flush);
	//net_context_p ctx;
	//if(g_pfrmctx == NULL) goto exit_net_close;
	//ctx = g_pfrmctx->netctx;
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	if(vctx->len == sizeof(va_net_context)) {		//return;
			
	}
	exit_net_close:
	//OS_DEBUG_EXIT();
	return;
}

static void va_ssl_close(VM_DEF_ARG) {			//param1 = context
	//OS_DEBUG_ENTRY(va_net_close);
	//net_context_p ctx;
	//if(g_pfrmctx == NULL) goto exit_net_close;
	//ctx = g_pfrmctx->netctx;
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	if(vctx->len == sizeof(va_net_context)) {		//return;
		//TODO:
		if((WOLFSSL_CTX *)((va_default_context *)vctx->bytes)->ctx != NULL) {
			wolfSSL_CTX_free((WOLFSSL_CTX *)((va_default_context *)vctx->bytes)->ctx);
			((va_default_context *)vctx->bytes)->ctx = NULL;
		}
		if(((va_net_context *)vctx->bytes)->sockfd != NULL) {
			net_close(((SOCKET*)((va_net_context *)vctx->bytes)->sockfd)[0]);
			net_shutdown(((SOCKET*)((va_net_context *)vctx->bytes)->sockfd)[0], SHUT_ALL);
			free(((va_net_context *)vctx->bytes)->sockfd);
			((va_net_context *)vctx->bytes)->sockfd = NULL;
		}
		if(((va_net_context *)vctx->bytes)->thread != NULL) {
			pthread_kill(((pthread_t *)((va_net_context *)vctx->bytes)->thread)[0], 0);
			free(((va_net_context *)vctx->bytes)->thread);
			((va_net_context *)vctx->bytes)->thread = NULL;
		}
	}
	exit_net_close:
	//OS_DEBUG_EXIT();
	return;
}

static void va_ssl_recv(VM_DEF_ARG) {			//param1 = context, param2=timeout, return response
	//OS_DEBUG_ENTRY(va_net_recv);
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	uint16 timeout = 5000;
	uint16 len = 0;
	char buffer[VA_OBJECT_MAX_SIZE];
	if(vm_get_argument_count(VM_ARG) > 1) timeout = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	if(vctx->len == sizeof(va_net_context)) {
		len = wolfSSL_recv((WOLFSSL* )((va_default_context *)vctx->bytes)->ctx, buffer, sizeof(buffer), 0);
		if(len != 0) {
			vm_set_retval(vm_create_object(VM_ARG, len, (uchar *)buffer));		//set response as result
		}
	}
	//OS_DEBUG_EXIT();
	return;
}

static void va_ssl_send(VM_DEF_ARG) {			//param1 = context, param2=payload
	//OS_DEBUG_ENTRY(va_net_send);
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	vm_object * payload =  vm_get_argument(VM_ARG, 1);
	uint16 len = 0;
	if(vctx->len == sizeof(va_net_context)) {		//return;
		wolfSSL_send((WOLFSSL* )((va_default_context *)vctx->bytes)->ctx, (const char *)payload->bytes, payload->len, 0);
	}
	//OS_DEBUG_EXIT();
}
#endif

#ifdef USE_OPENSSL
static unsigned int va_ssl_psk_callback(SSL* wctx, const char* buf,
	unsigned char* buf2, unsigned int sz) {
	printf("PSK callback\n");
	return 0;
}

static int va_ssl_send_s(SSL * wctx, char * buf, int sz, void * ictx) {
	int err = net_send((SOCKET)ictx, buf, sz, 0);
	printf("send : %d\n", err);
	return err;
}

static int va_ssl_recv_s(SSL * wctx, char * buf, int sz, void * ictx) {

	int err = net_recv((SOCKET)ictx, buf, sz, 0);
	//printf("%d\n",err);
	if (err == 0) return SSL_ERROR_WANT_ACCEPT;
	return err;
}

void * va_ssl_accept_task(void * vctx) {
	SSL* ssl = 0;
	va_net_context defctx;
	vm_function * func = NULL;
	net_instance * instance;
	vm_instance vcit;
	va_net_thread_arg * thread_arg = (va_net_thread_arg *)vctx;
	vm_instance * vm_inst = (vm_instance *)thread_arg->instance;
	va_net_context * ctx = (va_net_context *)thread_arg->netctx;
	SOCKET clientfd = (SOCKET)thread_arg->sockfd;
	if (ctx == NULL) return NULL;
	func = &ctx->callback;
	if (func == NULL) return NULL;
	instance = (net_instance *)vm_inst->data;
	memcpy(&defctx, ctx, sizeof(va_net_context));

	vm_init(&vcit, vm_inst->info, 2048 * 1024);		//set current PC offset to -1
	vcit.base_address = vm_inst->base_address;
	vcit.stream = vm_stream_open(STREAM_TYPE_MEMORY);
	vcit.data = instance;
	//if (CYASSL_SOCKET_IS_INVALID(clientfd)) err_sys("tcp accept failed");
	ssl =SSL_new((SSL_CTX *)ctx);
	if (ssl == NULL) err_sys("SSL_new failed");
	printf("ssl accept client\n");
	SSL_set_fd(ssl, clientfd);
	//custom context
	//wolfSSL_SetIOReadCtx(ssl, (void *)clientfd);
	//wolfSSL_SetIOWriteCtx(ssl, (void *)clientfd);
#ifdef WOLFSSL_DTLS
	SSL_dtls_set_peer(ssl, &client, client_len);
#endif
#if !defined(NO_FILESYSTEM) && !defined(NO_DH) && !defined(NO_ASN)
	//SSL_SetTmpDH_file(ssl, "./dh.temp", SSL_FILETYPE_PEM);
#elif !defined(NO_DH)
	//	SetDH(ssl);  /* will repick suites with DHE, higher than PSK */
#endif
	if (!SSL_accept(ssl)) {
		//if(vm_init_exec(&vcit, name, reconstructed_method) == 0) {
		if (func != NULL) {
			vm_exec_function(&vcit, (vm_function *)func);
			defctx.base.ctx = ssl;
			if (func->arg_count > 0) vm_push_argument(&vcit, sizeof(va_net_context), (uint8 *)&defctx);
			vm_decode(&vcit, 0, 0);
			vm_close(&vcit);
		}
		//}
		instance->response_size = vm_stream_get_count(vcit.stream);
		instance->response_payload = malloc(instance->response_size);
		vm_stream_get_bytes(vcit.stream, (uint8 *)instance->response_payload);
		//release stream
		vm_stream_close(vcit.stream);

		SSL_shutdown(ssl);
	}
	SSL_free(ssl);
	net_close(clientfd);
#ifdef WOLFSSL_DTLS
	tcp_listen(SocketFD, &port, useAnyAddr, doDTLS);
	SignalReady(args, port);
#endif

	//net_close(SocketFD);
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
	//wolfSSL_Cleanup();

#ifdef HAVE_CAVIUM
	CspShutdown(CAVIUM_DEV_ID);
#endif
	if (thread_arg->instance->data != NULL) free(thread_arg->instance->data);
	if (thread_arg->instance != NULL) free(thread_arg->instance);
	free(thread_arg);
	if (ctx->sockfd != NULL) free(ctx->sockfd);
	if (ctx->thread != NULL) free(ctx->thread);
	free(ctx);
	//os_resume(os_find_task_by_name("app"));
	pthread_exit(NULL);
	return 0;	//args.return_code;
}

void * va_ssl_listen_task(void * vctx)
{
	SOCKET clientfd;
	va_net_context defctx;
	vm_function * func = NULL;
	net_instance * instance;
	va_net_thread_arg * accept_arg;
	va_net_thread_arg * thread_arg = (va_net_thread_arg *)vctx;
	vm_instance * vm_inst = (vm_instance *)thread_arg->instance;
	va_net_context * ctx = (va_net_context *)thread_arg->netctx;
	SOCKET SocketFD = (SOCKET)thread_arg->sockfd;
	//while (!shutDown) {

	if (net_listen(SocketFD, 10) != -1) {
		for (;;) {
			//start executing
			//if(vm_is_running()) return;		//cancel operation if vm currently running

#ifndef WOLFSSL_DTLS
			clientfd = net_accept(SocketFD, NULL, NULL);
			if (0 > clientfd) {
				perror("accept failed");
				//net_close(SocketFD);
				break;
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
			accept_arg = (va_net_thread_arg *)malloc(sizeof(va_net_thread_arg));
			if (accept_arg != NULL) {
				accept_arg->instance = (vm_instance *)malloc(sizeof(vm_instance));
				memcpy(accept_arg->instance, thread_arg->instance, sizeof(vm_instance));
				accept_arg->instance->data = (net_instance *)malloc(sizeof(net_instance));
				memcpy(accept_arg->instance->data, thread_arg->instance->data, sizeof(net_instance));
				accept_arg->netctx = (va_net_context *)malloc(sizeof(va_net_context));
				memcpy(accept_arg->netctx, thread_arg->netctx, sizeof(va_net_context));
				((va_default_context *)accept_arg->netctx)->ctx = (void *)clientfd;
				accept_arg->sockfd = clientfd;
				accept_arg->netctx->thread = (pthread_t *)malloc(sizeof(pthread_t));
				accept_arg->netctx->sockfd = (SOCKET *)malloc(sizeof(SOCKET));
				((SOCKET *)accept_arg->netctx->sockfd)[0] = clientfd;
				pthread_create((pthread_t *)accept_arg->netctx->thread, NULL, va_ssl_accept_task, accept_arg);
				//memcpy((pthread_t *)thread_arg->netctx->thread, &ptid, sizeof(pthread_t)); 
			}

		}
	}
	net_close(SocketFD);
	//((func_args*)args)->return_code = 0;

	if (thread_arg->instance->data != NULL) free(thread_arg->instance->data);
	if (thread_arg->instance != NULL) free(thread_arg->instance);
	free(thread_arg);
	if (ctx->sockfd != NULL) free(ctx->sockfd);
	if (ctx->thread != NULL) free(ctx->thread);
	free(ctx);
	//os_resume(os_find_task_by_name("app"));
	pthread_exit(NULL);
	return 0;	//args.return_code;
}


static void va_ssl_flush(VM_DEF_ARG) {			//param1 = context
												//OS_DEBUG_ENTRY(va_net_flush);
												//net_context_p ctx;
												//if(g_pfrmctx == NULL) goto exit_net_close;
												//ctx = g_pfrmctx->netctx;
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	if (vctx->len == sizeof(va_net_context)) {		//return;

	}
exit_net_close:
	//OS_DEBUG_EXIT();
	return;
}

static void va_ssl_close(VM_DEF_ARG) {			//param1 = context
												//OS_DEBUG_ENTRY(va_net_close);
												//net_context_p ctx;
												//if(g_pfrmctx == NULL) goto exit_net_close;
												//ctx = g_pfrmctx->netctx;
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	if (vctx->len == sizeof(va_net_context)) {		//return;
													//TODO:
		if ((SSL_CTX *)((va_default_context *)vctx->bytes)->ctx != NULL) {
			SSL_CTX_free((SSL_CTX *)((va_default_context *)vctx->bytes)->ctx);
			((va_default_context *)vctx->bytes)->ctx = NULL;
		}
		if (((va_net_context *)vctx->bytes)->sockfd != NULL) {
			net_close(((SOCKET*)((va_net_context *)vctx->bytes)->sockfd)[0]);
			net_shutdown(((SOCKET*)((va_net_context *)vctx->bytes)->sockfd)[0], SHUT_ALL);
			free(((va_net_context *)vctx->bytes)->sockfd);
			((va_net_context *)vctx->bytes)->sockfd = NULL;
		}
		if (((va_net_context *)vctx->bytes)->thread != NULL) {
			pthread_kill(((pthread_t *)((va_net_context *)vctx->bytes)->thread)[0], 0);
			free(((va_net_context *)vctx->bytes)->thread);
			((va_net_context *)vctx->bytes)->thread = NULL;
		}
	}
exit_net_close:
	//OS_DEBUG_EXIT();
	return;
}

static void va_ssl_recv(VM_DEF_ARG) {			//param1 = context, param2=timeout, return response
												//OS_DEBUG_ENTRY(va_net_recv);
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	uint16 timeout = 5000;
	uint16 len = 0;
	char buffer[VA_OBJECT_MAX_SIZE];
	if (vm_get_argument_count(VM_ARG) > 1) timeout = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	if (vctx->len == sizeof(va_net_context)) {
		len = SSL_read((SSL*)((va_default_context *)vctx->bytes)->ctx, buffer, sizeof(buffer));
		if (len != 0) {
			vm_set_retval(vm_create_object(VM_ARG, len, (uchar *)buffer));		//set response as result
		}
	}
	//OS_DEBUG_EXIT();
	return;
}

static void va_ssl_send(VM_DEF_ARG) {			//param1 = context, param2=payload
												//OS_DEBUG_ENTRY(va_net_send);
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	vm_object * payload = vm_get_argument(VM_ARG, 1);
	uint16 len = 0;
	if (vctx->len == sizeof(va_net_context)) {		//return;
		SSL_write((SSL*)((va_default_context *)vctx->bytes)->ctx, (const char *)payload->bytes, payload->len);
	}
	//OS_DEBUG_EXIT();
}
#endif

//network APIs
static void va_net_flush(VM_DEF_ARG) {			//param1 = context
	//OS_DEBUG_ENTRY(va_net_flush);
	//net_context_p ctx;
	//if(g_pfrmctx == NULL) goto exit_net_close;
	//ctx = g_pfrmctx->netctx;
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	if(vctx->len == sizeof(va_net_context)) {		//return;
			
	}
	exit_net_close:
	//OS_DEBUG_EXIT();
	return;
}

static void va_net_close(VM_DEF_ARG) {			//param1 = context
	//OS_DEBUG_ENTRY(va_net_close);
	//net_context_p ctx;
	//if(g_pfrmctx == NULL) goto exit_net_close;
	//ctx = g_pfrmctx->netctx;
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	if(vctx->len == sizeof(va_net_context)) {		//return;
		if(((va_net_context *)vctx->bytes)->sockfd != NULL) {
			net_close(((SOCKET*)((va_net_context *)vctx->bytes)->sockfd)[0]);
			net_shutdown(((SOCKET*)((va_net_context *)vctx->bytes)->sockfd)[0], SHUT_ALL);
			free(((va_net_context *)vctx->bytes)->sockfd);
			((va_net_context *)vctx->bytes)->sockfd = NULL;
		}
		if(((va_net_context *)vctx->bytes)->thread != NULL) {
			pthread_kill(((pthread_t *)((va_net_context *)vctx->bytes)->thread)[0], 0);
			free(((va_net_context *)vctx->bytes)->thread);
			((va_net_context *)vctx->bytes)->thread = NULL;
		}
		//if(ctx != NULL) free(ctx);
	}
	exit_net_close:
	//OS_DEBUG_EXIT();
	return;
}

static void va_net_recv(VM_DEF_ARG) {			//param1 = context, param2=timeout, return response
	//OS_DEBUG_ENTRY(va_net_recv);
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	uint16 timeout = 5000;
	int len = 0;
	char buffer[VA_OBJECT_MAX_SIZE];
	if(vm_get_argument_count(VM_ARG) > 1) timeout = va_o2f(VM_ARG, vm_get_argument(VM_ARG, 1));
	if(vctx->len == sizeof(va_net_context)) {
		//net_accept((SOCKET)((va_default_context *)vctx->bytes)->ctx, 0, 0);
		//printf("socket : %d\n", (SOCKET)((va_default_context *)vctx->bytes)->ctx);
		len = net_recv((SOCKET)((va_default_context *)vctx->bytes)->ctx, buffer, sizeof(buffer), 0);
		if(len == -1) return;
		if(len != 0) {
			vm_set_retval(vm_create_object(VM_ARG, len, (uchar *)buffer));		//set response as result
		}
	}
	//OS_DEBUG_EXIT();
	return;
}

static void va_net_send(VM_DEF_ARG) {			//param1 = context, param2=payload
	//OS_DEBUG_ENTRY(va_net_send);
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	vm_object * payload =  vm_get_argument(VM_ARG, 1);
	uint16 len = 0;
	if(vctx->len == sizeof(va_net_context)) {		//return;
		net_send((SOCKET)((va_default_context *)vctx->bytes)->ctx, (const char *)payload->bytes, payload->len, 0);
	}
	//OS_DEBUG_EXIT();
}

void * va_net_accept_task(void * vctx) {
	va_net_context defctx;
	vm_function * func = NULL;
	vm_instance vcit;
	net_instance * instance;
	va_net_thread_arg * thread_arg = (va_net_thread_arg *)vctx;
	vm_instance * vm_inst = (vm_instance *)thread_arg->instance;
	va_net_context * ctx = (va_net_context *)thread_arg->netctx;
	SOCKET ConnectFD = (SOCKET)((va_default_context *)ctx)->ctx;
	if(ctx == NULL) return NULL;
	func = &ctx->callback;
	if(func == NULL) return NULL;
	instance = (net_instance *)vm_inst->data;
	memcpy(&defctx, ctx, sizeof(va_net_context));

	vm_init(&vcit, vm_inst->info, 2048 * 1024);		//set current PC offset to -1
	vcit.base_address = vm_inst->base_address;
	vcit.stream = vm_stream_open(STREAM_TYPE_MEMORY);
	vcit.data = instance;
	//if(vm_init_exec(&vcit, name, reconstructed_method) == 0) {
	if(func != NULL) {
		//printf("execute callback : %s\n", callback->name);
		vm_exec_function(&vcit, (vm_function *)func);
		defctx.base.ctx = (void *)ConnectFD;		
		if(func->arg_count > 0) vm_push_argument(&vcit, sizeof(va_net_context), (uint8 *)&defctx);
		vm_decode(&vcit, 0, 0);
		vm_close(&vcit);
	}
	//}
	instance->response_size = vm_stream_get_count(vcit.stream);
	instance->response_payload = malloc(instance->response_size);
	vm_stream_get_bytes(vcit.stream, (uint8 *)instance->response_payload);
	//release stream
	if(vcit.stream != NULL) {
		vm_stream_close(vcit.stream);
		vcit.stream = NULL;
	}

	net_close(ConnectFD);
	if(thread_arg->instance->data != NULL) free(thread_arg->instance->data);
	if(thread_arg->instance != NULL) free(thread_arg->instance);
	free(thread_arg);
	if(ctx->sockfd != NULL) free(ctx->sockfd);
	if(ctx->thread != NULL) free(ctx->thread);
	if(ctx != NULL) free(ctx);
	//os_resume(os_find_task_by_name("app"));
	pthread_exit(NULL);
	return NULL;
}

void * va_net_listen_task(void * vctx) {
    struct sockaddr_in sa;
	va_net_thread_arg * accept_arg;
	va_net_thread_arg * thread_arg = (va_net_thread_arg *)vctx;
	vm_instance * vm_inst = (vm_instance *)thread_arg->instance;
	va_net_context * ctx = (va_net_context *)thread_arg->netctx;
	SOCKET SocketFD = (SOCKET)((va_default_context *)ctx)->ctx;
	//memcpy(&vcit, vm_inst, sizeof(vm_instance));
	
    if (net_listen(SocketFD, 10) != -1) {
		for(;;) {
			//start executing
			//if(vm_is_running()) return;		//cancel operation if vm currently running
			int ConnectFD = net_accept(SocketFD, NULL, NULL);
			
			if (0 > ConnectFD) {
				//net_close(SocketFD);
				//break;
				break;
			}
			printf("connect descriptor : %d\n", ConnectFD);

			accept_arg = (va_net_thread_arg *)malloc(sizeof(va_net_thread_arg));
			if(accept_arg != NULL) {
				accept_arg->instance = (vm_instance *)malloc(sizeof(vm_instance));
				memcpy(accept_arg->instance, thread_arg->instance, sizeof(vm_instance));
				accept_arg->instance->data = (net_instance *)malloc(sizeof(net_instance));
				memcpy(accept_arg->instance->data, thread_arg->instance->data, sizeof(net_instance));
				accept_arg->netctx = (va_net_context *)malloc(sizeof(va_net_context));
				memcpy(accept_arg->netctx, thread_arg->netctx, sizeof(va_net_context));
				((va_default_context *)accept_arg->netctx)->ctx = (void *)ConnectFD;
				accept_arg->sockfd = ConnectFD;
				accept_arg->netctx->sockfd = (SOCKET *)malloc(sizeof(SOCKET));
				((SOCKET *)accept_arg->netctx->sockfd)[0] = ConnectFD;
				accept_arg->netctx->thread = (pthread_t *)malloc(sizeof(pthread_t));
				pthread_create((pthread_t *)accept_arg->netctx->thread, NULL, va_net_accept_task, accept_arg);
				//memcpy((pthread_t *)thread_arg->netctx->thread, &ptid, sizeof(pthread_t)); 
			}
			//while(1);
		}
	}
	net_close(SocketFD);
	if(thread_arg->instance->data != NULL) free(thread_arg->instance->data);
	if(thread_arg->instance != NULL) free(thread_arg->instance);
	free(thread_arg);
	if(ctx->sockfd != NULL) free(ctx->sockfd);
	if(ctx->thread != NULL) free(ctx->thread);
	if(ctx != NULL) free(ctx);
	//os_resume(os_find_task_by_name("app"));
	pthread_exit(NULL);
	return NULL;
}

#define VA_SSL_PROTOCOL		128
#define VA_TLS_PROTOCOL		129
#define VA_TCP_PROTOCOL		IPPROTO_TCP
#define VA_UDP_PROTOCOL		IPPROTO_UDP

#if defined(USE_WOLFSSL)
void va_net_open(VM_DEF_ARG) {			//param1 = type, param2 = address, param3 = port, param4=callback, return context
	//OS_DEBUG_ENTRY(va_net_open);
	//net_context_p ctx;
    struct sockaddr_in sa;
    pthread_t thread;
	uint16 protocol;
    WOLFSSL_METHOD* method = 0;
    WOLFSSL_CTX*    wctx    = 0;
	int SocketFD ;

    int    doDTLS = 0;
    int    doPSK = 0;
    int    outCreated = 0;
    //int    shutDown = 0;
    int    useAnyAddr = 0;
    int     echoSz = 0;
    int     firstRead = 1;
    int     gotFirstG = 0;
	
	vm_object * vtype = vm_get_argument(VM_ARG, 0);
	vm_object * vaddr = vm_get_argument(VM_ARG, 1);
	vm_object * vport = vm_get_argument(VM_ARG, 2);
	vm_object * vfunc = vm_get_argument(VM_ARG, 3);
	vm_object * vcert = vm_get_argument(VM_ARG, 4);
	vm_object * vkey = vm_get_argument(VM_ARG, 5);
	uint8 url[512];
	uint8 tag;
	uint16 size;
	uint16 mode = 0;
	vm_function * func = NULL;
	va_net_context defctx;
	va_net_thread_arg * thread_arg;
	pthread_t ptid;
	uint16 port = va_o2f(VM_ARG, vport);
	memset(&defctx, 0, sizeof(va_net_context));
	if(vm_get_argument_count(VM_ARG) < 3) goto exit_net_open;
	//process listen callback first
	if(vm_get_argument_count(VM_ARG) >= 4 && vfunc->len != 0) {
		mode = 1;			//switch to listen mode
		func = (vm_function *)malloc(sizeof(vm_function));
		memcpy(func, vfunc->bytes, sizeof(vm_function));
		//func->base.vars = NULL;		//clear root vars, by the time this callback called, script already terminated
		if(func->arg_count > 2) {	//check for function argument, must be one parameter
			vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
			goto exit_net_open;
		}
	}
	memset(url, 0, sizeof(url));
	memcpy(url, vaddr->bytes, vaddr->len);
	if(strncmp((const char *)vtype->bytes, "TCP", 3) == 0) {
		protocol = VA_TCP_PROTOCOL;
	} else if(strncmp((const char *)vtype->bytes, "UDP", 3) == 0) {
		protocol = VA_UDP_PROTOCOL;
	} else if(strncmp((const char *)vtype->bytes, "SSL", 3) == 0) {
		protocol = VA_SSL_PROTOCOL;
	} else if(strncmp((const char *)vtype->bytes, "TLS", 3) == 0) {
		protocol = VA_TLS_PROTOCOL;
	} else {
		vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_ARGUMENT);
		goto exit_net_open;
	}
	switch(protocol) {
		case VA_TCP_PROTOCOL:
		case VA_UDP_PROTOCOL:
			SocketFD = net_socket(AF_INET, SOCK_STREAM, protocol);
			memset(&sa, 0, sizeof(sa));
			if(mode == 1) {		//listen
				sa.sin_family = AF_INET;
				sa.sin_port = htons(port);
				sa.sin_addr.s_addr = htonl(INADDR_ANY);
			  
				if (net_bind(SocketFD,(struct sockaddr *)&sa, sizeof sa) == -1) {
				  perror("sock bind failed");
				  net_close(SocketFD);
				}		
				if(SocketFD != 0) {			//operation clear
					((va_default_context *)&defctx)->ctx = (void *)SocketFD;
					((va_default_context *)&defctx)->close = va_net_close;
					((va_default_context *)&defctx)->read = va_net_recv;
					((va_default_context *)&defctx)->write = va_net_send;
					((va_default_context *)&defctx)->offset =  0;
					((va_default_context *)&defctx)->seek = NULL;
					//defctx.callback = func;
					memcpy(&defctx.callback, func, sizeof(vm_function));
					//defctx.sockfd = SocketFD;
					defctx.sockfd = malloc(sizeof(SOCKET));
					defctx.thread = malloc(sizeof(pthread_t));
					((SOCKET *)defctx.sockfd)[0] = SocketFD;
					vm_set_retval( vm_create_object(VM_ARG, sizeof(va_net_context), (uchar *)&defctx) );
					thread_arg = (va_net_thread_arg *)malloc(sizeof(va_net_thread_arg));
					if(thread_arg != NULL) {
						thread_arg->instance = (vm_instance *)malloc(sizeof(vm_instance));
						memcpy(thread_arg->instance, VM_ARG, sizeof(vm_instance));
						thread_arg->netctx = (va_net_context *)malloc(sizeof(va_net_context));
						memcpy(thread_arg->netctx, &defctx, sizeof(va_net_context));
						thread_arg->netctx->thread = (pthread_t *)malloc(sizeof(pthread_t));
						thread_arg->netctx->sockfd = (pthread_t *)malloc(sizeof(SOCKET));
						pthread_create((pthread_t *)thread_arg->netctx->thread, NULL, va_net_listen_task, thread_arg);
						//memcpy((pthread_t *)thread_arg->netctx->thread, &ptid, sizeof(pthread_t)); 
					}
				}
			} else {
				//after socket created start connecting
				net_connect(SocketFD, (const struct sockaddr *)vaddr->bytes, vaddr->len);
			
				if(SocketFD != 0) {			//operation clear
					((va_default_context *)&defctx)->ctx = (void *)SocketFD;
					((va_default_context *)&defctx)->close = va_net_close;
					((va_default_context *)&defctx)->read = va_net_recv;
					((va_default_context *)&defctx)->write = va_net_send;
					((va_default_context *)&defctx)->offset =  0;
					((va_default_context *)&defctx)->seek = NULL;
					defctx.callback.arg_count = 0;
					defctx.callback.offset = -1;
					defctx.sockfd = malloc(sizeof(SOCKET));
					defctx.thread = NULL;
					((SOCKET *)defctx.sockfd)[0] = SocketFD;
					vm_set_retval(vm_create_object(VM_ARG, sizeof(va_net_context), (uchar *)&defctx));
				}
			}
			break;
		case VA_SSL_PROTOCOL:
		case VA_TLS_PROTOCOL:
#ifdef HAVE_CAVIUM
			int ret = OpenNitroxDevice(CAVIUM_DIRECT, CAVIUM_DEV_ID);
			if (ret != 0)
				err_sys("Cavium OpenNitroxDevice failed");
#endif /* HAVE_CAVIUM */

			//wolfSSL_Init();
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

#if defined(WOLFSSL_DTLS)
			method  = wolfDTLSv1_2_server_method();
#elif  !defined(NO_TLS)
			if(protocol == VA_TLS_PROTOCOL) {
				method = wolfTLSv1_2_server_method();
			}
#elif defined(WOLFSSL_ALLOW_SSLV3)
			if(protocol == VA_SSL_PROTOCOL) {
				method = wolfSSLv3_server_method();
			}
#else
			#error "no valid server method built in"
#endif

			wctx    = wolfSSL_CTX_new(method);
			wolfSSL_SetIORecv(wctx, va_ssl_recv_s);
			wolfSSL_SetIOSend(wctx, va_ssl_send_s);
		
	/* CyaSSL_CTX_set_session_cache_mode(wctx, SSL_SESS_CACHE_OFF); */

#if defined(OPENSSL_EXTRA) || defined(HAVE_WEBSERVER)
			wolfSSL_CTX_set_default_passwd_cb(wctx, PasswordCallBack);
#endif

#if defined(HAVE_SESSION_TICKET) && defined(HAVE_CHACHA) && \
                                defined(HAVE_POLY1305)
			if (TicketInit() != 0)
				err_sys("unable to setup Session Ticket Key context");
			wolfSSL_CTX_set_TicketEncCb(wctx, myTicketEncCb);
#endif

#ifndef NO_FILESYSTEM
			if (doPSK == 0) {
#ifdef HAVE_NTRU
				/* ntru */
				if (wolfSSL_CTX_use_certificate_file(wctx, ntruCert, SSL_FILETYPE_PEM)
						!= SSL_SUCCESS)
					err_sys("can't load ntru cert file, "
							"Please run from wolfSSL home dir");

				if (wolfSSL_CTX_use_NTRUPrivateKey_file(wctx, ntruKey)
						!= SSL_SUCCESS)
					err_sys("can't load ntru key file, "
							"Please run from wolfSSL home dir");
#elif defined(HAVE_ECC) && !defined(WOLFSSL_SNIFFER)
				/* ecc */
				if(vcert != NULL && vcert->len != 0) {
					if (wolfSSL_CTX_use_certificate_buffer(wctx, vcert->bytes, vcert->len, SSL_FILETYPE_PEM)
							!= SSL_SUCCESS)
						err_sys("can't load server cert file, "
								"Please run from wolfSSL home dir");
				} else {
					//use default certificate
				}

				if(vkey != NULL && vkey->len != 0) {
					if (wolfSSL_CTX_use_PrivateKey_buffer(wctx, vkey->bytes, vkey->len, SSL_FILETYPE_PEM)
							!= SSL_SUCCESS)
						err_sys("can't load server key file, "
								"Please run from wolfSSL home dir");
				} else {

				}
#elif defined(NO_CERTS)
		/* do nothing, just don't load cert files */
#else
				/* normal */
				if (wolfSSL_CTX_use_certificate_file(wctx, svrCert, SSL_FILETYPE_PEM)
						!= SSL_SUCCESS)
					err_sys("can't load server cert file, "
							"Please run from wolfSSL home dir");

				if (wolfSSL_CTX_use_PrivateKey_file(wctx, svrKey, SSL_FILETYPE_PEM)
						!= SSL_SUCCESS)
					err_sys("can't load server key file, "
							"Please run from wolfSSL home dir");
#endif
			} /* doPSK */
#elif !defined(NO_CERTS)
			if (!doPSK) {
				load_buffer(wctx, svrCert, WOLFSSL_CERT);
				load_buffer(wctx, svrKey,  WOLFSSL_KEY);
			}
#endif

#if defined(WOLFSSL_SNIFFER)
			/* don't use EDH, can't sniff tmp keys */
			wolfSSL_CTX_set_cipher_list(wctx, "AES256-SHA");
#endif

			if (doPSK) {
#ifndef NO_PSK
				const char *defaultCipherList;

				wolfSSL_CTX_set_psk_server_callback(wctx, va_ssl_psk_callback);
				wolfSSL_CTX_use_psk_identity_hint(wctx, "wolfssl server");
				#ifdef HAVE_NULL_CIPHER
					defaultCipherList = "PSK-NULL-SHA256";
				#elif defined(HAVE_AESGCM) && !defined(NO_DH)
					defaultCipherList = "DHE-PSK-AES128-GCM-SHA256";
				#else
					defaultCipherList = "PSK-AES128-CBC-SHA256";
				#endif
				if (wolfSSL_CTX_set_cipher_list(wctx, defaultCipherList) != SSL_SUCCESS)
					err_sys("server can't set cipher list 2");
#endif
			}
			SocketFD = net_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			memset(&sa, 0, sizeof(sa));
			if (SocketFD == -1) {
			  perror("https cannot create socket");
			  exit(EXIT_FAILURE);
			}
			if(mode == 1) {
				sa.sin_family = AF_INET;
				sa.sin_port = htons(port);
				sa.sin_addr.s_addr = htonl(INADDR_ANY);
				if (net_bind(SocketFD,(struct sockaddr *)&sa, sizeof sa) == -1) {
				  perror("http bind failed");
				  net_close(SocketFD);
				}		
				if(SocketFD != 0) {			//operation clear
					((va_default_context *)&defctx)->ctx = (void *)wctx;
					((va_default_context *)&defctx)->close = va_net_close;
					((va_default_context *)&defctx)->read = va_net_recv;
					((va_default_context *)&defctx)->write = va_net_send;
					((va_default_context *)&defctx)->offset =  0;
					((va_default_context *)&defctx)->seek = NULL;
					//defctx.callback = func;
					memcpy(&defctx.callback, func, sizeof(vm_function));
					defctx.sockfd = malloc(sizeof(SOCKET));
					defctx.thread = malloc(sizeof(pthread_t));
					((SOCKET *)defctx.sockfd)[0] = SocketFD;
					vm_set_retval( vm_create_object(VM_ARG, sizeof(va_net_context), (uchar *)&defctx) );
					thread_arg = (va_net_thread_arg *)malloc(sizeof(va_net_thread_arg));
					if(thread_arg != NULL) {
						thread_arg->instance = (vm_instance *)malloc(sizeof(vm_instance *));
						thread_arg->netctx = (va_net_context *)malloc(sizeof(va_net_context));
						thread_arg->sockfd = SocketFD;
						memcpy(thread_arg->instance, VM_ARG, sizeof(vm_instance));
						memcpy(thread_arg->netctx, &defctx, sizeof(va_net_context));
						pthread_create((pthread_t *)thread_arg->netctx->thread, NULL, va_ssl_listen_task, thread_arg);
					}
				}
			} else {
				//after socket created start connecting
				net_connect(SocketFD, (const struct sockaddr *)vaddr->bytes, vaddr->len);
			
				if(SocketFD != 0) {			//operation clear
					((va_default_context *)&defctx)->ctx = (void *)wctx;
					((va_default_context *)&defctx)->close = va_net_close;
					((va_default_context *)&defctx)->read = va_net_recv;
					((va_default_context *)&defctx)->write = va_net_send;
					((va_default_context *)&defctx)->offset =  0;
					((va_default_context *)&defctx)->seek = NULL;
					defctx.callback.arg_count = 0;
					defctx.callback.offset = -1;
					defctx.sockfd = malloc(sizeof(SOCKET));
					defctx.thread = NULL;
					((SOCKET *)defctx.sockfd)[0] = SocketFD;
					vm_set_retval(vm_create_object(VM_ARG, sizeof(va_net_context), (uchar *)&defctx));
				}
			}
			break;
	}
	exit_net_open:
	//OS_DEBUG_EXIT();
	return;
}
#endif

#if defined(USE_OPENSSL)
void va_net_open(VM_DEF_ARG) {			//param1 = type, param2 = address, param3 = port, param4=callback, return context
										//OS_DEBUG_ENTRY(va_net_open);
										//net_context_p ctx;
	struct sockaddr_in sa;
	pthread_t thread;
	uint16 protocol;
	const SSL_METHOD* method = 0;
	SSL_CTX*    wctx = 0;
	int SocketFD;

	int    doDTLS = 0;
	int    doPSK = 0;
	int    outCreated = 0;
	//int    shutDown = 0;
	int    useAnyAddr = 0;
	int     echoSz = 0;
	int     firstRead = 1;
	int     gotFirstG = 0;

	vm_object * vtype = vm_get_argument(VM_ARG, 0);
	vm_object * vaddr = vm_get_argument(VM_ARG, 1);
	vm_object * vport = vm_get_argument(VM_ARG, 2);
	vm_object * vfunc = vm_get_argument(VM_ARG, 3);
	vm_object * vcert = vm_get_argument(VM_ARG, 4);
	vm_object * vkey = vm_get_argument(VM_ARG, 5);
	uint8 url[512];
	uint8 tag;
	uint16 size;
	uint16 mode = 0;
	vm_function * func = NULL;
	va_net_context defctx;
	va_net_thread_arg * thread_arg;
	pthread_t ptid;
	uint16 port = va_o2f(VM_ARG, vport);
	memset(&defctx, 0, sizeof(va_net_context));
	if (vm_get_argument_count(VM_ARG) < 3) goto exit_net_open;
	//process listen callback first
	if (vm_get_argument_count(VM_ARG) >= 4 && vfunc->len != 0) {
		mode = 1;			//switch to listen mode
		func = (vm_function *)malloc(sizeof(vm_function));
		memcpy(func, vfunc->bytes, sizeof(vm_function));
		//func->base.vars = NULL;		//clear root vars, by the time this callback called, script already terminated
		if (func->arg_count > 2) {	//check for function argument, must be one parameter
			vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
			goto exit_net_open;
		}
	}
	memset(url, 0, sizeof(url));
	memcpy(url, vaddr->bytes, vaddr->len);
	if (strncmp((const char *)vtype->bytes, "TCP", 3) == 0) {
		protocol = VA_TCP_PROTOCOL;
	}
	else if (strncmp((const char *)vtype->bytes, "UDP", 3) == 0) {
		protocol = VA_UDP_PROTOCOL;
	}
	else if (strncmp((const char *)vtype->bytes, "SSL", 3) == 0) {
		protocol = VA_SSL_PROTOCOL;
	}
	else if (strncmp((const char *)vtype->bytes, "TLS", 3) == 0) {
		protocol = VA_TLS_PROTOCOL;
	}
	else {
		vm_invoke_exception(VM_ARG, VX_UNSUPPORTED_ARGUMENT);
		goto exit_net_open;
	}
	switch (protocol) {
	case VA_TCP_PROTOCOL:
	case VA_UDP_PROTOCOL:
		SocketFD = net_socket(AF_INET, SOCK_STREAM, protocol);
		memset(&sa, 0, sizeof(sa));
		if (mode == 1) {		//listen
			sa.sin_family = AF_INET;
			sa.sin_port = htons(port);
			sa.sin_addr.s_addr = htonl(INADDR_ANY);

			if (net_bind(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1) {
				perror("sock bind failed");
				net_close(SocketFD);
			}
			if (SocketFD != 0) {			//operation clear
				((va_default_context *)&defctx)->ctx = (void *)SocketFD;
				((va_default_context *)&defctx)->close = va_net_close;
				((va_default_context *)&defctx)->read = va_net_recv;
				((va_default_context *)&defctx)->write = va_net_send;
				((va_default_context *)&defctx)->offset = 0;
				((va_default_context *)&defctx)->seek = NULL;
				//defctx.callback = func;
				memcpy(&defctx.callback, func, sizeof(vm_function));
				//defctx.sockfd = SocketFD;
				defctx.sockfd = malloc(sizeof(SOCKET));
				defctx.thread = malloc(sizeof(pthread_t));
				((SOCKET *)defctx.sockfd)[0] = SocketFD;
				vm_set_retval(vm_create_object(VM_ARG, sizeof(va_net_context), (uchar *)&defctx));
				thread_arg = (va_net_thread_arg *)malloc(sizeof(va_net_thread_arg));
				if (thread_arg != NULL) {
					thread_arg->instance = (vm_instance *)malloc(sizeof(vm_instance));
					memcpy(thread_arg->instance, VM_ARG, sizeof(vm_instance));
					thread_arg->netctx = (va_net_context *)malloc(sizeof(va_net_context));
					memcpy(thread_arg->netctx, &defctx, sizeof(va_net_context));
					thread_arg->netctx->thread = (pthread_t *)malloc(sizeof(pthread_t));
					thread_arg->netctx->sockfd = (pthread_t *)malloc(sizeof(SOCKET));
					pthread_create((pthread_t *)thread_arg->netctx->thread, NULL, va_net_listen_task, thread_arg);
					//memcpy((pthread_t *)thread_arg->netctx->thread, &ptid, sizeof(pthread_t)); 
				}
			}
		}
		else {
			//after socket created start connecting
			net_connect(SocketFD, (const struct sockaddr *)vaddr->bytes, vaddr->len);

			if (SocketFD != 0) {			//operation clear
				((va_default_context *)&defctx)->ctx = (void *)SocketFD;
				((va_default_context *)&defctx)->close = va_net_close;
				((va_default_context *)&defctx)->read = va_net_recv;
				((va_default_context *)&defctx)->write = va_net_send;
				((va_default_context *)&defctx)->offset = 0;
				((va_default_context *)&defctx)->seek = NULL;
				defctx.callback.arg_count = 0;
				defctx.callback.offset = -1;
				defctx.sockfd = malloc(sizeof(SOCKET));
				defctx.thread = NULL;
				((SOCKET *)defctx.sockfd)[0] = SocketFD;
				vm_set_retval(vm_create_object(VM_ARG, sizeof(va_net_context), (uchar *)&defctx));
			}
		}
		break;
	case VA_SSL_PROTOCOL:
	case VA_TLS_PROTOCOL:

		//wolfSSL_Init();


#if defined(WOLFSSL_DTLS)
		method = wolfDTLSv1_2_server_method();
#elif  !defined(NO_TLS)
		if (protocol == VA_TLS_PROTOCOL) {
			method = TLSv1_2_server_method();
		}
#elif defined(WOLFSSL_ALLOW_SSLV3)
		if (protocol == VA_SSL_PROTOCOL) {
			method = wolfSSLv3_server_method();
		}
#else
#error "no valid server method built in"
#endif

		wctx = SSL_CTX_new(method);
		//wolfSSL_SetIORecv(wctx, va_ssl_recv_s);
		//wolfSSL_SetIOSend(wctx, va_ssl_send_s);

		/* CyaSSL_CTX_set_session_cache_mode(wctx, SSL_SESS_CACHE_OFF); */

#if defined(OPENSSL_EXTRA) || defined(HAVE_WEBSERVER)
		wolfSSL_CTX_set_default_passwd_cb(wctx, PasswordCallBack);
#endif

#if defined(HAVE_SESSION_TICKET) && defined(HAVE_CHACHA) && \
                                defined(HAVE_POLY1305)
		if (TicketInit() != 0)
			err_sys("unable to setup Session Ticket Key context");
		wolfSSL_CTX_set_TicketEncCb(wctx, myTicketEncCb);
#endif

#ifndef NO_FILESYSTEM
		if (doPSK == 0) {
#ifdef HAVE_NTRU
			/* ntru */
			if (wolfSSL_CTX_use_certificate_file(wctx, ntruCert, SSL_FILETYPE_PEM)
				!= SSL_SUCCESS)
				err_sys("can't load ntru cert file, "
					"Please run from wolfSSL home dir");

			if (wolfSSL_CTX_use_NTRUPrivateKey_file(wctx, ntruKey)
				!= SSL_SUCCESS)
				err_sys("can't load ntru key file, "
					"Please run from wolfSSL home dir");
#elif defined(HAVE_ECC) && !defined(WOLFSSL_SNIFFER)
			/* ecc */
#if defined(WIN32) || defined(WIN64)
			if (vcert != NULL && vcert->len != 0) {

				BIO *in;
				size_t bytes_read;
				X509 *x = NULL;
				in = BIO_new(BIO_s_mem());
				if (in == NULL) {
					SSLerr(SSL_F_SSL_USE_CERTIFICATE_FILE, ERR_R_BUF_LIB);
				}

				if (BIO_read_ex(in, vcert->bytes, vcert->len, &bytes_read) <= 0) {
					SSLerr(SSL_F_SSL_USE_CERTIFICATE_FILE, ERR_R_SYS_LIB);
					goto exit_net_open;
				}
				x = PEM_read_bio_X509(in, NULL, NULL, NULL);
				if (!SSL_CTX_use_certificate(wctx, x) )
					err_sys("can't load server cert file, "
						"Please run from OpenSSL home dir");
				BIO_free(in);
			}
			else {
				//use default certificate
			}
			if (vkey != NULL && vkey->len != 0) {
				BIO *in;
				size_t bytes_read;
				RSA *rsa = NULL;
				in = BIO_new(BIO_s_mem());
				if (in == NULL) {
					SSLerr(SSL_F_SSL_USE_CERTIFICATE_FILE, ERR_R_BUF_LIB);
				}

				if (BIO_read_ex(in, vkey->bytes, vkey->len, &bytes_read) <= 0) {
					SSLerr(SSL_F_SSL_USE_CERTIFICATE_FILE, ERR_R_SYS_LIB);
					goto exit_net_open;
				}
				rsa = PEM_read_bio_RSAPrivateKey(in, NULL, NULL, NULL);
				if (!SSL_CTX_use_RSAPrivateKey(wctx, rsa) )
					err_sys("can't load server key file, "
						"Please run from OpenSSL home dir");
				BIO_free(in);
			}
			else {

			}
#endif

#elif defined(NO_CERTS)
			/* do nothing, just don't load cert files */
#else
			/* normal */
			if (wolfSSL_CTX_use_certificate_file(wctx, svrCert, SSL_FILETYPE_PEM)
				!= SSL_SUCCESS)
				err_sys("can't load server cert file, "
					"Please run from wolfSSL home dir");

			if (wolfSSL_CTX_use_PrivateKey_file(wctx, svrKey, SSL_FILETYPE_PEM)
				!= SSL_SUCCESS)
				err_sys("can't load server key file, "
					"Please run from wolfSSL home dir");
#endif
		} /* doPSK */
#elif !defined(NO_CERTS)
		if (!doPSK) {
			load_buffer(wctx, svrCert, WOLFSSL_CERT);
			load_buffer(wctx, svrKey, WOLFSSL_KEY);
		}
#endif

#if defined(WOLFSSL_SNIFFER)
		/* don't use EDH, can't sniff tmp keys */
		wolfSSL_CTX_set_cipher_list(wctx, "AES256-SHA");
#endif

		if (doPSK) {
#ifndef NO_PSK
			const char *defaultCipherList;

			SSL_CTX_set_psk_server_callback(wctx, va_ssl_psk_callback);
			SSL_CTX_use_psk_identity_hint(wctx, "openssl server");
#ifdef HAVE_NULL_CIPHER
			defaultCipherList = "PSK-NULL-SHA256";
#elif defined(HAVE_AESGCM) && !defined(NO_DH)
			defaultCipherList = "DHE-PSK-AES128-GCM-SHA256";
#else
			defaultCipherList = "PSK-AES128-CBC-SHA256";
#endif
			if (!SSL_CTX_set_cipher_list(wctx, defaultCipherList)  )
				err_sys("server can't set cipher list 2");
#endif
		}
		SocketFD = net_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		memset(&sa, 0, sizeof(sa));
		if (SocketFD == -1) {
			perror("https cannot create socket");
			exit(EXIT_FAILURE);
		}
		if (mode == 1) {
			sa.sin_family = AF_INET;
			sa.sin_port = htons(port);
			sa.sin_addr.s_addr = htonl(INADDR_ANY);
			if (net_bind(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1) {
				perror("http bind failed");
				net_close(SocketFD);
			}
			if (SocketFD != 0) {			//operation clear
				((va_default_context *)&defctx)->ctx = (void *)wctx;
				((va_default_context *)&defctx)->close = va_net_close;
				((va_default_context *)&defctx)->read = va_net_recv;
				((va_default_context *)&defctx)->write = va_net_send;
				((va_default_context *)&defctx)->offset = 0;
				((va_default_context *)&defctx)->seek = NULL;
				//defctx.callback = func;
				memcpy(&defctx.callback, func, sizeof(vm_function));
				defctx.sockfd = malloc(sizeof(SOCKET));
				defctx.thread = malloc(sizeof(pthread_t));
				((SOCKET *)defctx.sockfd)[0] = SocketFD;
				vm_set_retval(vm_create_object(VM_ARG, sizeof(va_net_context), (uchar *)&defctx));
				thread_arg = (va_net_thread_arg *)malloc(sizeof(va_net_thread_arg));
				if (thread_arg != NULL) {
					thread_arg->instance = (vm_instance *)malloc(sizeof(vm_instance *));
					thread_arg->netctx = (va_net_context *)malloc(sizeof(va_net_context));
					thread_arg->sockfd = SocketFD;
					memcpy(thread_arg->instance, VM_ARG, sizeof(vm_instance));
					memcpy(thread_arg->netctx, &defctx, sizeof(va_net_context));
					pthread_create((pthread_t *)thread_arg->netctx->thread, NULL, va_ssl_listen_task, thread_arg);
				}
			}
		}
		else {
			//after socket created start connecting
			net_connect(SocketFD, (const struct sockaddr *)vaddr->bytes, vaddr->len);

			if (SocketFD != 0) {			//operation clear
				((va_default_context *)&defctx)->ctx = (void *)wctx;
				((va_default_context *)&defctx)->close = va_net_close;
				((va_default_context *)&defctx)->read = va_net_recv;
				((va_default_context *)&defctx)->write = va_net_send;
				((va_default_context *)&defctx)->offset = 0;
				((va_default_context *)&defctx)->seek = NULL;
				defctx.callback.arg_count = 0;
				defctx.callback.offset = -1;
				defctx.sockfd = malloc(sizeof(SOCKET));
				defctx.thread = NULL;
				((SOCKET *)defctx.sockfd)[0] = SocketFD;
				vm_set_retval(vm_create_object(VM_ARG, sizeof(va_net_context), (uchar *)&defctx));
			}
		}
		break;
	}
exit_net_open:
	//OS_DEBUG_EXIT();
	return;
}
#endif

#if !defined(USE_WOLFSSL) && !defined(USE_OPENSSL)
void va_net_open(VM_DEF_ARG) {

}
#endif

void va_net_transmit(VM_DEF_ARG) {
	
}


LIB_API char * vm_get_request_url(VM_DEF_ARG) {
	net_instance * instance;
	instance = (net_instance *)VM_ARG->data;
	return instance->url;
}

LIB_API char * vm_get_callback_name(VM_DEF_ARG, void * callback) {
	int len = strlen((const char *)((net_callback *)callback)->name);
	if(len == 0) return NULL;
	return ((net_callback *)callback)->name;
}

LIB_API void * vm_install_callback(VM_DEF_ARG, vm_function * func) {
	net_instance * instance;
	net_bytecodes * bytecodes;
	pk_class * pkc;
	net_callback * callback;
	char function_name[256];
	//vm_function * func = (vm_function *)obj->bytes;
	instance = (net_instance *)VM_ARG->data;
	//ASSERT(instance);
	if(instance == NULL) return NULL;
	bytecodes = (net_bytecodes *)instance->bytecodes;
	//ASSERT(bytecodes);
	if(bytecodes == NULL) return NULL;
	pkc = (pk_class *)bytecodes->root;
	//ASSERT(pkc);
	if(pkc == NULL) return NULL;
	sprintf(function_name, "f%d_%d", func->arg_count, func->offset);
	printf("install callback : %s\n", function_name);
	callback = net_bytecodes_find_callback(bytecodes, function_name);
	if(callback == NULL) {
		//create new callback
		callback = net_callback_create(function_name, func->arg_count, func->offset);
		net_bytecodes_add_callback(bytecodes, callback);
	}
	return callback;
}
