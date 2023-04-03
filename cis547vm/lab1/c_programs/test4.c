#include <stdio.h>

int main() {
    int k = 0;
    int c = (getchar() - 'W') % 10;
    for (int i = 0; i < c; i++) {
        for (int j = i + 1; j < c; j++) {
            k += 100 / ((j - i) + 10);
            // j is at least 1 
            // i shall always be less than j 
            // thus, j-i shall always be greater than 0 and a div by zero error will not occur 
            // same as test3.c
            // also the + 10 and the fact that (j - i) >= 0 ensures no div by zero error 
        }
    }
    return 0;
}