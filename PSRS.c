#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "profiles.h"
#include "utilities.h"

/************************************************************/

int main(int argc, char *argv[]) {
    int numprocs, myid, server, ret;
    int i, index, classindex, iprocessor;
    double startwtime = 0.0, endwtime;
    MPI_Comm world, workers;
    MPI_Group world_group, worker_group;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    world = MPI_COMM_WORLD;
    MPI_Comm_size(world, &numprocs);
    MPI_Comm_rank(world, &myid);
    server = 0;

    // prepare data
    int myDataSize = 60;

    FILE *ifp, *ofp;
    char *inFile;
    ifp = NULL;
    ofp = NULL;

    if (myid == 0) {
        ofp = fopen("out.data", "w");

        for (i = 0; i < argc; i++) {
            if (strcmp(argv[i], "-f") == 0) {
                inFile = argv[i + 1];
                ifp = fopen(inFile, "r");
                ret = fscanf(ifp, "%d", &myDataSize);
            }
        }
    }
    MPI_Bcast(&myDataSize, 1, MPI_INT, 0, world);

    int myData[myDataSize];
    int myDataLengths[numprocs];
    int myDataStarts[numprocs];

    int pivotbuffer[numprocs * numprocs];
    int pivotbufferSize;

    for (i = 0; i < numprocs; i++) {
        myDataLengths[i] = myDataSize / numprocs;
        myDataStarts[i] = i * myDataSize / numprocs;
    }
    myDataLengths[numprocs - 1] += (myDataSize % numprocs);

    // set values to sort
    if (myid == 0) {
        printf("Tablica:\n");
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
            for (index = 0; index < myDataSize; index++) {
                myData[index] = rand() % MAX_RANDOM;
                printf("%d ", myData[index]);
            }
        }
        printf("\nKoniec\n");
        startwtime = MPI_Wtime();
    }

    // scatter data to all process
    if (myid == server) {
        MPI_Scatterv(myData, myDataLengths, myDataStarts, MPI_INT, MPI_IN_PLACE, myDataLengths[myid], MPI_INT, server,
                     world);
    } else {
        MPI_Scatterv(myData, myDataLengths, myDataStarts, MPI_INT, &myData, myDataLengths[myid], MPI_INT, server,
                     world);
    }

    // processes sort their own data
    qsort(myData, myDataLengths[myid], sizeof(int), compare_ints);

    // processes get pivots from data
    for (index = 0; index < numprocs; index++) {
        pivotbuffer[index] = myData[index * myDataLengths[myid] / numprocs];
    }

    // server gather all pivots
    if (myid == server) {
        MPI_Gather(MPI_IN_PLACE, numprocs, MPI_INT, pivotbuffer, numprocs, MPI_INT, server, world);
    } else {
        MPI_Gather(pivotbuffer, numprocs, MPI_INT, pivotbuffer, numprocs, MPI_INT, server, world);
    }

    // server merge
    if (myid == server) {
        // multimerge the numproc sorted lists into one
        int *starts[numprocs];  // array of lists
        int lengths[numprocs];  // array of lengths of lists
        for (i = 0; i < numprocs; i++) {
            starts[i] = &pivotbuffer[i * numprocs];
            lengths[i] = numprocs;
        }
        int tempbuffer[numprocs * numprocs];  // merged list
        multimerge(starts, lengths, numprocs, tempbuffer, numprocs * numprocs);

        // regularly select numprocs-1 of pivot candidates to broadcast
        // as partition pivot values for myData
        for (i = 0; i < numprocs - 1; i++) {
            pivotbuffer[i] = tempbuffer[(i + 1) * numprocs];
        }
    }

    // server broadcasts the partition values
    MPI_Bcast(pivotbuffer, numprocs - 1, MPI_INT, server, world);

    // phase IV
    int classStart[numprocs];
    int classLength[numprocs];

    // need for each processor to partition its list using the values
    // of pivotbuffer
    int dataindex = 0;
    for (classindex = 0; classindex < numprocs - 1; classindex++) {
        classStart[classindex] = dataindex;
        classLength[classindex] = 0;

        // as long as dataindex refers to data in the current class
        while ((dataindex < myDataLengths[myid]) && (myData[dataindex] <= pivotbuffer[classindex])) {
            ++classLength[classindex];
            ++dataindex;
        }
    }
    // set Start and Length for last class
    classStart[numprocs - 1] = dataindex;
    classLength[numprocs - 1] = myDataLengths[myid] - dataindex;


    // *******************************************
    //
    // PHASE V:  All ith classes are gathered by processor i
    int recvbuffer[myDataSize];    // buffer to hold all members of class i
    int recvLengths[numprocs];     // on myid, lengths of each myid^th class
    int recvStarts[numprocs];      // indices of where to start the store from 0, 1, ...

    // processor iprocessor functions as the root and gathers from the
    // other processors all of its sorted values in the iprocessor^th class.
    for (iprocessor = 0; iprocessor < numprocs; iprocessor++) {
        // Each processor, iprocessor gathers up the numproc lengths of the sorted
        // values in the iprocessor class
        MPI_Gather(&classLength[iprocessor], 1, MPI_INT, recvLengths, 1, MPI_INT, iprocessor, world);


        // From these lengths the myid^th class starts are computed on
        // processor myid
        if (myid == iprocessor) {
            recvStarts[0] = 0;
            for (i = 1; i < numprocs; i++) {
                recvStarts[i] = recvStarts[i - 1] + recvLengths[i - 1];
            }
        }

        // each iprocessor gathers up all the members of the iprocessor^th
        // classes from the other nodes
        MPI_Gatherv(&myData[classStart[iprocessor]], classLength[iprocessor], MPI_INT, recvbuffer, recvLengths,
                    recvStarts, MPI_INT, iprocessor, world);
    }


    // multimerge these numproc lists on each processor
    int *mmStarts[numprocs]; // array of list starts
    for (i = 0; i < numprocs; i++) {
        mmStarts[i] = recvbuffer + recvStarts[i];
    }
    multimerge(mmStarts, recvLengths, numprocs, myData, myDataSize);

    int mysendLength = recvStarts[numprocs - 1] + recvLengths[numprocs - 1];

    // Root collects data
    int sendLengths[numprocs]; // lengths of consolidated classes
    int sendStarts[numprocs];  // starting points of classes
    // Root processor gathers up the lengths of all the data to be gathered
    MPI_Gather(&mysendLength, 1, MPI_INT, sendLengths, 1, MPI_INT, server, world);

    // The root processor compute starts from lengths of classes to gather
    if (myid == server) {
        sendStarts[0] = 0;
        for (i = 1; i < numprocs; i++) {
            sendStarts[i] = sendStarts[i - 1] + sendLengths[i - 1];
        }
    }

    // gather sorted data to root
    int sortedData[myDataSize];
    MPI_Gatherv(myData, mysendLength, MPI_INT, sortedData, sendLengths, sendStarts, MPI_INT, server, world);
    // phase V

    if (myid == 0) {
        endwtime = MPI_Wtime();
        printf("\nClock time (seconds) = %f\n\n", endwtime-startwtime);
        printf("Sorted data:\n");
        for (index = 0; index < myDataSize; index++) {
            printf("%d ", sortedData[index]);
            fprintf(ofp, "%d ", sortedData[index]);
        }
        printf("\nKoniec\n");
        fclose(ofp);
    }

    MPI_Finalize();
}


