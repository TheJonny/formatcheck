// LD_PRELOAD-library to find writable scanf/printf format strings
// license: public domain

// information about writable memory locations is taken from  /proc/self/maps

// compile test: cc -std=c99 -D_GNU_SOURCE -DFORMATCHECK_TEST -Wall -Wextra formatcheck.c -ldl -o formatcheck_test
// compile lib: cc -std=c99 -D_GNU_SOURCE -Wall -Wextra formatcheck.c -ldl -fPIC -shared -o libformatcheck.so
//     _GNU_SOURCE is for dlsym(RTLD_NEXT, ...) to load wrapped function
// use lib: env LD_PRELOAD=$PWD/libformatcheck.so $SHELL

#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



static int is_mapping_writable(const void *ptr){
	FILE *maps = fopen("/proc/self/maps", "r");
	int res = -1;
	if(maps == NULL) return -1;
	for(;;){
		size_t a, b;
		char flags[5];
		if(fscanf(maps, "%zx-%zx %4s", &a, &b, flags) != 3) break;
		if (a <= (size_t)ptr  && (size_t)ptr < b){
			res = flags[1] == 'w';
		}
		for(;;){
			int c = fgetc(maps);
			if(c == '\n' || c == EOF) break;
		}
	}
	fclose(maps);
	return res;
}

static void check(const char *format, const char *fname){
	int mw = is_mapping_writable(format);
	if(mw == -1){
		fprintf(stderr, "FORMATCHECK: ERROR could not determine if %p is writable\n", format);
	}
	else if(mw == 1){
		fprintf(stderr, "FORMATCHECK: ATTENTION: %s format string at %p is writable!\n", fname, format);
		fprintf(stderr, "FORMATCHECK:    VALUE: \"%s\"\n", format);
	}

}

// a guard to avoid recursion
static __thread bool already_running = false;

// printing
//  vfprintf and vsprintf do the check and use dlsym to call original function
//  others are wrappers around vfprintf and vsprintf

int vfprintf(FILE *stream, const char *format, va_list ap){
	if(!already_running){
		already_running = true;
		check(format, "printf");
		already_running = false;
	}

	int (*vfprintf_ori)(FILE *, const char*, va_list) = dlsym(RTLD_NEXT, "vfprintf");
	return vfprintf_ori(stream, format, ap);
}

int fprintf(FILE *stream, const char *format, ...){
	va_list ap;
	va_start(ap, format);
	int ret = vfprintf(stream, format, ap);
	va_end(ap);
	return ret;
}

int printf(const char *format, ...){
	va_list ap;
	va_start(ap, format);
	int ret = vfprintf(stdout, format, ap);
	va_end(ap);
	return ret;
}

int vprintf(const char *format, va_list ap){
	return vfprintf(stdout, format, ap);
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap){
	FILE *stream = fmemopen(str, size, "w");
	int ret = vfprintf(stream, format, ap);
	fclose(stream);
	return ret;
}

int snprintf(char *str, size_t size, const char *format, ...){
	va_list ap;
	va_start(ap, format);
	int ret = vsnprintf(str, size, format, ap);
	va_end(ap);
	return ret;
}

// sprintf can not be trivially realized with fmemopen + fprintf
//   because of unbounded output size
//   open_memstream + strcpy would be possible but copies all output
int vsprintf(char *str, const char *format, va_list ap){
	if(!already_running){
		already_running = true;
		check(format, "printf");
		already_running = false;
	}

	int (*vsprintf_ori)(char *, const char*, va_list) = dlsym(RTLD_NEXT, "vsprintf");
	return vsprintf_ori(str, format, ap);
}

int sprintf(char *str, const char *format, ...){
	va_list ap;
	va_start(ap, format);
	int ret = vsprintf(str, format, ap);
	va_end(ap);
	return ret;
}


// Scanning
// vfscanf and vsscanf do check and dlsym

int vfscanf(FILE *stream, const char *format, va_list ap){
	if(!already_running){
		already_running = true;
		check(format, "scanf");
		already_running = false;
	}

	int (*vfscanf_ori)(FILE *, const char*, va_list) = dlsym(RTLD_NEXT, "vfscanf");
	return vfscanf_ori(stream, format, ap);
}

int fscanf(FILE *stream, const char *format, ...){
	va_list ap;
	va_start(ap, format);
	int ret = vfscanf(stream, format, ap);
	va_end(ap);
	return ret;
}

int vscanf(const char *format, va_list ap){
	return fscanf(stdin, format, ap);
}

int scanf(const char *format, ...){
	va_list ap;
	va_start(ap, format);
	int ret = vfscanf(stdin, format, ap);
	va_end(ap);
	return ret;
}

int vsscanf(const char *str, const char *format, va_list ap){
	if(!already_running){
		already_running = true;
		check(format, "scanf");
		already_running = false;
	}

	int (*vsscanf_ori)(const char *, const char *, va_list) = dlsym(RTLD_NEXT, "vsscanf");
	return vsscanf_ori(str, format, ap);
}

int sscanf(const char *str, const char *format, ...){
	va_list ap;
	va_start(ap, format);
	int ret = vsscanf(str, format, ap);
	va_end(ap);
	return ret;
}

#ifdef FORMATCHECK_TEST
int main(){
	printf("%d. safe!\n", 1);
	char format[] = "%d. unsafe\n";
	printf(format, 2);

	char sformat[4], buf[4];
	sprintf(sformat, "%%s");
	sscanf(format, sformat, buf);
	return 0;
}
#endif
