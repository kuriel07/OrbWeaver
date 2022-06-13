#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "defs.h"
#include "config.h"
#include "StackHost.h"
#include <dlfcn.h>

int main(int argc, char * argv[]) {
	//void * dll = dlopen("liborbweaver.so", RTLD_GLOBAL);
#if 0
	printf("libgmodule-2.0.so.0\n");
	dlopen("/usr/lib/x86_64-linux-gnu//usr/lib/x86_64-linux-gnu/libgmodule-2.0.so.0", RTLD_GLOBAL);
	printf("/lib/x86_64-linux-gnu/libc.so.6\n");
	dlopen("/lib/x86_64-linux-gnu/libc.so.6", RTLD_GLOBAL);
	printf("/lib64/ld-linux-x86-64.so.2\n");
	dlopen("/lib64/ld-linux-x86-64.so.2", RTLD_GLOBAL);
	printf("/usr/lib/x86_64-linux-gnu/libffi.so.6\n");
	dlopen("/usr/lib/x86_64-linux-gnu/libffi.so.6", RTLD_GLOBAL);
	printf("/lib/x86_64-linux-gnu/libpcre.so.3\n");
	dlopen("/lib/x86_64-linux-gnu/libpcre.so.3", RTLD_GLOBAL);
	printf("/lib/x86_64-linux-gnu/libpthread.so.0\n");
	dlopen("/lib/x86_64-linux-gnu/libpthread.so.0", RTLD_GLOBAL);
	printf("libglib-2.0.so.0\n");
	dlopen("/usr/lib/x86_64-linux-gnu/libglib-2.0.so.0", RTLD_GLOBAL);
	printf("libgobject-2.0.so.0\n");
	dlopen("/usr/lib/x86_64-linux-gnu/libgobject-2.0.so.0", RTLD_GLOBAL);
	printf("libgio-2.0.so.0\n");
	dlopen("/usr/lib/x86_64-linux-gnu/libgio-2.0.so.0", RTLD_GLOBAL);
	printf("libgdk_pixbuf-2.0.so.0\n");
	dlopen("/usr/lib/x86_64-linux-gnu/libgdk_pixbuf-2.0.so.0", RTLD_GLOBAL);
	printf("libgdk-3.so.0\n");
	dlopen("/usr/lib/x86_64-linux-gnu/libgdk-3.so.0", RTLD_GLOBAL);
	printf("libpangocairo-1.0.so.0\n");
	dlopen("/usr/lib/x86_64-linux-gnu/libpangocairo-1.0.so.0", RTLD_GLOBAL);
	printf("libcairo.so.2\n");
	dlopen("/usr/lib/x86_64-linux-gnu/libcairo.so.2", RTLD_GLOBAL);
	printf("libatk-1.0.so.0\n");
	dlopen("/usr/lib/x86_64-linux-gnu/libatk-1.0.so.0", RTLD_GLOBAL);
	printf("libgtk-3\n");
	dlopen("/usr/lib/x86_64-linux-gnu/libgtk-3.so", RTLD_GLOBAL);
	printf("libopencv_core\n");
	dlopen("libopencv_core.so", RTLD_GLOBAL);
	printf("libopencv_imgproc\n");
	dlopen("libopencv_imgproc.so", RTLD_GLOBAL);
	printf("libopencv_imgcodecs\n");
	dlopen("libopencv_imgcodecs.so", RTLD_GLOBAL);
	printf("libopencv_cudev\n");
	dlopen("libopencv_cudev.so", RTLD_GLOBAL);
	printf("libopencv_videoio\n");
	dlopen("libopencv_videoio.so", RTLD_LAZY);
	printf("libopencv_highgui\n");
	dlopen("libopencv_highgui.so", RTLD_LAZY);
	printf("libstack\n");
	dlopen("libstack.so", RTLD_GLOBAL);
	printf("libopencv_lib\n");
	void * dll = dlopen("libopencv_lib.so", RTLD_LAZY);
#endif

#if 0
	#define RTLD_DEEPBIND	0x00008	/* Use deep binding.  */

	void * dll = dlopen("/home/kouryuu/gstreamer_lib/lib/libgstreamer_lib.so", RTLD_NOW | RTLD_GLOBAL | RTLD_DEEPBIND );
	//void * dll = dlopen("liborbweaver.so", RTLD_NOW | RTLD_GLOBAL | RTLD_DEEPBIND );
	int (*host)(int argc, char * argv[]) = (int (* )(int, char * []))dlsym(dll, "host_main");
	if(host != 0) {
		return host(argc, argv);
	}	
	return -1;
#else
	return host_main(argc, argv);
#endif
}
