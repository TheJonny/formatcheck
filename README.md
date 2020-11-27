# `LD_PRELOAD` based formatstring security checker

Author: Jonathan Krebs

License: CC0

I use this in CTFs to check if user input is used as printf/scanf format string. Checks if the `format` argument of the `printf` or `scanf` families is a writable memory mapping, and if so, prints the memory location and value.

usage:

```
LD_PRELOAD=$PWD/libformatcheck.so target_binary

# compile and test:
make
LD_PRELOAD=$PWD/libformatcheck.so ./demo thisstringwillbefound
# make demo will also call this
```
