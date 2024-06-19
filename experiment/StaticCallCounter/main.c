#include<stdio.h>
void foo() { }
void bar() {foo(); }
void fez() {bar(); }

int main() {
  foo();
  bar();
  fez();
  printf("1234");
  int ii = 0;
  for (ii = 0; ii < 10; ii++)
    foo();

  return 0;
}