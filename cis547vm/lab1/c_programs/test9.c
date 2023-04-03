#include <stdio.h>

int main() {
  int sum = 0, len = 0;
  int inp;

  // min inp 0 
  // loop runs at least once 
  while ((inp = getchar()) && len < 20) {
    sum += inp;
    len++;
  }
  // if loop runs at least once, len is at least 1. thus, div by zero error never occurs. 

  int avg = sum / len;

  return 0;
}