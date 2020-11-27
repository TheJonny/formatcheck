CFLAGS += -std=c11 -D_XOPEN_SOURCE=700 -Wall -Wextra -Wconversion -Wmissing-prototypes -Werror
CC ?= cc

.PHONY: all clean demo
all: libformatcheck.so printf_demo

clean:
	rm -f *.so *.o printf_demo

demo: libformatcheck.so printf_demo
	LD_PRELOAD=`pwd`/libformatcheck.so ./printf_demo insecure_string_will_be_found

libformatcheck.so: formatcheck.c
	$(CC) $(CFLAGS) --shared -fPIC -o $@ $^ -ldl

printf_demo: printf_demo.c
	$(CC) $(CFLAGS) -o $@ $<
