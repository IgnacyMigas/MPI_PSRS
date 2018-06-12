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

    int numprocs, myid, lastproc, server;
    int i, j, ret;
    double startwtime = 0.0, endwtime;

    numprocs = xmp_num_nodes();
    myid = xmpc_node_num();
    lastproc = numprocs - 1;
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
    #pragma xmp distribute t[block] onto p

    #pragma xmp template nodes_t[numprocs]
    #pragma xmp distribute nodes_t[cyclic] onto p

    #pragma xmp bcast (myDataSize)

    printf("[%d] Whole data to process size: %d \n", myid, myDataSize);

    int myData[myDataSize]:[*];
    // #pragma xmp align myData[i] with t[i]

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
    #pragma xmp barrier

    int myDataLengths;
    int myDataStarts;

    #pragma xmp loop on nodes_t[i]
    for (i = 0; i < numprocs; i++) {
        myDataLengths = myDataSize / numprocs;
        myDataStarts = i * (myDataSize / numprocs);
    }
    #pragma xmp task on p[lastproc]
    {
        myDataLengths += (myDataSize % numprocs);
    }

    #pragma xmp loop on nodes_t[i]
    for (i = 0; i < numprocs; i++) {
        printf("[process-%d] myDataLength=%d, myDataStarts=%d\n", i, myDataLengths, myDataStarts);
    }

    #pragma xmp barrier

    int myPartData[myDataLengths];

    #pragma xmp loop on nodes_t[i]
    for (i = 0; i < numprocs; i++) {
        for (j = myDataStarts; j < myDataStarts + myDataLengths; j++){
            int tmp = 0;
            myPartData[tmp] = myData[j]:[server];
            tmp++;
        }
    }
    xmpc_sync_all(NULL);

    /*
    #pragma xmp task on p[server]
    {
        int dataLength;
        int dataStart;
        for (i = 0; i < numprocs - 1; i++) {
            dataLength = myDataSize / numprocs;
            dataStart = i * (myDataSize / numprocs);
            for (j = 0; j < dataLength; j++) {
                myPartData[j] = myData[dataStart + j];
            }
            #pragma bcast (myPartData) on p[i]
        }
        dataLength = myDataSize / numprocs;// + (myDataSize % numprocs);
        dataStart = i * (myDataSize / numprocs);
        for (j = 0; j < dataLength; j++) {
            myPartData[j] = myData[dataStart + j];
        }
        #pragma bcast (myPartData) on p[lastproc]
    }*/

    #pragma xmp loop on t[i]
    for (i = 0; i < myDataLengths; i++) {
        printf("[%d] %d \n", myid, myPartData[i]);
    }
}