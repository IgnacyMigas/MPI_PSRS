#include <stdio.h>
#include <stdlib.h>
//#include <math.h>
#include <time.h>
#include <string.h>
//#include "profiles.h"
#include "utilities.h"
#include <xmp.h>

#pragma xmp nodes p[*]

#define N 500000
#define MAX_PROCS 40

int myData[N]:[*];
int sortedData[N]:[*];
int pivots[MAX_PROCS * MAX_PROCS]:[*];

int partitionsStart[MAX_PROCS]:[*];
int partitionsLength[MAX_PROCS]:[*];
int tmpstart;
int tmplength;

int newDataLength:[*];
int dataLength;

int newBuffer[N]:[*];
int newLengths[MAX_PROCS]:[*];

// TODO: KOMENTARZE do dodania + maly refactoring
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
        printf("Number of processes %d\n\n", numprocs);
    }

    int myDataSize = N;

    FILE *ifp, *ofp;
    char *inFile;
    ifp = NULL;
    ofp = NULL;

#pragma xmp task on p[server]
    {
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

#pragma xmp bcast (myDataSize)
#pragma xmp barrier

#pragma xmp template nodes_t[numprocs]
#pragma xmp distribute nodes_t[cyclic] onto p

#pragma xmp template t[myDataSize]
#pragma xmp distribute t[block] onto p


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
//#pragma xmp barrier
    
#pragma xmp loop on nodes_t[i]
    for (i = 0; i < numprocs; i++) {
        myData[0:myDataLengths]= myData[myDataStarts:myDataLengths]:[server];
    }

//#pragma xmp barrier

    // processes quick sort each process own data
    qsort(myData, myDataLengths, sizeof(int), compare_ints);
	printArrayAtOnce(myid, "myData", myData, myDataLengths);

// #pragma xmp barrier

    for (i = 0; i < numprocs; i++) {
        pivots[i] = myData[i * myDataLengths / numprocs];
    }
    printArrayAtOnce(myid, "pivots", pivots, numprocs);

#pragma xmp barrier
    
   if(xmpc_this_image() == 0) {
	for (i = 0; i < numprocs; i++) {
		pivots[i*numprocs:numprocs] = pivots[0:numprocs]:[i];
	}
    printArrayAtOnce(myid, "server pivots", pivots, numprocs * numprocs);  
   }
   xmp_sync_all(NULL);


#pragma xmp task on p[server]
    {
	int *starts[numprocs];
        int lengths[numprocs];

        for (i = 0; i < numprocs; i++) {
            starts[i] = &pivots[i * numprocs];
            lengths[i] = numprocs;
        }
        int tmpBuffer[numprocs * numprocs];

        multimerge(starts, lengths, numprocs, tmpBuffer, numprocs * numprocs);

        for (i = 0; i < numprocs - 1; i++) {
            pivots[i] = tmpBuffer[(i + 1) * numprocs];
        }

	printArrayAtOnce(myid, "Merged pivots", tmpBuffer, numprocs * numprocs);
	printArrayAtOnce(myid, "Chosen pivots", pivots, numprocs - 1);

        for (i = 0; i < numprocs; i++) {
            pivots[0:numprocs-1]:[i] = pivots[0:numprocs-1];
        }
    }

#pragma xmp barrier

    printArrayAtOnce(myid, "Chosen pivots on node", pivots, numprocs - 1);

    // iterate over local data to partition data to classes
    int dataindex = 0;
    for (i = 0; i < numprocs - 1; i++) {
        partitionsStart[i] = dataindex;
        partitionsLength[i] = 0;

        // update data class index and length
        while ((dataindex < myDataLengths) && (myData[dataindex] <= pivots[i])) {
            ++partitionsLength[i];
            ++dataindex;
        }
        printf("[process-%d][class-%d] class start=%d, class length=%d\n", myid, i, partitionsStart[i],
               partitionsLength[i]);
    }
// set Start and Length for last class
    partitionsStart[numprocs - 1] = dataindex;
    partitionsLength[numprocs - 1] = myDataLengths - dataindex;

    printf("[process-%d][class-%d] class start=%d, class length=%d\n", myid, numprocs - 1, partitionsStart[numprocs - 1],
           partitionsLength[numprocs - 1]);

#pragma xmp barrier

    int newStarts[numprocs];

    // this process gets all sent partitioned data to corresponding process
    for (i = 0; i < numprocs; i++) {
        // each process gathers up data length of corresponding class from the other nodes
        //MPI_Gather(&partitionsLength[i], 1, MPI_INT, newLengths, 1, MPI_INT, i, world);
		newLengths[i] = partitionsLength[myid]:[i];
    }	
    // on local process, calculate start positions of received data
    newStarts[0] = 0;
    for (j = 1; j < numprocs; j++) {
		newStarts[j] = newStarts[j - 1] + newLengths[j - 1];
    }

//#pragma xmp barrier

    // each process gathers up all the members of corresponding class from the other nodes
	for (i = 0; i < numprocs; i++) {		
		tmpstart=partitionsStart[myid]:[i];
		tmplength=partitionsLength[myid]:[i];
		int tmpnewstart = newStarts[i];
		int tmpnewlength = newLengths[i];
		printf("[%d] tmp: %d %d | new: %d %d \n", myid, tmpstart, tmplength, tmpnewstart, tmpnewlength);
		newBuffer[tmpnewstart:tmpnewlength] = myData[tmpstart:tmplength]:[i];     
    }

#pragma xmp barrier

    int *newData[numprocs];
    for (i = 0; i < numprocs; i++) {
        newData[i] = newBuffer + newStarts[i];
    }
    multimerge(newData, newLengths, numprocs, myData, myDataSize);

	newDataLength = newStarts[numprocs - 1] + newLengths[numprocs - 1];
	printArrayAtOnce(myid, "Data for myid partition", myData, newDataLength);

#pragma xmp barrier

#pragma xmp task on p[server] 
	{
		int sendStart = 0;
		for (i = 0; i < numprocs; i++) {
			dataLength = newDataLength:[i];
			sortedData[sendStart:dataLength] = myData[0:dataLength]:[i];
			sendStart += dataLength;
		}
		endwtime = MPI_Wtime();

		printArrayAtOnce(myid, "sorted data", sortedData, myDataSize);

		fprintf(ofp, "%d\n", myDataSize);
	 	for (i = 0; i < myDataSize; i++) 
			fprintf(ofp, "%d ", sortedData[i]);

		printf("\nClock time (seconds) = %f\n", endwtime - startwtime);
		printf("\nThe end\n");
		fclose(ofp);
	}

	return 0;
}
