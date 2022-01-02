#include <stddef.h>

#define BLOCK_SML 16 * 1024
#define BLOCK_MED 1024 * 1024
#define BLOCK_BIG 32 * 1024 * 1024

void *mm_alloc(size_t size);
void mm_free(void *ptr);
void *mm_calloc(size_t nmemb, size_t size);

/*Test only*/
int mm_initial_avail_space();
int mm_cur_avail_space();
int count_nodes();