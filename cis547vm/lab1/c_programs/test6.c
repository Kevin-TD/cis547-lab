#include<stdio.h>

int main() {
    printf("running\n");

    // testing with input 6 already led to error 
    // note: if % 0 is executed, there will be an error 

    int K = getchar();

    char sum[65536];
    fgets(sum, sizeof(sum), stdin);

    printf("%s\n", sum); 

    // seems to get all but first char of input
    // if input = 0, sum is empty 

    for (int i = 1; i < 13; i++) {
        sum[i] = (sum[i-1] * 5) + 1;
        printf("should be fine\n");
    }
    for (int i = 12; i >= 0; i--) {
        // very possible for sum[i] to equal 0 
        // k % 0 results in div by zero 
        // loop will def run at least once (13 times exactly)
        // this program may lead to div by zero; e.g., one char input makes s[1 or more] equal zero -> div by zeor error 
        K = K % sum[i];
    }
    return 0;
}
