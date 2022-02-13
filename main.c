#include <stdio.h>
#include "serial.h"

int main(int argc, char * args[]) {
    printf("%s: running with %d parameters\n", args[0], argc-1);
    for (int i=1; i<argc; i++) {
        printf("  %02d: [%s]\n", i, args[i]);
    }
    printf("%s: Message: [%s]\n", args[0], getMessage());
    return 0;
}