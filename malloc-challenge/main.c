#ifdef LIB_MALLOC
#include "stdlib.h"
#else
#include "mm.h"
void *
malloc(size_t size)
{
	return mm_alloc(size);
}
void
free(void *ptr)
{
	mm_free(ptr);
}
#endif
#include <assert.h>

int
main(int argc, char const *argv[])
{
	// Se crea reserva memoria
	int *arr = malloc(100 * sizeof(int));
	for (size_t i = 0; i < 100; i++) {
		arr[i] = i + 1;
	}
	// Se utiliza la memoria
	int sum = 0;
	for (size_t i = 0; i < 100; i++) {
		sum += arr[i];
	}
	assert(sum == (100 * 101) / 2);
	// Se libera la memoria
	free(arr);

	// Se piden muchos bloques, se pueden usar y liberar todos
	int **pointers = malloc(30 * sizeof(int *));
	for (size_t i = 0; i < 30; i++) {
		pointers[i] = malloc(sizeof(int) * 100);
	}
	// Seteo cada arreglo con 1
	for (size_t i = 0; i < 30; i++) {
		int *arr = pointers[i];
		for (size_t j = 0; j < 100; j++) {
			arr[i] = 1;
		}
	}

	// Verifico uso
	for (size_t i = 0; i < 30; i++) {
		int *arr = pointers[i];
		int suma = 0;
		for (size_t j = 0; j < 100; j++) {
			suma += arr[i];
		}
		assert(suma == 100);
	}

	// Libero todos los bloques
	for (size_t i = 0; i < 30; i++) {
		free(pointers[i]);
	}
	free(pointers);

	// Probando si se hizo coalesce
	int *big_region = malloc(16000);
	assert(big_region != NULL);
	free(big_region);
	return 0;
}
