/*
 * Author: Dongfang Zhao
 * Email: dzhao@uw.edu
 */

#include <mpi.h>
#include <vector>
#include <iostream>
#include <cstdio> // For printf

int main(int argc, char **argv)
{
    // 1. Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // 2. Get the number of processes (Size)
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // 3. Get the rank of the process (ID)
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // 4. Get the name of the processor (Hostname)
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // 5. Print the message
    // Use printf to avoid output interleaving issues (more atomic than cout)
    printf("Hello World from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);

    // 6. Finalize the MPI environment
    MPI_Finalize();
    return 0;
}

/**
 * To setup the system
 *      On Ubuntu: sudo apt install openmpi-bin libopenmpi-dev build-essential
 *      On MacOS: brew install open-mpi
 * 
 * To compile: mpic++ proc_name.cpp -o proc_name.bin
 * 
 * To run
 *      Single process: ./proc_name.bin
 *      Multiple processes: mpirun -np 4 ./proc_name.bin
 */