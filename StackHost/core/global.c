#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "defs.h"
#include "config.h"
#include "StackHost.h"
#include "core/port.h"
#include "core/global.h"
#include "Stack/pkg_encoder.h"
#ifdef WIN32
#include "util/dirent.h"
#else
#include "dirent.h"
#endif
#include "StackVM/vm_stack.h"
#include "StackVM/midgard.h"


//uint8 global_buffer[8000000];
//heap_manager global_heap = { (alloc_chain *)global_buffer , sizeof(global_buffer) };
#define GLOBAL_HEAP_MAX 8000000
static heap_manager * global_heap = NULL;

void x_init() {
	//heap_manager * heap;
	global_heap = malloc(GLOBAL_HEAP_MAX + sizeof(heap_manager));
	if(global_heap != NULL) {
		global_heap->root = (uint8 *)global_heap + sizeof(heap_manager);
		global_heap->size = GLOBAL_HEAP_MAX;
		m_init_alloc(global_heap);
		vm_init_global(NULL);
	}
}

void * x_malloc(uint32 size) {
	if(global_heap == NULL) return NULL;
	return m_alloc(global_heap, size);
}

void x_free(void * ptr) {
	m_free(global_heap, ptr);
}

void x_cleanup() {
	if(global_heap != NULL) free(global_heap);
}
