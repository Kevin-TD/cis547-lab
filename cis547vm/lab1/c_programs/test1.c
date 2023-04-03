#include <stdio.h>

int main() {
    // awaits user input; convers char to string 
    // could be div by zero error if bad user input 
    // because of this we should reject(?)

    printf("Insert two chars: "); 

    int a = getchar() - '0';
    int b = getchar() - '0';

    printf("Nums: %d %d\n", a, b);

    b *= a;
    a = b / a;
    return 0;
}