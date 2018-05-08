#include "profiles.h"

int MPI_Init(int *argc, char ***argv) {

    int ret, myid;
    ret = PMPI_Init(argc, argv);

    MPE_Init_log();

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    if (myid == 0) {
        MPE_Describe_state(START_BCAST, END_BCAST, "Brodcast", "blue");
        MPE_Describe_state(START_SCAT, END_SCAT, "Scatter", "red");
        MPE_Describe_state(START_SCATV, END_SCATV, "Scatterv", "cyan");
        MPE_Describe_state(START_GATH, END_GATH, "Gather", "black");
        MPE_Describe_state(START_GATHV, END_GATHV, "Gatherv", "grey");
        MPE_Describe_state(START_ALLRED, END_ALLRED, "All Reduce", "white");
        MPE_Describe_state(START_RECV, END_RECV, "Recive", "white");
        MPE_Describe_state(START_SEND, END_SEND, "Send", "white");
    }

    MPE_Start_log();

    return ret;

}

int MPI_Finalize(void) {

    int ret;

    MPE_Finish_log("stats");

    ret = PMPI_Finalize();

    return ret;

}

int MPI_Bcast(void *buf, int count, MPI_Datatype datatype,
              int root, MPI_Comm comm) {
    int ret, myid;

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    MPE_Log_event(START_BCAST, myid, "Start Brodcast");

    ret = PMPI_Bcast(buf, count, datatype, root, comm);

    MPE_Log_event(END_BCAST, myid, "End Brodcast");

    return ret;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
             MPI_Comm comm) {
    int ret, myid;

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    MPE_Log_event(START_SEND, myid, "Start Send");

    ret = PMPI_Send(buf, count, datatype, dest, tag, comm);

    MPE_Log_event(END_SEND, myid, "End Send");

    return ret;
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
             MPI_Comm comm, MPI_Status *status) {
    int ret, myid;

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    MPE_Log_event(START_RECV, myid, "Start Brodcast");

    ret = PMPI_Recv(buf, count, datatype, source, tag, comm, status);

    MPE_Log_event(END_RECV, myid, "End Brodcast");

    return ret;
}


int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
    int ret, myid;

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    MPE_Log_event(START_ALLRED, myid, "Start Brodcast");

    ret = PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);

    MPE_Log_event(END_ALLRED, myid, "End Brodcast");

    return ret;
} 

int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
    int ret, myid;

	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	MPE_Log_event(START_SCAT, myid, "Start Scatter");
   
    ret = PMPI_Scatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);

	MPE_Log_event(END_SCAT, myid, "End Scatter");
   
    return ret; 
}

int MPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs, MPI_Datatype sendtype, 
                    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)
{ 
    int ret, myid;

	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	MPE_Log_event(START_SCATV, myid, "Start Scatterv");
   
    ret = PMPI_Scatterv(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm);

	MPE_Log_event(END_SCATV, myid, "End Scatterv");
   
    return ret; 
}

int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
               int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
    int ret, myid;

	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	MPE_Log_event(START_GATH, myid, "Start Gather");
   
    ret = PMPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);

	MPE_Log_event(END_GATH, myid, "End Gather");
   
    return ret; 
    
}

int MPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
                const int *recvcounts, const int *displs, MPI_Datatype recvtype, int root,
                MPI_Comm comm)
{
    int ret, myid;

	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	MPE_Log_event(START_GATHV, myid, "Start Gatherv");
   
    ret = PMPI_Gatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm);

	MPE_Log_event(END_GATHV, myid, "End Gatherv");
   
    return ret; 
    
}



