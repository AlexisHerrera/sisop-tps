#define _GNU_SOURCE
#include "mm.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

// #include <signal.h>

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
	int magic;
} header_t;

void *heap_start = NULL;
node_t *head = NULL;

/* Funciones auxiliares*/
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
/*
 * Itera la lista de regiones/nodos libres uniéndolas si son contiguas
 */

static void
coalesce()
{
	node_t *base_node = head;
	node_t *next_node = base_node->next;

	while (next_node != NULL) {
		void *back_contiguous_reg =
		        (void *) next_node + sizeof(node_t) + next_node->size;
		void *forw_contiguous_reg =
		        (void *) base_node + sizeof(node_t) + base_node->size;
		if (back_contiguous_reg == (void *) base_node) {
			// Respecto de base, la region de atrás está libre.
			if (base_node == head) {
				// Mantego el head actualizado.
				head = next_node;
			}
			next_node->size = next_node->size + sizeof(node_t) +
			                  base_node->size;
			memset(base_node, 0, sizeof(node_t));
		} else if (forw_contiguous_reg == next_node) {
			// Respecto de base, la region de adelante está libre
			base_node->size = base_node->size + sizeof(node_t) +
			                  next_node->size;
			base_node->next = next_node->next;
			memset(next_node, 0, sizeof(node_t));
			next_node = base_node->next;
			continue;
		}
		base_node = next_node;
		next_node = base_node->next;
	}
}

static size_t
max(size_t x, size_t y)
{
	return (x > y) ? x : y;
}

/* Implementación*/

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
	size = max(size, MIN_REGION_SIZE);

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
	if (ptr == NULL) {
		return;
	}
	if (heap_start == NULL || ptr < heap_start ||
	    ptr > (heap_start + BLOCK_SIZE)) {
		// raise(SIGSEGV);
		return;
	}

	header_t *region_to_free = (header_t *) (ptr - sizeof(header_t));
	if (region_to_free->magic != MAGIC_NO) {
		// Se intento liberar una region inválida
		return;
	}
	node_t *new_free_node = (node_t *) region_to_free;
	// Limpio toda la region. Me guardo el tamaño antes de borrar todo
	int size_region_to_free = region_to_free->size;
	memset(region_to_free, 0, size_region_to_free + sizeof(header_t));

	// Seteo valores del nuevo nodo libre y lo inserto a la lista
	new_free_node->size =
	        size_region_to_free + sizeof(header_t) - sizeof(node_t);
	new_free_node->next = head;
	head = new_free_node;

	coalesce();
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

// Indica cuanto es la memoria disponible para allocar.
// Es decir, siempre puedo hacer mm_alloc(mm_cur_avail_space())
int
mm_cur_avail_space()
{
	int avail_free_space = 0;
	int count_nodes = 0;
	if (head == NULL) {
		return mm_initial_avail_space();  // 16360
	}
	node_t *iter = head;
	while (iter != NULL) {
		// fprintf(stderr, "nodo %d tiene %d bytes \n", count_nodes, iter->size);
		avail_free_space += iter->size;
		iter = iter->next;
		count_nodes++;
	}
	if (count_nodes == 1) {
		avail_free_space -= sizeof(header_t);  // 16368-8 = 16360
	}
	return avail_free_space;
}