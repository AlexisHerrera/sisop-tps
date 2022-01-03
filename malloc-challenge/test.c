#include <stdio.h>
#include <assert.h>
 #include <string.h>
#include "mm.h"
#define RESET "\033[0m"
#define BLACK "\033[30m"  /* Black */
#define RED "\033[31m"    /* Red */
#define GREEN "\033[32m"  /* Green */
#define YELLOW "\033[33m" /* Yellow */

typedef struct __node_t {
	int size;
	char type;
	unsigned short id;
	struct __node_t *anterior;
	struct __node_t *next;
} node_t;

typedef struct __header_t {
	int size;
	char type;
	unsigned short id;
	int magic;
} header_t;

/* Single block tests*/
static void
test_basic_functionality()
{
	printf("Malloc test - Funcionalidad básica (single block)\n");
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

	printf("- Free Memory available: ");
	int free = mm_cur_avail_space();
	int calculated = BLOCK_SML - 100 - ((count_nodes()+1) * sizeof(header_t));
	// int calculated = 0;
	assert(free == calculated);
	printf(GREEN "OK\n" RESET);

	printf("- Memory can be freed: ");
	mm_free(ptr);
	free = mm_cur_avail_space();
	// calculated = BLOCK_SML - count_nodes() * 8;
	calculated = 0;
	assert(free == calculated);
	printf(GREEN "OK\n" RESET);
}

static void
test_coalesce()
{
	printf("Malloc test - Coalesce (single block)\n");
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
	ptr1 = mm_alloc(BLOCK_SML - sizeof(header_t));
	assert(ptr1 != NULL);
	printf(GREEN "OK\n" RESET);

	printf("- Memory can be freed: ");
	mm_free(ptr1);
	assert(initial_free_space == mm_cur_avail_space());
	printf(GREEN "OK\n" RESET);
}

static void
test_free()
{
	printf("Malloc test - invalid free and double free\n");
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
	int free = mm_cur_avail_space();
	int calculated = 0;
	assert(free == calculated);
	printf(GREEN "OK\n" RESET);
}

/* Multiblock tests*/
static void
test_basic_functionality_multiblock() {
	printf("Malloc test- Funcionalidad básica multibloque\n");
	void* ptr;
	int free;
	int calculated;
	
	printf("- allocated medium block: ");
	ptr = mm_alloc(BLOCK_SML + 100);
	assert(ptr != NULL);
	printf(GREEN "OK\n" RESET);

	printf("- Free Memory available: ");
	free = mm_cur_avail_space();
	calculated = BLOCK_MED - BLOCK_SML - 100 - ((count_nodes()+1) * sizeof(header_t));
	assert(free == calculated);
	printf(GREEN "OK\n" RESET);

	printf("- Memory can be freed: ");
	mm_free(ptr);
	free = mm_cur_avail_space();
	calculated = 0;
	assert(free == calculated);
	printf(GREEN "OK\n" RESET);

	printf("- allocated big block: ");
	ptr = mm_alloc(BLOCK_MED + 100);
	assert(ptr != NULL);
	printf(GREEN "OK\n" RESET);

	printf("- Free Memory available: ");
	free = mm_cur_avail_space();
	calculated = BLOCK_BIG - BLOCK_MED - 100 - ((count_nodes()+1) * sizeof(header_t));	
	assert(free == calculated);
	printf(GREEN "OK\n" RESET);

	printf("- Memory can be freed: ");
	mm_free(ptr);
	free = mm_cur_avail_space();
	calculated = 0;
	assert(free == calculated);
	printf(GREEN "OK\n" RESET);

	printf("- 10 allocs: ");
	void* array[10];
	for (int i = 0; i < 10; i++) {
		array[i] = mm_alloc(10);	
	}
	for (int i = 0; i < 10; i++) {
		mm_free(array[i]);	
	}
	free = mm_cur_avail_space();
	calculated = 0;
	assert(free == calculated);
	printf(GREEN "OK\n" RESET);
}

static void
test_coalesce_multiblock()
{
	printf("Malloc test - Coalesce (multi block)\n");
	void *ptr1, *ptr2;
	// Fuerzo un split de bloques que deben ser combinados
	// lo hago en un bloque grande, usando 2 allocs de tamanio mayor a bloque mediano
	// asi me aseguro que el bloque grande se divide
	int initial_free_space = mm_cur_avail_space();
	ptr1 = mm_alloc(BLOCK_MED + 100);
	ptr2 = mm_alloc(BLOCK_MED + 100);
	mm_free(ptr1);
	mm_free(ptr2);
	int final_free_space = mm_cur_avail_space();
	printf("- Does not lose memory after allocs: ");
	assert(initial_free_space == final_free_space);
	printf(GREEN "OK\n" RESET);

	printf("- Can allocate total avail_space: ");
	ptr1 = mm_alloc(BLOCK_BIG - sizeof(header_t));
	assert(ptr1 != NULL);
	printf(GREEN "OK\n" RESET);

	mm_free(ptr1);
	int free = mm_cur_avail_space();
	int calculated = 0;
	assert(free == calculated);
}

static void
test_malloc_edge_cases()
{
	printf("Malloc test- Casos borde\n");
	void *ptr;
	printf("- Should not allocate more than BIG block: ");
	ptr = mm_alloc(BLOCK_BIG+1);
	assert(ptr == NULL);
	printf(GREEN "OK\n" RESET);

	printf("- Really small alloc: ");
	ptr = mm_alloc(1);
	int free = mm_cur_avail_space();
	int calculated = BLOCK_SML - (32 + 2 * sizeof(header_t));
	assert(free == calculated);
	mm_free(ptr);
	free = mm_cur_avail_space();
	calculated = 0;
	assert(free == calculated);

	printf(GREEN "OK\n" RESET);

}


static void
test_calloc() {
	printf("Malloc test - calloc\n");
	printf("- small calloc: ");
	void* ptr = mm_calloc(4, 10);
	int free = mm_cur_avail_space();
	int calculated = BLOCK_SML - (40 + 2 * sizeof(header_t));
	assert(free == calculated);
	mm_free(ptr);
	free = mm_cur_avail_space();
	calculated = 0;
	assert(free == calculated);
	printf(GREEN "OK\n" RESET);

	printf("- BIG calloc: ");
	ptr = mm_calloc(1024, 2048);
	free = mm_cur_avail_space();
	calculated = BLOCK_BIG - (1024*2048 + 2 * sizeof(header_t));
	assert(free == calculated);
	mm_free(ptr);
	free = mm_cur_avail_space();
	calculated = 0;
	assert(free == calculated);
	printf(GREEN "OK\n" RESET);

	printf("- overflow DANGER: ");
	ptr = mm_calloc(__INT_MAX__, __INT_MAX__);
	assert(ptr == NULL);
	printf(GREEN "OK\n" RESET);

	printf("- memset to 0: ");
	static const char test_mem[100];
	char* casted_str = mm_calloc(100, sizeof(char));
	assert(casted_str != NULL);
	assert(memcmp(test_mem, casted_str, 100) == 0);
	printf(GREEN "OK\n" RESET);
}


int
main()
{
	// Single block
	test_basic_functionality();
	// test_coalesce();
	test_free();
	// Multibloque
	printf("\n - - - - - - MULTIBLOCK - - - - - - \n \n");
	test_basic_functionality_multiblock();
	test_coalesce_multiblock();
	test_malloc_edge_cases();
	test_calloc();
	return 0;
}
