#include <stdio.h>

int main(int argc, char **argv)
{
    const char hexstring[] = "416E61", *pos = hexstring;
    unsigned char val[12];
    size_t count = 0;

    printf("String inicial %s\n", hexstring);
     /* WARNING: no sanitization or error-checking whatsoever */
    for(count = 0; count < sizeof(val)/sizeof(val[0]); count++) {
        sscanf(pos, "%2hhx", &val[count]);
        pos += 2;
    }

   // for(count = 0; count < sizeof(val)/sizeof(val[0]); count++)
        printf("%s", val);
    printf("\n");



    
    return(0);
}
