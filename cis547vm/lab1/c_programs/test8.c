#include <stdio.h>

int main() {
    int a = 0;
    int b = 0;
    int c = getchar(); // min 0, max 127 (i think if it's doing ascii conversion)
    printf("%d\n", c);
    int d;
    // runs at least once 
    do {
        a = 1;
        b = c;
        // could run 
        if (c < 50) {
            a = 0;
            c++;
        }
    } while (b != c); // runs if c < 50 

    // if c >= 50, a will def be set to 1 and there will be no div by zero error 
    // c will definitely reach this point:      c < 50 (c = 50) -> c++, c is now 51. loop runs again. a is set to 1, 51 < 50 is false, and the loop ends. a is still 1, thus, no div by zero error. 

    d = 1 / a;
    return 0;
}