#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Hello from Rank %d of %d\n", rank, size);
    
    MPI_Finalize();
    return 0;
}

/**
 * To setup the system
 *      On Ubuntu: sudo apt install openmpi-bin libopenmpi-dev
 *      On MacOS: brew install open-mpi
 * To compile: mpic++ hello_mpi.cpp -o hello_mpi.bin
 * To run
 *      Single process: ./hello_mpi.bin
 *      Multiple processes: mpirun -np 4 ./hello_mpi.bin
 */