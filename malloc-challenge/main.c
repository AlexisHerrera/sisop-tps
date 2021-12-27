#include <stdio.h>
#include <assert.h>
#include "mm.h"
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
int main() {
    assert(mm_alloc(0) == NULL);
    printf("Alloc 0: " GREEN "passed\n" RESET);
    mm_free(NULL);
    printf("free null: " GREEN "passed\n" RESET);
    assert(mm_calloc(0, 0) == NULL);
    printf("Calloc 0,0 : " GREEN "passed\n" RESET);
    return 0;
}
