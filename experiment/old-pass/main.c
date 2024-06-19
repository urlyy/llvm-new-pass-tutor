#include <stdio.h>
#include <stdlib.h>

void sum(int a,int b,int c,int d){
    
}


int main(int argc, const char** argv) {
    char *s=NULL;
    printf("LLVM Test posix_memalign\n");
    // 这个函数的作用大致是分配一块内存
    posix_memalign((void **)&s, 16, 1024);
    printf("====== Finish ======\n");
    return 0;
}