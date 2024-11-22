#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int alloc_size = 1024 * 1024;
    char *p;
    int c = 0;
    pid_t pid = getpid();

    fprintf(stderr, "PID: %d\n", pid);

    while (1) {
        p = (char *)malloc(alloc_size);

        // it won't fail due to overcommitment :-)
        if (p == NULL) {
            fprintf(stdout, "allocated %d MB memory\n", c);
            break;
        }

        // set the allocated memory to 0
        memset(p, 0, alloc_size);

        ++c;
    }

    return 0;
}
