/*
 * File:    reduce_demo.cpp
 *
 * Purpose: Demonstrates MPI_Reduce for parallel reduction.
 * Shows how to compute global Sum and Max from distributed values.
 *
 * Scenario:
 * 1. Each rank generates a local value.
 * 2. MPI_Reduce sums all values to Rank 0.
 * 3. MPI_Reduce finds the maximum value to Rank 0.
 *
 * Author:  dzhao@uw.edu
 * Date:    2026-01-21
 * Course:  TCSS 558
 */

#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

int main(int argc, char** argv) {
    // 1. Initialize
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 2. Data Preparation
    // Each rank picks a random number between 0 and 99.
    srand(time(NULL) + rank); 
    int local_val = rand() % 100;
    
    printf("[Rank %d] My local value: %d\n", rank, local_val);

    // Buffers for results (Only significant at Root)
    int global_sum = 0;
    int global_max = 0;

    // 3. Reduction 1: Summation
    // Combines 'local_val' from everyone using MPI_SUM.
    // Result is stored in 'global_sum' only at Root (Rank 0).
    MPI_Reduce(
        &local_val,     // Send buffer (Input)
        &global_sum,    // Recv buffer (Output at Root)
        1,              // Count
        MPI_INT,        // Datatype
        MPI_SUM,        // Operation
        0,              // Root rank
        MPI_COMM_WORLD  // Communicator
    );

    // 4. Reduction 2: Maximum
    // Finds the largest 'local_val' using MPI_MAX.
    MPI_Reduce(
        &local_val,     // Send buffer
        &global_max,    // Recv buffer
        1,              // Count
        MPI_INT,        // Datatype
        MPI_MAX,        // Operation
        0,              // Root rank
        MPI_COMM_WORLD  // Communicator
    );

    // 5. Verification (Rank 0 Only)
    if (rank == 0) {
        printf("--------------------------------\n");
        printf("[Rank 0] Total Sum: %d\n", global_sum);
        printf("[Rank 0] Global Max: %d\n", global_max);
        printf("--------------------------------\n");
    }

    MPI_Finalize();
    return 0;
}

/*
 * ============================================================
 * Compile & Run Instructions:
 * ============================================================
 * 1. Compile:
 * mpic++ reduce_demo.cpp -o reduce_demo.bin
 *
 * 2. Run (Example with 4 processes):
 * mpirun -np 4 ./reduce_demo.bin
 * ============================================================
 */