#include <stdio.h>

int main() {
    int k = 0;
    int c = (getchar() - 'W') % 10;
    // can very well have a div by zero if i < c
    // e.g., getchar input of g makes c > 0; for loop runs and we'll have k += 100 / 0 

    for (int i = 0; i < c; i++) {
        for (int j = i; j < c; j++) {
            k += 100 / (j - i);
        }
    }
    return 0;
}