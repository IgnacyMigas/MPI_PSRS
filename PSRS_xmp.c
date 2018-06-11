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

    numprocs = xmp_num_nodes();
    myid = xmp_node_num();
    server = 0;

    printf("My id is %d\n", myid);

    #pragma xmp task on p[server]
    {
        printf("\nStarting\n");
        printf("Number of processes %d\n", numprocs);
    }
}