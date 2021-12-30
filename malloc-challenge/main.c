#include <stdio.h>
#include <assert.h>
#include "mm.h"
#define RESET "\033[0m"
#define BLACK "\033[30m"  /* Black */
#define RED "\033[31m"    /* Red */
#define GREEN "\033[32m"  /* Green */
#define YELLOW "\033[33m" /* Yellow */

void
test_basic_functionality()
{
	printf("- Alloc 0: ");
	assert(mm_alloc(0) == NULL);
	printf(GREEN "OK\n" RESET);

	void *ptr;
	printf("- Alloc 100: ");
	ptr = mm_alloc(100);
	assert(ptr != NULL);
	printf(GREEN "OK\n" RESET);

	printf("- Allocated memory can be used: ");
	char *casted_string = ptr;
	for (size_t i = 0; i < 100; i++) {
		casted_string[i] = 'a';
	}
	for (size_t i = 0; i < 100; i++) {
		assert(casted_string[i] == 'a');
	}
	printf(GREEN "OK\n" RESET);

	printf("- Memory can be freed: ");
	mm_free(ptr);
	assert(mm_cur_avail_space() == mm_initial_avail_space());
	printf(GREEN "OK\n" RESET);
}

void
test_malloc_edge_cases()
{
	void *ptr;
	printf("- Should not allocate more than available space (%d bytes): ",
	       mm_initial_avail_space());
	ptr = mm_alloc(mm_initial_avail_space() + 1);
	assert(ptr == NULL);
	printf(GREEN "OK\n" RESET);

	printf("- Can allocate all memory available (%d bytes): ",
	       mm_initial_avail_space());
	ptr = mm_alloc(mm_initial_avail_space());
	assert(ptr != NULL);
	printf(GREEN "OK\n" RESET);
}

int
main()
{
	printf("Malloc test 1 - Funcionalidad básica\n");
	test_basic_functionality();

	return 0;
}
