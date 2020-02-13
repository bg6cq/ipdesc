#include <stdio.h>
#include <string.h>

int debug = 0;

#include "shmcount.c"


main()
{
	initshm(0);
	printf("%lu\n", get_lookup_count());
}

