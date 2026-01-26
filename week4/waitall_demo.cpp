/*
 * File:    async_waitall_demo.cpp
 *
 * Purpose: Demonstrates using MPI_Isend and MPI_Irecv together with MPI_Waitall.
 * This exemplifies how to handle multiple non-blocking operations efficiently.
 *
 * Scenario:
 * Rank 0 and Rank 1 exchange integers simultaneously (Full Duplex).
 * Instead of waiting one by one, we use an array of requests and Waitall.
 *
 * Author:  dzhao@uw.edu
 * Date:    2026-01-26
 * Course:  TCSS 558
 */

#include <mpi.h>
#include <cstdio>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Safety check: We need at least 2 processes
    if (size < 2) {
        if (rank == 0) {
            printf("Error: Run with at least 2 processes.\n");
        }
        MPI_Finalize();
        return 0;
    }

    // 1. Setup Data and Neighbors
    // We only demonstrate exchange between Rank 0 and Rank 1
    if (rank > 1) {
        MPI_Finalize();
        return 0;
    }

    int send_val = rank + 1000; // Rank 0 sends 1000, Rank 1 sends 1001
    int recv_val = 0;
    int partner = 1 - rank; // 0 <-> 1

    // 2. Prepare the Request Array
    // We have 2 asynchronous operations:
    // - One for Receiving data (reqs[0])
    // - One for Sending data (reqs[1])
    MPI_Request reqs[2];

    // 3. Post the Receive (Non-blocking)
    // Best Practice: Always post Irecv before Isend to ensure buffer is ready.
    // We store the handle in reqs[0].
    printf("[Rank %d] Posting Irecv from Rank %d...\n", rank, partner);
    MPI_Irecv(&recv_val, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, &reqs[0]);

    // 4. Post the Send (Non-blocking)
    // We store the handle in reqs[1].
    printf("[Rank %d] Posting Isend to Rank %d...\n", rank, partner);
    MPI_Isend(&send_val, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, &reqs[1]);

    // 5. The "Overlap" Zone
    // At this point, the network controller (DMA) is handling the transfer.
    // The CPU is free to do other work here without blocking.
    printf("[Rank %d] Operations active. CPU is doing other work...\n", rank);
    // do_heavy_computation(); 

    // 6. Synchronization with MPI_Waitall
    // Instead of calling MPI_Wait twice, we wait for the entire array.
    // The program blocks here until BOTH Send and Recv are finished.
    printf("[Rank %d] Waiting for all requests to complete...\n", rank);
    MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

    // 7. Verify Results
    printf("[Rank %d] Exchange Done. Sent: %d, Received: %d\n", rank, send_val, recv_val);

    MPI_Finalize();
    return 0;
}

/*
 * ============================================================
 * Compile & Run Instructions:
 * ============================================================
 * 1. Compile:
 * mpic++ waitall_demo.cpp -o waitall_demo.bin
 *
 * 2. Run (requires 2 slots):
 * mpirun -np 2 ./waitall_demo.bin
 * ============================================================
 */