/*
 * File:    deadlock_sol_isend.cpp
 *
 * Purpose: Solution 2 for MPI Deadlock - Non-Blocking Operations
 * Demonstrates how to use MPI_Isend (Immediate Send) to initiate
 * a transfer without blocking, allowing the program to reach the
 * Receive call and break the circular dependency.
 *
 * Author:  dzhao@uw.edu
 * Date:    2026-01-14
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

  // Requirement: Exactly 2 processes for this demo
  if (size != 2) {
    if (rank == 0) {
      printf("Error: This program requires exactly 2 processes.\n");
    }
    MPI_Finalize();
    return 0;
  }

  // 1. Data Setup
  const int N = 10000;
  std::vector<int> send_buf(N, rank + 1);
  std::vector<int> recv_buf(N, 0);

  // 2. The Solution Logic (Non-Blocking)
  // Both ranks can attempt to "Send" first, but we use MPI_Isend.
  // This function returns IMMEDIATELY, creating a request handle.
  
  MPI_Request req;
  int dest = (rank == 0) ? 1 : 0;
  
  printf("[Rank %d] Initiating Isend to Rank %d...\n", rank, dest);
  
  // Step A: Fire and Forget (Start the Send)
  // We pass &req to track the status of this operation later.
  MPI_Isend(send_buf.data(), N, MPI_INT, dest, 0, MPI_COMM_WORLD, &req);

  // Step B: Receive
  // Since Isend returned immediately, we reach this line.
  // The deadlock is broken because both sides are now active in Recv
  // while the Send happens in the background.
  printf("[Rank %d] Entering Recv block...\n", rank);
  MPI_Recv(recv_buf.data(), N, MPI_INT, dest, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // Step C: Wait / Cleanup
  // CRITICAL: We must ensure the Send has actually finished before
  // we modify send_buf or exit.
  printf("[Rank %d] Waiting for Isend to complete...\n", rank);
  MPI_Wait(&req, MPI_STATUS_IGNORE);

  printf("[Rank %d] Exchange Complete. Received value: %d\n", rank, recv_buf[0]);

  MPI_Finalize();
  return 0;
}

/*
 * ============================================================
 * Compile & Run Instructions:
 * ============================================================
 * 1. Compile:
 * mpic++ deadlock_sol_isend.cpp -o deadlock_sol_isend.bin
 *
 * 2. Run (requires 2 slots):
 * mpirun -np 2 ./deadlock_sol_isend.bin
 * ============================================================
 */