#include <stdio.h>
#include <stdlib.h>

static const char *get_format(){
	return "other secure format string\n";
}

int main(int argc, char **argv){
	(void) argc;
	printf("secure format string\n");
	printf(get_format());
	if(argv[1]){
		printf(argv[1], "insecure format string, the compiler should find this, too");
		printf("\n");
	}
	return 0;
}
