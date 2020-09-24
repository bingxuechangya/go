#include "common.h"
#include <unistd.h>
#include <stdlib.h>

int _czm_debug_in_loop_;
int _czm_debug_loop_counter_;

#ifdef VERBOSE
int verbose_flag = VERBOSE;
#else
int verbose_flag;
#endif

void *xmalloc(size_t size)
{
    void *ptr = NULL;
	while (ptr == NULL) {
		ptr = malloc(size);
		usleep(1);
	}
    return ptr;
}

void xfree(void *ptr)
{
    free(ptr);
    // czm_print("%s:%s:%lu: -%#lx\n", fileName, funcName, line, ptr);
}

