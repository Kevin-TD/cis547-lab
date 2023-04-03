#include <stdio.h>
#include <string.h>

int main() {
    int a = 0;
    char s[20];
    fgets(s, sizeof(s), stdin);
    // gets up to 20 chars from input and puts it in s 

    int l = 0;
    int r = strnlen(s, 20) - 1;
    // strnlen() returns the number of characters in the string s, not including the terminating \0 character, but at most maxlen. In doing this, strnlen() looks only at the first maxlen characters at s and never beyond s + maxlen.
    printf("length %d\n", r); 
    
    // here, length min 0 
    // expr could run 
    if (r < 5) {
        a = 1;
    }

    // l = 0 
    // r minimum is zero 
    // 0 < 0 -> false -> loop does not execute 
    // 0 < (1 or more) -> true -> loop executes  
    while (l < r) {
        printf("pre: %d %d\n", l, r);
        // s[l++]; l min is 0 
        // s[r--]; r min is 1 (could NOT be 0)
        // 0 != 1 -> true 
        // s[l++] != s[r--] always runs as we have 0 != (1 or more) at least running once 
        // thus, a = 1 and no div by zero error happens 
        if (s[l++] != s[r--]) {
            // l++: l is returned then incremented. same logic for r--
            a = 1;
        }
        printf("post: %d %d\n", l, r);
    }

    printf("%d %d %d\n", s[0], s[1], s[2]);

    // in the case the former loop did not run (0 < 0), there are no chars input, so s[...] = 0. s[2] != s[1] + 4 evals to 0 != 4, which is true. thus, a = 1 and no div by zero happens in that case 
    // in the case the former loop did run, well we don't have to worry if this executes since we know a will be set to 1 in that case, and that's the only property we care about here
    if (s[2] != s[1] + 4) {
        a = 1;
    }

    // this program has no div by zero errors 

    int b = 1 / a;

    return 0;
}
