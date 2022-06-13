#include "../defs.h"
#ifndef MSTREAM__H


typedef struct stream_page stream_page;
typedef struct stream_file stream_file;
#define MAX_PAGE_SIZE 4096

#define STREAM_TYPE_CONSOLE				1
#define STREAM_TYPE_MEMORY				2

struct stream_page {
	uint32 size;
	uint32 page_size;
	struct stream_page * next;
	uint8 bytes[MAX_PAGE_SIZE];
};

struct stream_file {
	int type;
	stream_page * first;
	stream_page * current;
	uint32 offset;
};

#ifdef __cplusplus
extern "C" {
#endif
void * vm_stream_create(void * stream);
void vm_stream_release(void * stream) ;
stream_file * vm_stream_open(int type);
void vm_stream_write(stream_file * file, uint8 * buffer, uint32 size);
uint32 vm_stream_get_count(stream_file * file);
uint32 vm_stream_get_bytes(stream_file * file, uint8 * bytes);
void vm_stream_close(stream_file * file);

#ifdef __cplusplus
}
#endif

#define MSTREAM__H
#endif
