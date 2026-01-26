/*
 * File:    wait_test.cpp
 * Purpose: Demonstrates MPI_Test for polling.
 * 
 * Author:  dzhao@uw.edu
 */

#include <mpi.h>
#include <cstdio>
#include <unistd.h> // For sleep

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        // ------------------------------------------------
        // Receiver Side (The Poller)
        // ------------------------------------------------
        int recv_buf = 0;
        MPI_Request req;
        int flag = 0; // 0 = False, 1 = True

        printf("[Rank 0] Posting Irecv...\n");
        MPI_Irecv(&recv_buf, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &req);

        // Polling Loop
        int work_counter = 0;
        while (!flag) {
            // 1. Check status (Instant return)
            MPI_Test(&req, &flag, MPI_STATUS_IGNORE);

            if (!flag) {
                // 2. If not ready, do useful work
                printf("[Rank 0] Data not ready. Doing local work step %d...\n", ++work_counter);
                sleep(1); // Simulate computation
            }
        }

        // When we exit loop, flag is 1, meaning transfer is done.
        printf("[Rank 0] Data arrived! Received value: %d\n", recv_buf);

    } else if (rank == 1) {
        // ------------------------------------------------
        // Sender Side (The Slow Worker)
        // ------------------------------------------------
        int send_buf = 999;
        
        // Simulate a delay so Rank 0 has to wait
        printf("[Rank 1] Sleeping for 3 seconds before sending...\n");
        sleep(3);

        printf("[Rank 1] Sending now...\n");
        MPI_Send(&send_buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}

/**
 * To compile: mpic++ -o wait_test.bin wait_test.cpp
 * To run: mpirun -np 2 ./wait_test.bin
 */
