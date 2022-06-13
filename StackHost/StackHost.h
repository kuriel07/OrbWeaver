#include "defs.h"
#include "config.h"
#include <string.h>
#include "core/net.h"
#ifndef OWS__H

#define CAST_FUNC(x)		((void (*)())x)
#ifdef __cplusplus
extern "C" {
#endif
net_bytecodes * stack_load_file(char * filename, int is_script);
uint32 stack_exec_bytecodes_json(net_instance * instance, net_bytecodes * b, char * name, char * method, char * arguments);
uint32 stack_exec_bytecodes(net_instance * instance, net_bytecodes * b, char * name, char * method, int argc, char * argv[]);

void stack_setup_modules();

LIB_API int host_main(int argc, char * argv[]);

#ifdef __cplusplus
}
#endif

#define OWS__H
#endif
