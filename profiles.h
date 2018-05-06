#ifndef PROFILES_H
#define PROFILES_H

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "mpe.h"

#define START_BCAST 0
#define END_BCAST 1
#define START_ALLRED 2
#define END_ALLRED 3
#define START_RECV 4
#define END_RECV 5
#define START_SEND 6
#define END_SEND 7
#define START_SCAT 8
#define END_SCAT 9

int MPI_Init(int *argc, char ***argv);

int MPI_Finalize(void);

int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm);

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);

/*
int MPI_Scatterv(const void *sendbuf, int *sendcnts, int *displs, MPI_Datatype sendtype, 
                    void *recvbuf, int recvcnt, MPI_Datatype recvtype, int root, MPI_Comm comm)
{ 
    int ret, myid;

	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	MPE_Log_event(START_SCAT, myid, "Start Scatterv");
   
    ret = PMPI_Scatterv(sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt, recvtype, root, comm);

	MPE_Log_event(END_SCAT, myid, "End Scatterv");
   
    return ret; 
} */

#endif
