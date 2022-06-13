#include "defs.h"
#include "config.h"

//#ifndef WBUFFER__H

extern char hostname[128];
extern char www_directory[512];
extern char mod_directory[512];
extern int http_port;
extern int https_port;
extern char inc_directory[512];
extern char cert_file[512];
extern char privkey_file[512];

#ifdef __cplusplus
extern "C" {
#endif
void x_init();
void * x_malloc(uint32 size) ;
void x_free(void * ptr);
void x_cleanup();

#ifdef __cplusplus
}
#endif
//#define WBUFFER__H
//#endif
