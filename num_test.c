#include <stdio.h>
#include <stdint.h>
 
int main(int argc, char** argv)
{   //char *num = NULL;
    uint8_t num; 
    //num = calloc(6000, 1);
    num = 192;
    printf("%u\n", num);
    printf("%u\n", sizeof(num));
    //printf("%u\n", num[1]);
}