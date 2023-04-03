#include <stdio.h>

int main() {
    // figuring out if maxP could ever equal 5 (if maxP = 5, div by zero error)

    // program fails if input is "00000", "00"

    char s[2000];
    fgets(s, sizeof(s), stdin);
    int maxP = 1, curP = 1;
    char curChar = s[0];

    // loop def runs at least once 
    for (int i = 1; i < 10; i++) {
        // might run; runs e.g., str="ee"; does not run if e.g., str="ab"
        if (s[i] == curChar) {
            curP++;
        } else {
            curChar = s[i];
            curP = 1;
        }
        if (maxP < curP)
            // could make maxP equal 5 if curP equals 5 
            maxP = curP;
    }
    printf("%d\n", maxP); 
    return curP / (maxP - 5);
}
