#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 打印入参
void rt_posix_memalign(void **p, size_t align, size_t size) {
    printf("%p -- %p -- %zu -- %zu\n", p, *p, align, size);
}