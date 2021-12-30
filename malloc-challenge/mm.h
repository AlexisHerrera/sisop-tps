#include <stddef.h>
void *mm_alloc(size_t size);
void mm_free(void *ptr);
void *mm_calloc(size_t nmemb, size_t size);