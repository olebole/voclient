#include <stdio.h>

int main (int argc, char *argv[])
{
    char buf[1024];

    while (fgets (buf, 1024, stdin)) {
       printf ("fgets says '%s'\n", buf);
    }
    printf ("fgets gets EOF\n");
}
