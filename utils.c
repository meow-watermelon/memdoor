#include <stdio.h>
#include <time.h>
#include "utils.h"

void print_current_time() {
    time_t current_time;

    /* acquire current time */
    current_time = time(NULL);

    /* convert time_t type time data to string */
    fprintf(stdout, "Report Time: %s", ctime(&current_time));
    fflush(stdout);

    return;
}
