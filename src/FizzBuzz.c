#include <stdio.h>
#include "FizzBuzz.h"

void FizzBuzz(char n, char* output)
{
    char
        three = n % 3 == 0,
        five = n % 5 == 0
    ;
    
    if (three || five) {
        sprintf(
            output,
            "%s%s",
            (three ? "Fizz" : ""),
            (five ? "Buzz" : "")
        );
    } else {
        sprintf(output, "%d", n);
    }
}
