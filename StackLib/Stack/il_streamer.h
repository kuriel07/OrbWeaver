#include "../defs.h"
#include "../StackVM/vm_stack.h"
#ifndef _IL_STREAMER__H

#define MAX_BUFFER_SIZE		65536

//.db	-> data byte
//.dw	-> data word

//APIs:
//disptext	-> display text
//getinput	-> get input
//sendsm		-> send short message
//sendussd	-> send ussd
//getlocal	-> local info
//showlist 	-> select item
#define IS_OPTIMIZE_L0		0
#define IS_OPTIMIZE_L1		1
#define IS_OPTIMIZE_L2		2
#define IS_OPTIMIZE_L3		4

typedef struct optrec {
	struct optrec * next;			//internal use
	uchar ins;						//instruction
	uint16 index;					//
	uint16 offset;					//original offset
	uint16 opt_offset;				//optimized offset
} optrec;

typedef struct coderec {
	struct coderec * next;
	uint16 offset;					//original offset
	uint16 rdc;						//code reduction, opt_offset = original_offset - code_reduction
} coderec;

typedef struct lblrec {
	struct lblrec * next;
	uint32 offset;
	uint16 refcount;
	void * tag;
	uchar name[240];
} lblrec;

extern uchar _istream_code_buffer[MAX_BUFFER_SIZE];
extern uchar _istream_data_buffer[MAX_BUFFER_SIZE];

#ifdef __cplusplus
extern "C" {
#endif
void is_init(uchar * inpath);
uint32 is_push_constant(uint16 size, uchar * value) ;
uint32 is_gencode(uint32 offset, uchar opcode, ...);
uint32 is_link_optimize(uchar mode);
void * is_get_codebuffer();
void is_file_flush();

//label record (debug information)
uint16 is_push_lblrec(uint16 offset, uchar * name);
lblrec * is_get_lblrec(uint16 offset);
void is_clear_unused_lblrec(void);
void is_clear_lblrec(void);

#ifdef __cplusplus
}
#endif
#define _IL_STREAMER__H
#endif
