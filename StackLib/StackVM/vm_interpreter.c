#include "../defs.h"
#include "../config.h"
#include "../Stack/sem_proto.h"
#include "../Stack/lex_proto.h"

#include "vm_stack.h"
#include "vm_framework.h"
#ifdef CGI_APPLICATION
#include "../libcgi/src/cgi.h"
#include "../libcgi/src/error.h"
#endif
#include "../Stack/pkg_linker.h"
#include "../Stack/asm_streamer.h"
#include <stdio.h>
#include "build_int.h"


void vi_register_system_apis() {
	//BYTE i;
	vm_api_entry * iterator = (vm_api_entry *)&g_vaRegisteredApis;
	while(iterator->entry != NULL) {
		//printf("install api %s : %d\r\n", iterator->name, iterator->id);
		sp_install_api(iterator->name, iterator->id, 0);
		iterator++;
	} 
}

uint8 sample_code[] = "\
#define null \"\"\r\n\
#define true \"true\"\r\n\
#define false \"false\"\r\n\
\r\n\
//comment;\r\n\
class program {\r\n\
	function foreach(x, func) {\r\n\
		print(x.count());\r\n\
			for(var i=0;i<x.count();i++) {\r\n\
				print(x.pen);\r\n\
				print(x[i]);\r\n\
			}\r\n\
	}\r\n\
    public function main() {\r\n\
	var w=\"\";\r\n\
	var t = new c1();\r\n\
	this()->var1 = \"variable 1\";\r\n\
	t->x1(function(tx) { print(tx); });	\r\n\
	print(\"var 1 : \" + this()->var1);\r\n\
	print(\"123\".hash(\"crc32\").to_hex());\r\n\
	var jstr = { head:{ eyes : 2} , limbs : { hands:2, legs:2, neck:1 } };\r\n\
        var my_array = jstr;\r\n\
		print(jstr.to_hex());\r\n\
		print(jstr.to_json());\r\n\
		jstr.head.eyes=1;\r\n\
		print(jstr.to_json());\r\n\
		//my_array.pen.pencil = {\"pencil\":2};\r\n\
		//for(var i=0;i<7;i++) print(i);\r\n\
		var my_array = [\"apple\", {fruit:1}];\r\n\
        //print(my_array.to_hex());\r\n\
        //print(my_array.to_json().from_json().to_hex());\r\n\
        //print(\"number of objects : \" + my_array.count());\r\n\
        //print(\"array 1: \" + my_array[1]);\r\n\
        my_array.add(\"pineapple\");\r\n\
		print(my_array.to_hex());\r\n\
        print(my_array.to_json());\r\n\
		my_array[2] = 0;\r\n\
        print(my_array.to_json());\r\n\
	var arr = [\"pen\", \"pineapple\", \"apple\", \"pen\", 0.1];\r\n\
	print(arr.to_json());\r\n\
	print(\"count : \" + arr.count());\r\n\
		foreach(arr, function(t) {\r\n\
			print(t);\r\n\
		});\r\n\
		print(\"hash : \" + \"123\".hash(\"lrc\").to_dec());\r\n\
		print(my_array.remove(1).to_json());\r\n\
	var lstr = \"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\";\r\n\
	print(lstr);\r\n\
	if(lstr == lstr) {\r\n\
		print(\"hello \" + (0.3 + arr[4]));\r\n\
	} else {\r\n\
		print(\"no hello\");\r\n\
	}\r\n\
	print(\"result = \" + !2);\r\n\
    }\r\n\
}\r\n\r\n\
class c1 {\r\n\
	public function x1(x) {\r\n\
		x(\"from x1\");\r\n\
	}\r\n\
}\r\n\
\r\n\
\r\n";

void cgi_about() {
	printf("Orb-Weaver %d.%d.%d CGI Interpreter <br>", IS_MAJOR_VERSION, IS_MINOR_VERSION, BUILD_NUMBER);
	printf("Copyright 2018 @ Orbleaf Technology<br>");
}

void int_about(char * name) {
	vm_api_entry * iterator = (vm_api_entry *)&g_vaRegisteredApis;
	printf("Orb-Weaver %d.%d.%d Interpreter\n", IS_MAJOR_VERSION, IS_MINOR_VERSION, BUILD_NUMBER);
	printf("Copyright 2018 @ Orbleaf Technology\n");
	printf("\nUsage : \n");
	printf("%s [filename]\n", name);
	printf("\nAvailable System APIs : \n");
	while(iterator->entry != NULL) {
		printf("%s()\r\n", iterator->name);
		//sp_install_api(iterator->name, iterator->id, 0);
		iterator++;
	} 
}

int main(int argc, char * argv[]) {
	uint32 err;
	uint32 ssize, n;
	char * source = NULL;
	uint32 headersize = 0;
	FILE * infile;
	pp_config * pconfig;
	as_record * iterator;
	vm_instance vcit;
	as_init(NULL);
	pk_init(NULL);
	vi_register_system_apis();
#ifdef CGI_APPLICATION
	cgi_init();
	cgi_process_form();	
	cgi_init_headers();
	//_istream_code_size = 0;
	if(cgi_param("script")) {
		source = cgi_param("script");
	} else {
		cgi_about();
		cgi_end();
		return 0;
	}
	pconfig = sp_clr_init(_RECAST(uchar *, source), strlen(source));
#else
	if(argc > 1 ) {
		printf("file : %s\n", argv[1]);
		infile = fopen(argv[1], "r");
		if(infile == NULL) return 0;
		fseek(infile, 0, SEEK_END);
		ssize = ftell(infile);
		printf("file size : %d\n", ssize);
		source = (char *)malloc(ssize);
		n = 0;
		fseek(infile, 0, SEEK_SET);
		for(n=0;n<ssize;n++) source[n] = fgetc(infile);
	} else {
		int_about(argv[0]);
		return 0;
	}
	printf(source);
	pconfig = sp_clr_init(_RECAST(uchar *, source), ssize );
#endif
	err = sp_parse();
	//printf("error : %d\n", err);
	if(err == 0) {
		//printf("link and optimize\n");
		//fflush(0);
		is_link_optimize(0);
		headersize = pk_flush_root();
		//lk_clear_entries();
		//bytecodes = gcnew array<System::Byte>(headersize + _istream_code_size);
		//pin_ptr<System::Byte> ptr = &bytecodes[0];
		//memcpy(_RECAST(uchar *, ptr), _pkg_buffer, headersize);
		//memcpy(_RECAST(uchar *, ptr) + headersize, _istream_code_buffer, _istream_code_size);
		//_istream_code_size = 0;
		lk_set_root(pk_get_root());
#ifndef CGI_APPLICATION
		lk_dump_classes();
#endif
		printf("header size : %d\r\n", headersize);
		//iterator = as_get_enumerator();
		//while(iterator != NULL) {
			//lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
		//	printf("%s\n", iterator->buffer);
		//	iterator = as_get_next_record(iterator);
		//}
		vm_init(&vcit, pk_get_root(), MEM_HEAP_SIZE);
		printf("start execute\n");
		if(vm_init_exec(&vcit, "program", "main?0") == 0) {
			vm_decode(&vcit, 0, 0);
			vm_close(&vcit);
		}
	} else {
		
		headersize = pk_flush_root();
		//_istream_code_size = 0;
	}
	lk_clear_entries();
		
	free(pconfig);
	//return bytecodes; 
#ifdef CGI_APPLICATION
	cgi_end();
#endif
	return 0;
}
