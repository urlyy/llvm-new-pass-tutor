#include<stdio.h>

void dead(){

}

void loop_print(){
    for (int i = 7; i*i < 1000; ++i){
        printf("%d\n",i);
    }
}

int main(){
    dead();
    loop_print();
    return 0;
}


