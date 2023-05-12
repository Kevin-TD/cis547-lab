#include <stdio.h>

int main() {
  int a = 0;
  int b = 1;
  int *p = &a; // p = 0  
  int *q = &b; // q = 1

  *p = *q; // p = a = 1 

  int s = b / *p; // 1 / 1 
  return 0;
}
