#include <stdio.h>
#include "FizzBuzz.h"

int main(void)
{
    printf("Hello, world!\n");
    
    char result[9];
    for (int i = 1; i <= 15; i++) {
        FizzBuzz(i, result);
        
        printf(
            "%s\n",
            result
        );
    }
    
    printf("Goodbye, world!\n");
    
    return 0;
}
