/*
 * File:    bcast_demo.cpp
 *
 * Purpose: Demonstrates the usage of MPI_Bcast (Broadcast).
 * Shows how to replicate data from a 'Root' process to all other
 * processes in the communicator.
 *
 * Key Concept: Collective Operations must be called by ALL processes
 * in the communicator, not just the sender.
 *
 * Author:  dzhao@uw.edu
 * Date:    2026-01-19
 * Course:  TCSS 558
 */

#include <mpi.h>
#include <cstdio>

int main(int argc, char** argv) {
    // 1. Initialize MPI Environment
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int secret_code;

    // 2. Data Preparation
    // Only the Root process (Rank 0) possesses the valid data initially.
    // Everyone else initializes their variable to a dummy value (0).
    if (rank == 0) {
        secret_code = 999;
        printf("[Rank 0] I have the secret code: %d. Broadcasting now...\n", secret_code);
    } else {
        secret_code = 0; 
    }

    // 3. The Collective Call (Broadcast)
    // CRITICAL: Every process in the communicator must call this function.
    // - If Rank == Root (0): It sends the data from its buffer.
    // - If Rank != Root    : It receives the data into its buffer.
    //
    // Note: The 'root' parameter (4th arg) must be the same for everyone.
    MPI_Bcast(&secret_code, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 4. Verification
    // At this point, everyone should have '999'.
    printf("[Rank %d] My secret code is now: %d\n", rank, secret_code);

    MPI_Finalize();
    return 0;
}

/*
 * ============================================================
 * Compile & Run Instructions:
 * ============================================================
 * 1. Compile:
 * mpic++ bcast_demo.cpp -o bcast_demo.bin
 *
 * 2. Run (Example with 4 processes):
 * mpirun -np 4 ./bcast_demo.bin
 * ============================================================
 */