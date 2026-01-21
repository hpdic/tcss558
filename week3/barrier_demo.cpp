/*
 * File:    barrier_demo.cpp
 *
 * Purpose: Demonstrates MPI_Barrier for synchronization.
 * Scenario:
 * 1. Processes sleep for random amounts of time (simulating uneven workload).
 * 2. They hit a Barrier.
 * 3. They proceed together only after everyone has arrived.
 *
 * Author:  dzhao@uw.edu
 * Date:    2026-01-21
 * Course:  TCSS 558
 */

#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h> // For sleep()

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 1. Simulate Uneven Workload
    int sleep_time = rank * 2 + 1; // Rank 0 sleeps 1s, Rank 1 sleeps 3s...
    
    printf("[Rank %d] I am working hard for %d seconds...\n", rank, sleep_time);
    sleep(sleep_time); // Simulate computation
    
    printf("[Rank %d] Work done. Waiting at the Barrier...\n", rank);

    // 2. The Synchronization Point
    // Faster ranks (like Rank 0) will BLOCK here until the slowest rank arrives.
    MPI_Barrier(MPI_COMM_WORLD);

    // 3. Post-Barrier Execution
    // This line prints roughly at the same time for everyone.
    printf("[Rank %d] I passed the Barrier! Moving to next phase.\n", rank);

    MPI_Finalize();
    return 0;
}

/*
 * ============================================================
 * Compile & Run Instructions:
 * ============================================================
 * 1. Compile:
 * mpic++ barrier_demo.cpp -o barrier_demo.bin
 *
 * 2. Run:
 * mpirun -np 4 ./barrier_demo.bin
 *
 * Observation:
 * You will see "Work done" messages appear one by one.
 * Then, after a pause, all "Passed the Barrier" messages appear together.
 * ============================================================
 */