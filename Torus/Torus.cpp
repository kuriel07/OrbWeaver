// Torus.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "dlfcn.h"
#include <stdio.h>

int main(int argc, char * argv[])
{
	//char a = 20; short b = 21; int c = 23; __int64 d = 24; float e = 2.4;
	void * dll = dlopen("StackHost.dll", RTLD_GLOBAL);
	//printf("func 4 called %d, %d, %d, %lld, %f\n", a, b, c, d, e);
	int (* host)(int argc, char * argv[]) = (int (* )(int, char * []))dlsym(dll, "host_main");
	if(host != 0) {
		return host(argc, argv);
	}
	return -1;
}

