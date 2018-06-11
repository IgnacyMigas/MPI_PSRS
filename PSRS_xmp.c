#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "profiles.h"
#include "utilities.h"
#include <xmp.h>

#pragma xmp nodes p[*]


int main(int argc, char *argv[]) {

    int numprocs, myid, server;
    int i, j, ret;
    double startwtime = 0.0, endwtime;

    server = 0;

    #pragma xmp task on p[0]
    {
        printf("\nStarting\n");
        printf("Number of processes %d\n", numprocs);
    }
}