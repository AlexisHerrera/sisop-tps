#include <stdio.h>
#include <assert.h>
#include "mm.h"
#define RESET "\033[0m"
#define BLACK "\033[30m"  /* Black */
#define RED "\033[31m"    /* Red */
#define GREEN "\033[32m"  /* Green */
#define YELLOW "\033[33m" /* Yellow */


static void
test_basic_functionality()
{
	printf("Malloc test 1 - Funcionalidad básica\n");
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

	printf("- Alocated medium block: ");
	ptr = mm_alloc(BLOCK_SML + 100);
	assert(ptr != NULL);
	printf(GREEN "OK\n" RESET);
}

static void
test_coalesce()
{
	printf("Malloc test 2 - Coalesce\n");
	void *ptr1, *ptr2;
	// Fuerzo un split de bloques que deben ser combinados
	// (si bien el test 1 ya lo hace, es para no depender de el)
	int initial_free_space = mm_cur_avail_space();
	ptr1 = mm_alloc(100);
	ptr2 = mm_alloc(100);
	mm_free(ptr1);
	mm_free(ptr2);
	int final_free_space = mm_cur_avail_space();
	printf("- Does not lose memory after allocs: ");
	assert(initial_free_space == final_free_space);
	printf(GREEN "OK\n" RESET);

	printf("- Can allocate total avail_space: ");
	ptr1 = mm_alloc(mm_cur_avail_space());
	assert(ptr1 != NULL);
	printf(GREEN "OK\n" RESET);

	mm_free(ptr1);
	assert(mm_cur_avail_space() == mm_initial_avail_space());
}

static void
test_malloc_edge_cases()
{
	// Coalesce must be implemented
	printf("Malloc test 3 - Casos borde\n");
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

	mm_free(ptr);
	assert(mm_cur_avail_space() == mm_initial_avail_space());
}

static void
test_free()
{
	printf("Malloc test 4 - invalid free and double free\n");
	printf("- Free null: ");
	mm_free(NULL);
	printf(GREEN "OK\n" RESET);

	printf("- Free invalid memory: ");
	mm_free((void *) 0xabcdef);
	printf(GREEN "OK\n" RESET);

	printf("- Double free: ");
	void *ptr = mm_alloc(100);
	mm_free(ptr);
	mm_free(ptr);
	printf(GREEN "OK\n" RESET);
	assert(mm_cur_avail_space() == mm_initial_avail_space());
}

int
main()
{
	test_basic_functionality();
	test_coalesce();
	test_malloc_edge_cases();
	test_free();
	return 0;
}
