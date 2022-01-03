#define _GNU_SOURCE
#include "mm.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

// #include <signal.h>

// Tamaño máximo de memoria disponible de la librería
#define MAX_HEAP 2 * 32 * 1024 * 1024  // 64 MiB (64*2^20)
// Nunca una región puede ser inferior a este tamaño
#define MIN_REGION_SIZE 32

#define MAGIC_NO 1234567

enum blk_type{SML_T, MED_T, BIG_T};

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

void *heap_start = NULL;
node_t *head = NULL;
int block_id_gen = 0;
int blk_mem_alocated = 0;

/* Funciones auxiliares*/

static size_t
max(size_t x, size_t y)
{
	return (x > y) ? x : y;
}

static size_t
min(size_t x, size_t y)
{
	return (x > y) ? y : x;
}

static node_t *
aloc_block(size_t size)
{
	if (size <= 0) {
		return NULL;
	}
	// MAP_ANON indica que no está backeado por un archivo
	// MAP_PRIVATE hace que se haga un mapeo con COW
	int block_size = 0;
	unsigned short block_type = -1;
	if (size <= BLOCK_SML) {
		block_size = BLOCK_SML;
		block_type = SML_T;
	} else if (size <= BLOCK_MED) {
		block_size = BLOCK_MED;
		block_type = MED_T;
	} else if (size <= BLOCK_BIG) {
		block_size = BLOCK_BIG;
		block_type = BIG_T;
	} else {
		return NULL;
	}
	if ((blk_mem_alocated+block_size) > MAX_HEAP) {
		return NULL;
	}
	node_t *new_block = mmap(NULL,
	            block_size,
	            PROT_READ | PROT_WRITE,
	            MAP_ANONYMOUS | MAP_PRIVATE,
	            -1,
	            0);
	if (new_block == MAP_FAILED) {
		exit(-1);
	}
	blk_mem_alocated += block_size;
	new_block->size = block_size - sizeof(header_t);
	new_block->type = block_type;
	new_block->id = block_id_gen;
	block_id_gen++;
	if (!heap_start) {
		heap_start = new_block;
	} else {
		heap_start = min((void *)heap_start, (void *)new_block);
	}
	if (!head) {
		head = new_block;
		head->next = NULL;
		head->anterior = NULL;
	} else {
		head->anterior = new_block;
		new_block->next = head;
		new_block->anterior = NULL; 
		head = new_block;
	}
	return new_block;
}

static node_t *
find_free_region(size_t size)
{
	node_t *free_region = NULL;
	node_t *iter = head;
	while (iter) {
		if (size <= iter->size) {
			return iter;
		}
		iter = iter->next;
	}
	free_region = aloc_block(size);
	return free_region;
}

/*
 * Itera la lista de regiones/nodos libres uniéndolas si son contiguas
 */

int check_empty_block(node_t *block) {
	if ((block->type == SML_T) && (block->size == (BLOCK_SML - sizeof(header_t)))) {
		return BLOCK_SML;
	} else if ((block->type == MED_T) && (block->size == (BLOCK_MED - sizeof(header_t)))) {
		return BLOCK_MED;
	} else if ((block->type == BIG_T) && (block->size == (BLOCK_BIG - sizeof(header_t)))) {
		return BLOCK_BIG;
	}
	return 0;
}

static void
coalesce()
{
	node_t *base_node = head;
	node_t *next_node = base_node->next;

	// caso borde: si uso todo un bloque y lo libero, me queda solo 1 nodo en la lista y no entra al loop
	{int size_to_unmap = check_empty_block(head);
	if (size_to_unmap) {
		munmap(head, size_to_unmap);
		blk_mem_alocated -= size_to_unmap;
		head = NULL;
	} }
	while (next_node) {
		if (base_node->type != next_node->type || base_node->id != next_node->id) {
			base_node = next_node;
			next_node = base_node->next;
			continue;
		}
		void *back_contiguous_reg =
		        (void *) next_node + sizeof(header_t) + next_node->size;
		void *forw_contiguous_reg =
		        (void *) base_node + sizeof(header_t) + base_node->size;
		if (back_contiguous_reg == (void *) base_node) {
			// Respecto de base, la region de atrás está libre.
			if (base_node == head) {
				// Mantego el head actualizado.
				head = next_node;
				head->anterior = NULL;
			} else {
				next_node->anterior = base_node->anterior;
				base_node->anterior->next = next_node;
			}
			next_node->size = next_node->size + sizeof(header_t) +
			                  base_node->size;
			memset(base_node, 0, sizeof(node_t));
			int size_to_unmap = check_empty_block(next_node);
			if (size_to_unmap) {
				// si el bloque esta vacio, lo tengo que borrar de la lista y liberar la memoria
				if (next_node == head) {
				// Mantego el head actualizado.
					head = next_node->next;
					head->anterior = NULL;
				} else {
					next_node->next->anterior = next_node->anterior;
					next_node->anterior->next = next_node->next;
				}
				base_node = next_node->next;
				next_node = next_node->next->next;
				// libero la memoria
				munmap(next_node, size_to_unmap);
				blk_mem_alocated -= size_to_unmap;
			}
		} else if (forw_contiguous_reg == next_node) {
			// Respecto de base, la region de adelante está libre
			base_node->size = base_node->size + sizeof(header_t) +
			                  next_node->size;
			if (next_node->next) {
				next_node->next->anterior = base_node;
			}
			base_node->next = next_node->next;
			memset(next_node, 0, sizeof(node_t));
			int size_to_unmap = check_empty_block(base_node);
			if (size_to_unmap) {
				// si el bloque esta vacio, lo tengo que borrar de la lista y liberar la memoria
				node_t *new_next = base_node->next;
				if (base_node == head) {
					head = new_next;
					if (new_next) {
						head->anterior = NULL;
					}
				} else {
					new_next->anterior = base_node->anterior;
					base_node->anterior->next = new_next;
				}
				munmap(base_node, size_to_unmap);
				blk_mem_alocated -= size_to_unmap;
				if (!new_next) {
					break;
				}
				base_node = new_next;
			}
			next_node = base_node->next;
			continue;
		}
		base_node = next_node;
		next_node = base_node->next;
	}
}


/* Implementación*/

void *
mm_alloc(size_t size)
{
	if (size <= 0) {
		return NULL;
	}
	size = max(size, MIN_REGION_SIZE);

	size_t total_size = size + sizeof(header_t);
	node_t *free_region = find_free_region(size);
	if (!free_region) {
		// No hay mas memoria libre, por ahora devuelvo NULL
		errno = ENOMEM;
		return NULL;
	}
	// Splitting
	if (!free_region->anterior) {
		// si el anterior es NULL estoy en head
		// entonces actualizo head para que apunte al siguiente nodo libre
		// que pasa si es NULL el siguiente? se arregla cuando se crea el nuevo nodo
		head = free_region->next;
	} else {
		// si no es NULL le paso el nuevo siguiente
		free_region->anterior->next = free_region->next;
	}
	if (free_region->next) {
		// si el siguiente no es NULL, le pongo el anterior del actual
		// Si el anterior es NULL, apunta a NULL porque es el head, que ya actualice previamente
		free_region->next->anterior = free_region->anterior;
	}
	free_region->size -= size;
	if (free_region->size) {
		// si le queda lugar a la seccion agrego un nuevo nodo a la lista
		node_t *new_region;
		new_region = (void *) free_region + total_size;
		new_region->id = free_region->id;
		new_region->type = free_region->type;
		new_region->size = free_region->size - sizeof(header_t);
		new_region->anterior = NULL;
		new_region->next = head;
		if (head) {
			head->anterior = new_region;
		}
		head = new_region;
	}
	// Seteo la metadata de la region allocada
	((header_t *) free_region)->size = size;
	((header_t *) free_region)->type = free_region->type;
	((header_t *) free_region)->id = free_region->id;
	((header_t *) free_region)->magic = MAGIC_NO;
	// Le devuelvo el puntero a la posicion donde empieza el payload
	return (void *) free_region + sizeof(header_t);
}

void
mm_free(void *ptr)
{
	if (!ptr) {
		return;
	}
	if (!head || ptr < heap_start ) {
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
	int id_old_region = region_to_free->id;
	int type_old_region = region_to_free->type;
	// creo que coin liberar el header solo alcanza
	memset(region_to_free, 0, size_region_to_free + sizeof(header_t));

	// Seteo valores del nuevo nodo libre y lo inserto a la lista
	new_free_node->size = size_region_to_free;
	new_free_node->id = id_old_region;
	new_free_node->type = type_old_region;
	new_free_node->next = head;
	new_free_node->anterior = NULL;
	if (head) {
		head->anterior = new_free_node;
	}
	head = new_free_node;
	coalesce();
	return;
}
void *
mm_calloc(size_t nmemb, size_t size)
{
	if (size <= 0) return NULL; 
	int result;
	if (__builtin_mul_overflow(nmemb, size, &result)) {
		return NULL;
	}
	void * ptr = mm_alloc(result);
	if (!ptr) return NULL;
	return ptr;
}

/* Test only*/
int
mm_initial_avail_space()
{
	// total - header del bloque allocado
	return BLOCK_SML - sizeof(header_t);
}

// Indica cuanto es la memoria disponible para allocar.
// Es decir, siempre puedo hacer mm_alloc(mm_cur_avail_space())
int
mm_cur_avail_space()
{
	int avail_free_space = 0;
	int count_nodes = 0;
	node_t *iter = head;
	while (iter) {
		avail_free_space += iter->size;
		iter = iter->next;
		count_nodes++;
	}
	return avail_free_space;
}

int
count_nodes() {
	node_t *iter = head;
	int cont = 0;
	while (iter) {
		cont++;
		iter = iter->next;
	}
	return cont;
}