#ifndef _CONFIG__H

/* Midgard Configuration */
#define MIDGARD_ACTIVATED			1				//use midgard
#define MIDGARD_HEAP_SIZE			0x4000			//total heap size
#define MIDGARD_LOW_OVERHEAD		1				//4 bytes header
#define MIDGARD_HEAP_ALLOC			0				//use internal heap alloc
#define MIDGARD_32BIT_MODE			0				//32/16 bit mode

/* VM Configuration */
#define VM_CODEDEBUG				1
#define VM_GC_DEBUG					0

/* IL Streamer  Configuration */
#define IS_MAJOR_VERSION			1
#define IS_MINOR_VERSION			4
#define IS_CODEDEBUG				1
#define IS_REL_JUMP_ADDRESS			1

//#define STANDALONE_COMPILER			1

//#define SP_MAX_VARS					4
//#define IL_MAX_CODESIZE				0x200
#define STACK_CONVERTER_APIS 		1
#define STACK_CRYPTO_APIS 			1
#define STACK_BIT_APIS				0
#define STACK_VAR_APIS				1

#ifndef MACH_LITTLE_ENDIAN
#define MACH_LITTLE_ENDIAN			1
#endif

//#define WOLFSSL_MDK_ARM
//#define WOLFSSL_MDK5
//#define WOLFSSL_USER_IO

//#define WOLFSSL_STM32F2
//#define STM32F2_CRYPTO
//#define SMT32F2_RNG
#define NO_SESSION_CACHE
//#define NO_CERTS

#define NO_WRITEV
#define NO_WOLFSSL_DIR
//#define NO_MAIN_DRIVER
//#define NO_FILESYSTEM
#define NO_WOLFSSL_MEMORY

#define SINGLE_THREADED

//#define WOLFSSL_DER_LOAD
#define WOLFSSL_SMALL_STACK
#define HAVE_NULL_CIPHER
#define WOLFSSL_USER_TIME
#define NO_TIME_H
#define USER_TICKS

#define NO_ECHOSERVER
#define NO_ECHOCLIENT
#define NO_SIMPLE_SERVER
#define NO_SIMPLE_CLIENT

//wolf ssl optimization
//#define NO_PSK
//#define NO_WOLFSSL_CLIENT
//#define NO_WOLFSSL_SERVER
//#define NO_DSA
//#define NO_HMAC
#define HAVE_AESGCM
#define NO_MD4
//#define NO_MD5
//#define NO_SHA256
#define NO_PWDBASED
#define NO_SESSION_CACHE
#define NO_RABBIT
#define NO_HC128
#define NO_DES3
//#define NO_ERROR_STRINGS
//#define MAX_DIGEST_SIZE 	32
//#define NO_TLS
//#define NO_DH
//#define NO_RSA
//#define NO_SHA
#define WOLFSSL_KEY_GEN
#define HAVE_ECC
//#define HAVE_TRUNCATED_HMAC
#define WOLFSSL_SHA384
#define WOLFSSL_SHA512
#define HAVE_AESCCM
#define HAVE_CAMELLIA
//#define HAVE_TLS_EXTENSIONS
//#define OPENSSL_EXTRA
//#define WOLFSSL_DER_LOAD
//#define WOLFSSL_DTLS

#define TFM_TIMING_RESISTANT
#define USE_FAST_MATH

#define PNG_ABORT()

static  int ValidateDate(const unsigned char* date, unsigned char format, int dateType){ return 1; }

#if    defined(MDK_CONF_RTX_TCP_FS)
#include "WolfSSL/inc/config-RTX-TCP-FS.h"
#elif  defined(MDK_CONF_TCP_FS)
#include "WolfSSL/inc/config-TCP-FS.h"
#elif  defined(MDK_CONF_FS)
#include "WolfSSL/inc/config-FS.h"
#elif  defined(MDK_CONF_BARE_METAL)
#include "WolfSSL/inc/config-BARE-METAL.h"
#elif  defined(MDK_WOLFLIB)
#include "WolfSSL/inc/config-WOLFLIB.h"
#endif

#define _CONFIG__H
#endif
