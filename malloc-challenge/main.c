#include <stdio.h>
#include <assert.h>
#include "mm.h"
#define RESET "\033[0m"
#define BLACK "\033[30m"  /* Black */
#define RED "\033[31m"    /* Red */
#define GREEN "\033[32m"  /* Green */
#define YELLOW "\033[33m" /* Yellow */

int
main()
{
	printf("Alloc 0: ");
	assert(mm_alloc(0) == NULL);
	printf(GREEN "passed\n" RESET);

	void *ptr;
	printf("Alloc 100: ");
	ptr = mm_alloc(100);
	assert(ptr != NULL);
	printf(GREEN "passed\n" RESET);

	return 0;
}
