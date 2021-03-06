#ifndef _TOKEN__H
#include "defs.h"

typedef struct token /* The Semantic Records */
{
	int length;
	uchar bytes[4096]; /* Identifiers */
} yytoken;

/* symbol */
#define VARIABLE	257		//foo, a, index, label		--> variable
#define CONSTANT	258		//"asda", {0,0,0}			--> constant value
#define N_CONSTANT	259		//negative constant
#define NUMERIC		261		//number
#define N_NUMERIC	262		//negative numeric
#define ARRAY		260		//bytes

#define P_VOID		290
#define P_AUTO		291
#define P_BYTE		292
#define P_HWORD		293
#define P_WORD		294
#define P_DWORD		295
#define P_FLOAT		296
#define P_DOUBLE	297
#define P_STRING	298
#define P_PTR		299
#define P_VAR		300		//string
#define P_NEW		301
#define P_HEX		302
#define P_SIZEOF	310		//sizeof

#define P_STDCALL	314
#define P_FASTCALL	315

#define P_TRY		345
#define P_CATCH		346
#define P_IN		347		//foreach
#define P_FOREACH	348		//in
#define P_GOTO		349		//goto
#define P_IF		350		//if
#define P_ELSE		351		//else
#define P_SWITCH	352		//switch
#define P_CASE		353		//case 
#define P_DEFAULT	354
#define P_BREAK		355		//break
#define P_FOR		356
#define P_WHILE		357		//while
#define P_DO		358		//do
#define P_CONTINUE	359		//continue


#define P_FUNCTION	360		//function
#define P_RETURN	361		//return
#define P_ALIAS		362		//alias
#define P_EVENT		363		//event
#define P_PROTO		364		//proto
#define P_EXTERN	365		//extern
#define P_SYSCALL	366		//syscall

#define P_CLASS		380		//class
#define P_INTERFACE	381		//interface
#define P_PACKAGE	382		//package
#define P_IMPORT	383		//import
#define P_PUBLIC	385		//public
#define P_PRIVATE	386		//private
#define P_PROTECTED	387		//protected
#define P_LIB		388		//lib

#define L_SB		398		//left square bracket
#define R_SB		399		//right square bracket
#define L_CL		400		//left curl
#define R_CL		401		//right curl
#define EOS			402		//semicolon (end of statement)
#define EOC			403		//colon
#define L_BR		406		//left bracket
#define R_BR		407		//right bracket
#define T_EQ		410		//type equal
#define T_GT		411		//type greater than
#define T_LT		412		//type less than
#define T_GTEQ		413		//type greater than
#define T_LTEQ		414		//type less than
#define T_NE		415		//type not equal
#define T_AND		416		//type and
#define T_OR		417		//type or

#define S_EQ		450		
#define S_ADD		451
#define S_SUB		452
#define S_ADDEQ		453
#define S_SUBEQ		454
#define S_ADDADD	455
#define S_SUBSUB	456
#define S_MUL		457
#define S_DIV		458
#define S_MULEQ		459
#define S_DIVEQ		460
#define S_MOD		461
#define S_MODEQ		462
#define S_RNEXT		470
#define S_PDOT		471
#define S_AND		472
#define S_OR		473
#define S_XOR		474
#define S_NOT		475
#define S_ANDEQ		476
#define S_OREQ		477
#define S_XOREQ		478
#define S_SHL		480
#define S_SHR		481
#define S_SHLEQ		482
#define S_SHREQ		483

extern int yylex();
#define YYTOKENTYPE
#define _TOKEN__H
#endif
