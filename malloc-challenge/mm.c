#define _GNU_SOURCE
#include "mm.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
// Tamaño máximo de memoria disponible de la librería
#define MAX_HEAP 32 * 1024 * 1024  // 32 MiB (32*2^20)
// Nunca una región puede ser inferior a este tamaño
#define MIN_REGION_SIZE 32
// Tamaño del primer bloque que se pide con mmap
#define BLOCK_SIZE 16 * 1024  // 16 KiB

#define MAGIC_NO 1234567

typedef struct __node_t {
	int size;
	struct __node_t *next;
} node_t;

typedef struct __header_t {
	int size;
	int magic;  // No es necesario, pero lo dejo por conformidad con la bibliografía
} header_t;

void *heap_start = NULL;
node_t *head = NULL;

static void
mm_init()
{
	// MAP_ANON indica que no está backeado por un archivo
	// MAP_PRIVATE hace que se haga un mapeo con COW
	head = mmap(NULL,
	            BLOCK_SIZE,
	            PROT_READ | PROT_WRITE,
	            MAP_ANONYMOUS | MAP_PRIVATE,
	            -1,
	            0);
	if (head == MAP_FAILED) {
		exit(-1);
	}
	// Para mantener el inicio del heap
	heap_start = head;
	// Se marca metadata del primer bloque
	head->size = BLOCK_SIZE - sizeof(node_t);
	head->next = NULL;
}

static node_t *
find_free_region(size_t size)
{
	node_t *free_region = NULL;
	node_t *iter = head;
	while (iter != NULL) {
		if (size <= iter->size) {
			// Se encontró el bloque
			free_region = iter;
			break;
		}
		iter = iter->next;
	}
	return free_region;
}

void *
mm_alloc(size_t size)
{
	if (size <= 0) {
		return NULL;
	}
	// Primer alloc, se hace mmap
	if (heap_start == NULL) {
		mm_init();
	}
	size_t total_size = size + sizeof(header_t);
	node_t *free_region = find_free_region(total_size);
	if (free_region == NULL) {
		// No hay mas memoria libre, por ahora devuelvo NULL
		errno = ENOMEM;
		return NULL;
	}
	// Splitting
	// Corro el head total_size posiciones
	head = (void *) free_region + total_size;
	// Actualizo metadata del head tras split
	head->size = free_region->size - total_size;
	head->next = free_region->next;
	// Seteo la metadata de la region allocada
	((header_t *) free_region)->size = size;
	((header_t *) free_region)->magic = MAGIC_NO;
	// Le devuelvo el puntero a la posicion donde empieza el payload
	return (void *) free_region + sizeof(header_t);
}
void
mm_free(void *ptr)
{
	return;
}
void *
mm_calloc(size_t nmemb, size_t size)
{
	return NULL;
}

/* Test only*/
int
mm_initial_avail_space()
{
	// total - header del bloque allocado
	return BLOCK_SIZE - sizeof(header_t) - sizeof(node_t);
}