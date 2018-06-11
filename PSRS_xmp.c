#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
//#include "profiles.h"
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

    int myDataSize = 60;

    FILE *ifp, *ofp;
    char *inFile;
    ifp = NULL;
    ofp = NULL;

    #pragma xmp task on p[server]
    {
//        ofp = fopen("out.data", "w");

        // set data length
        for (i = 0; i < argc; i++) {
            if (strcmp(argv[i], "-f") == 0) {
                inFile = argv[i + 1];
                ifp = fopen(inFile, "r");
                ret = fscanf(ifp, "%d", &myDataSize);
            }
        }
        printf("Size of whole data to process: %d\n", myDataSize);
    }

    #pragma xmp bcast (myDataSize)

    printf("My data dize: %d \n", myDataSize);
}