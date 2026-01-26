/*
 * File:    deadlock_sol_irecv.cpp
 *
 * Purpose: Solution 4 for MPI Deadlock - Non-Blocking Receive
 * Demonstrates the "Post Receive First" pattern. By initiating
 * MPI_Irecv before the blocking Send, we ensure that a matching
 * receive is always available, breaking the circular dependency.
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

  // 2. The Solution Logic (Non-Blocking Receive)
  // Instead of modifying the Send, we modify the Receive side.
  
  MPI_Request req;
  int dest = 1 - rank;
  
  printf("[Rank %d] Posting Irecv (Listening for Rank %d)...\n", rank, dest);
  
  // Step A: Initiate the Receive Early
  // "Post" the receive buffer to the system. This returns IMMEDIATELY.
  // It tells MPI: "If data comes from 'dest', put it here."
  MPI_Irecv(recv_buf.data(), N, MPI_INT, dest, 0, MPI_COMM_WORLD, &req);

  // Step B: Blocking Send
  // Now we can safely call the blocking Send.
  // Even if both ranks reach this line simultaneously, the transfers will
  // succeed because the Receives (Step A) are already active in the background.
  printf("[Rank %d] Calling Blocking Send...\n", rank);
  MPI_Send(send_buf.data(), N, MPI_INT, dest, 0, MPI_COMM_WORLD);

  // Step C: Wait for Receive Completion
  // Although Send is done, we don't know if 'recv_buf' is full yet.
  // We must Wait on the request handle from Step A.
  printf("[Rank %d] Waiting for Irecv to complete...\n", rank);
  MPI_Wait(&req, MPI_STATUS_IGNORE);

  // Step D: Verify
  // Now it is safe to read recv_buf
  printf("[Rank %d] Exchange Complete. Received value: %d\n", rank, recv_buf[0]);

  MPI_Finalize();
  return 0;
}

/*
 * ============================================================
 * Compile & Run Instructions:
 * ============================================================
 * 1. Compile:
 * mpic++ deadlock_sol_irecv.cpp -o deadlock_sol_irecv.bin
 *
 * 2. Run (requires 2 slots):
 * mpirun -np 2 ./deadlock_sol_irecv.bin
 * ============================================================
 */