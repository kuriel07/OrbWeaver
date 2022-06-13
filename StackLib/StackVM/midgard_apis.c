#include "../defs.h"
#include "midgard.h"
//#include "ucos_ii.h"
//#include "2440lib.h"

uint32 m_init_alloc(heap_manager * heap) _REENTRANT_
{
	//global memory management allocation
	//chunkroot needed in order to initialize memory manager
	((alloc_chain *)heap->root)->size = 0;		//chunkroot tidak bisa di s_free
	((alloc_chain *)heap->root)->next = (alloc_chain *)((intptr)((alloc_chain *)heap->root) + (sizeof(alloc_chain) + (sizeof(alloc_chain) % 4) ));
	#if ALLOC_CHAIN_USE_PREV
	((alloc_chain *)alloc_buffer)->prev = NULL;
	#endif
	((alloc_chain *)heap->root)->next->next = NULL;		//if not null might caused bug on allocation
	//_chunkroot->next = 0;
	//_total_heap = _chunkroot->size;
	return 0;
}

#ifdef _USE_MIDGARD
#if MEMORY_WIDTH == MW_32BIT
void * m_alloc(heap_manager * heap, uint32 size) _REENTRANT_ {
#else
void * m_alloc(uint16 size) _REENTRANT_ {
#endif
	//#ifdef OS_uCOS_II_H
	//heap used by several task simultaneously like sharing memory
	//#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    //OS_CPU_SR  cpu_sr;
    //#endif
    //#endif
    void * allocated = NULL;
    alloc_chain *alloc_ptr = ((alloc_chain *)heap->root);
	alloc_chain *alloc_ptr_temp = (alloc_chain *)NULL;
	alloc_chain * candidate;
	//alloc_chain *temp;
	//#ifdef OS_uCOS_II_H
	//OS_ENTER_CRITICAL();
	//#endif
	//size = size + (4 - (size % 4));

#if 0
	get_next_chain:
	if(alloc_ptr->next == NULL) {		//allocate new chunk at the end of the heap
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * creating new chunk with size %i\n", size);
		#endif
		alloc_ptr->size = size;
		alloc_ptr->next = (alloc_chain *)((intptr)alloc_ptr + sizeof(alloc_chain) + size);
		//disini cek STACK POINTER, dikhawatirkan akan bertabrakan dengan stack, sebaiknya antara stack pointer dan heap dibuat
		//GAP yang cukup lebar karena dikhawatirkan ukuran stack akan bertambah setelah keluar dari fungsi s_alloc karena automatic allocation
		#if ALLOC_CHAIN_USE_PREV
		alloc_ptr->prev = alloc_ptr_temp;
		#endif
		alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
		#if ALLOC_CHAIN_USE_PREV
		alloc_ptr_temp->prev = alloc_ptr;
		#endif
		alloc_ptr_temp->next = NULL;
		alloc_ptr_temp->size = 0;
		//alloc_ptr = alloc_ptr_temp;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * address of new chunk at %ld\n", (alloc_chain *)((intptr)alloc_ptr));
		#endif
		//Uart_Printf("%d\n", alloc_ptr);
		//#ifdef OS_uCOS_II_H
		//OS_EXIT_CRITICAL();
		//#endif
		//_total_heap += alloc_ptr->size;
		/*if(_total_heap > _maximum_total_heap) {
			printf("Maximum total heap : %d\n", _total_heap);
			_maximum_total_heap = _total_heap;
		}  */
		return (void *)((intptr)alloc_ptr + sizeof(alloc_chain));	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	alloc_ptr_temp = alloc_ptr;
	alloc_ptr = alloc_ptr->next;
	if((intptr)alloc_ptr >= (intptr)((intptr)alloc_ptr_temp + (alloc_ptr_temp->size + sizeof(alloc_chain)) + (sizeof(alloc_chain) + size))) {
		//printf("allocate in previous memory : %x, %x\n", (uint32)alloc_ptr, ((uint32)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain) + sizeof(alloc_chain) + size + (size % 4)));
		//printf("allocate in previous memory : %x, %x\n", (intptr)alloc_ptr, ((intptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain)));
		//printf("allocate in previous memory : %x, %x\n", (uint32)alloc_ptr, sizeof(alloc_chain));
		//allocate new heap using FFA (First Fit Algorithm)
		//Uart_Printf("allocate previous heap\n");
		candidate = (alloc_chain *)((intptr)alloc_ptr_temp + (alloc_ptr_temp->size + sizeof(alloc_chain)));
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * creating new chunk with size %i\n", size);
		#endif
		//candidate->size = size + (size % 4);
		candidate->size = size;
		alloc_ptr_temp->next = candidate;
		#if ALLOC_CHAIN_USE_PREV
		candidate->prev = alloc_ptr_temp;
		#endif
		candidate->next = alloc_ptr;
		#if ALLOC_CHAIN_USE_PREV
		alloc_ptr->prev = candidate;
		#endif
		//Uart_Printf("%d %d %d %d\n", candidate->prev, candidate, candidate->size, candidate->next);
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * address of new chunk at %x\n", (uint32)candidate);
		#endif
		//#ifdef OS_uCOS_II_H
		//Uart_Printf("exit\n");
		//OS_EXIT_CRITICAL();
		//#endif
		//_total_heap += candidate->size;
		/*if(_total_heap > _maximum_total_heap) {
			printf("Maximum total heap : %d\n", _total_heap);
			_maximum_total_heap = _total_heap;
		}*/
		return (void *)((intptr)candidate + (intptr)sizeof(alloc_chain));
	}
	goto get_next_chain;
#endif
	while (TRUE) {
		if (alloc_ptr->next == NULL) {		//allocate new chunk at the end of the heap
			if (((uint8 *)alloc_ptr + size + (sizeof(alloc_chain) + sizeof(alloc_chain))) >= ((uint8 *)heap->root + heap->size)) break;        //not enough memory
			alloc_ptr->size = size;
			alloc_ptr->next = (alloc_chain *)((uint8 *)alloc_ptr + (sizeof(alloc_chain) + size));
			alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
			alloc_ptr_temp->next = NULL;
			alloc_ptr_temp->size = 0;
			allocated = (alloc_chain *)((uint8 *)alloc_ptr + sizeof(alloc_chain));	//return pointer sekarang + ukuran header karena *[header]+[body]
			break;
		}
		alloc_ptr_temp = alloc_ptr;
		alloc_ptr = (alloc_chain *)alloc_ptr->next;
		if ((uint8 *)alloc_ptr >= ((uint8 *)alloc_ptr_temp + (alloc_ptr_temp->size + (sizeof(alloc_chain) + sizeof(alloc_chain)) + size))) {

			candidate = (alloc_chain *)((uint8 *)alloc_ptr_temp + (alloc_ptr_temp->size + sizeof(alloc_chain)));
			candidate->size = size;
			alloc_ptr_temp->next = candidate;
			candidate->next = alloc_ptr;
			//OS_RESTORE_CONTEXT
			allocated = (alloc_chain *)((uint8 *)candidate + sizeof(alloc_chain));
			break;
		}
	}
	return allocated;
}

#if MEMORY_WIDTH == MW_32BIT
uint32 m_get_allocated_space(heap_manager * heap) _REENTRANT_ {
	uint32 alloc_space = 0;
#else
uint16 m_get_allocated_space(void) _REENTRANT_ {
	uint16 alloc_space = 0;
#endif
    alloc_chain *alloc_ptr = ((alloc_chain *)heap->root);
	//size = size + (4 - (size % 4));
	get_next_chain:
	if(alloc_ptr->next==NULL) {		//allocate new chunk at the end of the heap
		alloc_space += alloc_ptr->size + sizeof(alloc_chain);
		return alloc_space;	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	//alloc_ptr_temp = alloc_ptr;
	alloc_space += alloc_ptr->size;
	alloc_ptr = alloc_ptr->next;
	goto get_next_chain;
}

void m_free(heap_manager * heap, void *ptr) _REENTRANT_ {
	//#ifdef OS_uCOS_II_H
	//#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    //OS_CPU_SR  cpu_sr;
    //#endif
    //#endif
	#if ALLOC_CHAIN_USE_PREV
    alloc_chain *alloc_ptr = (alloc_chain *)((intptr)ptr - (intptr)sizeof(alloc_chain));
	alloc_chain *alloc_prev = alloc_ptr->prev;
	alloc_chain *alloc_next = alloc_ptr->next;
	if(ptr == NULL) return;
	if(alloc_prev == NULL) return;		//this memory is already freed
	if(alloc_ptr->size == 0) return;		//this memory is already freed
	/*if(alloc_next == NULL) {		//this is the last chunk
		printf("this is the last chunk\n");	
	}*/
	//#ifdef OS_uCOS_II_H
	//OS_ENTER_CRITICAL();
	//#endif
	//alloc_prev = alloc_ptr->prev;
	#ifdef MIDGARD_DEBUG_ACTIVATED
	//printf(" * delete chunk at %x\n", (intptr)alloc_ptr);
	//printf(" * chunk at %x, next chunk at ", (intptr)alloc_prev);
	#endif
	//_total_heap -= alloc_ptr->size;
	//alloc_ptr = alloc_ptr->next;
	if(alloc_ptr == ((alloc_chain *)alloc_buffer)) {
		//alloc_prev->next = alloc_ptr->next;
		//alloc_prev->next->prev = alloc_prev;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf("%x\n", (intptr)alloc_prev->next);
		#endif
	} else { 
		alloc_next->prev = alloc_prev;
		alloc_prev->next = alloc_next;
		//alloc_prev->next->prev = alloc_prev;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf("%x\n", (intptr)alloc_prev->next);
		#endif
		//s_gc();
		//getch();
	}
	alloc_ptr->next = NULL;
	alloc_ptr->prev = NULL;
	alloc_ptr->size = 0;
	#else
	alloc_chain * alloc_ptr = ((alloc_chain *)heap->root);
	alloc_chain * temp;
	//alloc_chain *alloc_prev = NULL;
	while(alloc_ptr != NULL) {
		//if(alloc_ptr->next == NULL) return;		//not found
		if ((alloc_ptr->size == 0 && alloc_ptr->next != NULL ) && alloc_ptr->next->size == 0) {
			//printf("compacting\n...");
			alloc_ptr->next = alloc_ptr->next->next;
		}
		if((intptr)alloc_ptr->next == (intptr)((uchar *)ptr - sizeof(alloc_chain))) {		//found
			temp = alloc_ptr->next;
			alloc_ptr->next = temp->next;
			if (alloc_ptr->next == NULL) { 		//shift end of heap added 2015.04.09 v2.1, modified 2015.04.14
				temp = (alloc_chain*)((uchar *)alloc_ptr + (alloc_ptr->size + sizeof(alloc_chain)));
				alloc_ptr->next = temp;
			}
			temp->size = 0;
			temp->next = NULL;
			return;
		}
		alloc_ptr = alloc_ptr->next;		//next iterator
	}

	#endif
	//#ifdef OS_uCOS_II_H
	//OS_EXIT_CRITICAL();
	//#endif
}
#endif


