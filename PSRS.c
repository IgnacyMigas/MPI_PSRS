#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
//#include "mpi.h"
#include "profiles.h"
#include "utilities.h"


int main(int argc, char *argv[]) {

    int numprocs, myid, server;
    int i, j, ret;
    double startwtime = 0.0, endwtime;

    MPI_Comm world, workers;
    MPI_Group world_group, worker_group;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    world = MPI_COMM_WORLD;
    MPI_Comm_size(world, &numprocs);
    MPI_Comm_rank(world, &myid);
    server = 0;

    if (myid == server) {
        printf("\nStarting\n");
        printf("Number of processes %d\n", numprocs);
    }


    /**
     * PHASE I
     * Data preparation
     */
    int myDataSize = 60;

    FILE *ifp, *ofp;
    char *inFile;
    ifp = NULL;
    ofp = NULL;

    if (myid == server) {
        ofp = fopen("out.data", "w");

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

    // send data size of whole data to all processes
    MPI_Bcast(&myDataSize, 1, MPI_INT, server, world);

    int myData[myDataSize];

    if (myid == server) {
        // set values to sort
        // printf("Table values to sort:\n");
        if (ifp != NULL) {
            for (i = 0; i < myDataSize; i++) {
                ret = fscanf(ifp, "%d", &myData[i]);
                if (feof(ifp)) {
                    printf("ERROR in reading from file!\n");
                    return -1;
                }
                //   printf("%d ", myData[i]);
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

    int myDataLengths[numprocs];
    int myDataStarts[numprocs];

    // set data lengths and data start values of each process
    for (i = 0; i < numprocs; i++) {
        myDataLengths[i] = myDataSize / numprocs;
        myDataStarts[i] = i * (myDataSize / numprocs);
    }
    myDataLengths[numprocs - 1] += (myDataSize % numprocs);

    if (myid == server) {
        printf("Lengths of data to be send to processes...\n");

        for (i = 0; i < numprocs; i++) {
            printf("[process-%d] myDataLength=%d, myDataStarts=%d\n", i, myDataLengths[i], myDataStarts[i]);
        }
    }

    /**
     * PHASE II
     * Data gathering, pivot choosing
     */
    // scatter data to all processes
    if (myid == server) {
        MPI_Scatterv(myData, myDataLengths, myDataStarts, MPI_INT, MPI_IN_PLACE, myDataLengths[myid], MPI_INT, server,
                     world);
    } else {
        MPI_Scatterv(myData, myDataLengths, myDataStarts, MPI_INT, myData, myDataLengths[myid], MPI_INT, server,
                     world);
    }

    // processes quick sort each process own data
    qsort(myData, myDataLengths[myid], sizeof(int), compare_ints);

    //   printArrayAtOnce(myid, "Sorted data", myData, myDataLengths[myid]);

    int pivots[numprocs * numprocs];

    // process get pivots from data
    for (i = 0; i < numprocs; i++) {
        pivots[i] = myData[i * myDataLengths[myid] / numprocs];
    }

    // printArrayAtOnce(myid, "Pivot values", pivots, numprocs);

    // server gather all pivots
    if (myid == server) {
        MPI_Gather(MPI_IN_PLACE, numprocs, MPI_INT, pivots, numprocs, MPI_INT, server, world);
    } else {
        MPI_Gather(pivots, numprocs, MPI_INT, pivots, numprocs, MPI_INT, server, world);
    }


    /**
     * PHASE III
     * Server pivot merge, choose
     */
    // server merge pivots
    if (myid == server) {
        int *starts[numprocs];
        int lengths[numprocs];

        for (i = 0; i < numprocs; i++) {
            starts[i] = &pivots[i * numprocs];
            lengths[i] = numprocs;
        }
        int tmpBuffer[numprocs * numprocs];

        multimerge(starts, lengths, numprocs, tmpBuffer, numprocs * numprocs); // sorted list of pivots

        // regularly select numprocs - 1 of pivot candidates to broadcast as data partitions
        for (i = 0; i < numprocs - 1; i++) {
            pivots[i] = tmpBuffer[(i + 1) * numprocs];
        }

        //   printArrayAtOnce(myid, "Merged pivots", tmpBuffer, numprocs * numprocs);
        //   printArrayAtOnce(myid, "Chosen pivots", pivots, numprocs - 1);

    }

    // server broadcasts the partition values
    MPI_Bcast(pivots, numprocs - 1, MPI_INT, server, world);


    /**
     * PHASE IV
     * Data partitioning
     */
    int partitionsStart[numprocs];
    int partitionsLength[numprocs];

    // iterate over local data to partition data to classes
    int dataindex = 0;
    for (i = 0; i < numprocs - 1; i++) {
        partitionsStart[i] = dataindex;
        partitionsLength[i] = 0;

        // update data class index and length
        while ((dataindex < myDataLengths[myid]) && (myData[dataindex] <= pivots[i])) {
            ++partitionsLength[i];
            ++dataindex;
        }
        printf("[process-%d][class-%d] class start=%d, class length=%d\n", myid, i, partitionsStart[i],
               partitionsLength[i]);
    }
    // set Start and Length for last class
    partitionsStart[numprocs - 1] = dataindex;
    partitionsLength[numprocs - 1] = myDataLengths[myid] - dataindex;

    printf("[process-%d][class-%d] class start=%d, class length=%d\n", myid, numprocs - 1, partitionsStart[numprocs - 1],
           partitionsLength[numprocs - 1]);

    /**
     * PHASE V
     * Partitioned data gather, multi-merge
     */
    int newBuffer[myDataSize];
    int newLengths[numprocs];
    int newStarts[numprocs];

    // this process gets all sent partitioned data to corresponding process
    for (i = 0; i < numprocs; i++) {
        // each process gathers up data length of corresponding class from the other nodes
        MPI_Gather(&partitionsLength[i], 1, MPI_INT, newLengths, 1, MPI_INT, i, world);

        // on local process, calculate start positions of received data
        if (myid == i) {
            newStarts[0] = 0;
            for (j = 1; j < numprocs; j++) {
                newStarts[j] = newStarts[j - 1] + newLengths[j - 1];
            }
        }

        // each process gathers up all the members of corresponding class from the other nodes
        MPI_Gatherv(&myData[partitionsStart[i]], partitionsLength[i], MPI_INT, newBuffer, newLengths,
                    newStarts, MPI_INT, i, world);

    }

    // multimerge this single partition data
    int *newData[numprocs];
    for (i = 0; i < numprocs; i++) {
        newData[i] = newBuffer + newStarts[i];
    }

    multimerge(newData, newLengths, numprocs, myData, myDataSize);
    int newDataLength = newStarts[numprocs - 1] + newLengths[numprocs - 1];

    //  printArrayAtOnce(myid, "Data for myid partition", myData, newDataLength);

    /**
     * PHASE VI
     * Server sorted data from all processes merge
     */
    // root collects data
    int sendLengths[numprocs];
    int sendStarts[numprocs];

    // root processor gathers up the lengths of all the data to be gathered
    MPI_Gather(&newDataLength, 1, MPI_INT, sendLengths, 1, MPI_INT, server, world);

    // the root processor compute starts from lengths of classes to gather
    if (myid == server) {
        sendStarts[0] = 0;
        for (i = 1; i < numprocs; i++) {
            sendStarts[i] = sendStarts[i - 1] + sendLengths[i - 1];
        }
    }

    int sortedData[myDataSize];

    // gather sorted data to root
    MPI_Gatherv(myData, newDataLength, MPI_INT, sortedData, sendLengths, sendStarts, MPI_INT, server, world);

    if (myid == 0) {
        endwtime = MPI_Wtime();
        printf("Sorted data:\n");
        for (i = 0; i < myDataSize; i++) {
            //    printf("%d ", sortedData[i]);
            fprintf(ofp, "%d ", sortedData[i]);
        }
        printf("\nClock time (seconds) = %f\n", endwtime - startwtime);
        printf("\nThe end\n");
        fclose(ofp);
    }

    MPI_Finalize();
}


