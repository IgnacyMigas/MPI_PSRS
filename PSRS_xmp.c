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

    #pragma xmp template t[myDataSize]
    #pragma xmp distribute t[cyclic] onto p

    #pragma xmp bcast (myDataSize)

    printf("[%d] Whole data to process size: %d \n", myid, myDataSize);

    #pragma xmp task on p[server]
    {
        // set values to sort
        printf("[%d] Table values to sort:\n", myid);
        if (ifp != NULL) {
            for (i = 0; i < myDataSize; i++) {
                ret = fscanf(ifp, "%d", &myData[i]);
                if (feof(ifp)) {
                    printf("ERROR in reading from file!\n");
                    return -1;
                }
                printf("%d ", myData[i]);
            }
            fclose(ifp);
        } else {
            srand(time(NULL));
            for (i = 0; i < myDataSize; i++) {
                myData[i] = rand() % MAX_RANDOM;
                printf("%d ", myData[i]);
            }
        }
        printf("\nThe end\n");
        startwtime = MPI_Wtime();
    }
}