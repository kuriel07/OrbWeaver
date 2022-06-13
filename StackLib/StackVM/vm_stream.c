#include <string.h>
#include <sys/types.h>
#include "defs.h"
#include "config.h"
#include "vm_stream.h"

void * vm_stream_create(void * stream) {
	stream_page * iterator = (stream_page *)stream;
	stream_page * page = (stream_page *) malloc(sizeof(stream_page));
	page->size = 0;
	page->next = NULL;
	page->page_size = MAX_PAGE_SIZE;
	if(iterator != NULL) {
		while(iterator->next != NULL) {
			iterator = iterator->next;
		}
		if(iterator != NULL) iterator->next = page;
	}
	return page;
}

void vm_stream_release(void * stream) {
	stream_page * iterator = (stream_page *)stream;
	stream_page * candidate;
	if(iterator != NULL) {
		while(iterator != NULL) {
			candidate =  iterator;
			iterator = iterator->next;
			free(candidate);
		}
	}
}

stream_file * vm_stream_open(int type) {
	stream_file * file = (stream_file *)malloc(sizeof(stream_file));
	stream_page * page = (stream_page *)vm_stream_create(NULL);
	file->type = type;
	file->first = page;
	file->current = page;
	file->offset = 0;
	return file;
}

void vm_stream_write(stream_file * file, uint8 * buffer, uint32 size) {
	uint32 offset = 0;
	uint32 wlen = 0;
	stream_page * current;	// = file->current;
	if (file == NULL) return;
	current = file->current;
	if (current == NULL) return;
	switch (file->type) {
		case STREAM_TYPE_CONSOLE:
			printf("%s\n", buffer);
			break;
		case STREAM_TYPE_MEMORY:
			if ((current->page_size - current->size) > size) {
				//write current stream
				memcpy(current->bytes + current->size, buffer, size);
				file->offset += size;
				current->size += size;
			}
			else {
				//write stream per chunk
				for (offset = 0; offset<size;offset += MAX_PAGE_SIZE) {
					current = (stream_page *)vm_stream_create(current);
					wlen = ((size - offset) > MAX_PAGE_SIZE) ? MAX_PAGE_SIZE : (size - offset);
					memcpy(current->bytes + current->size, buffer + offset, wlen);
					current->size += wlen;
					file->offset += wlen;
					file->current = current;
				}
			}
			break;
	}
}

uint32 vm_stream_get_count(stream_file * file) {
	return file->offset;
}

uint32 vm_stream_get_bytes(stream_file * file, uint8 * bytes) {
	uint32 total_size = file->offset;
	uint32 offset = 0;
	stream_page * iterator = (stream_page *)file->first;
	if(iterator != NULL) {
		while(iterator != NULL) {
			memcpy(bytes + offset, iterator->bytes, iterator->size);
			offset += iterator->size;
			iterator = iterator->next;
		}
	}
	return offset;
}


void vm_stream_close(stream_file * file) {
	if(file != NULL) {
		vm_stream_release(file->first);
		free(file);
	}
}