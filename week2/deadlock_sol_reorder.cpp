/*
 * File:    deadlock_sol_reorder.cpp
 *
 * Purpose: Solution 1 for MPI Deadlock - Manual Reordering
 * Demonstrates how to avoid deadlock by ordering Send/Recv calls
 * asymmetrically between ranks.
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
  // Use a large buffer to force Rendezvous Protocol (skipping system buffers)
  const int N = 10000;
  std::vector<int> send_buf(N, rank + 1); // Rank 0 sends 1s, Rank 1 sends 2s
  std::vector<int> recv_buf(N, 0);

  // 2. The Solution Logic (Manual Reordering)
  // We break the circular dependency by ensuring one rank Recvs while the other Sends.
  
  if (rank == 0) {
    // Strategy for Rank 0: SEND then RECEIVE
    printf("[Rank 0] Sending to Rank 1...\n");
    MPI_Send(send_buf.data(), N, MPI_INT, 1, 0, MPI_COMM_WORLD);
    
    printf("[Rank 0] Waiting to Receive from Rank 1...\n");
    MPI_Recv(recv_buf.data(), N, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
  } else if (rank == 1) {
    // Strategy for Rank 1: RECEIVE then SEND
    // IMPORTANT: This RECV matches Rank 0's SEND immediately.
    printf("[Rank 1] Waiting to Receive from Rank 0...\n");
    MPI_Recv(recv_buf.data(), N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    printf("[Rank 1] Sending to Rank 0...\n");
    MPI_Send(send_buf.data(), N, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }

  printf("[Rank %d] Exchange Complete. Received value: %d\n", rank, recv_buf[0]);

  MPI_Finalize();
  return 0;
}

/*
 * ============================================================
 * Compile & Run Instructions:
 * ============================================================
 * 1. Compile:
 * mpic++ deadlock_sol_reorder.cpp -o deadlock_sol_reorder.bin
 *
 * 2. Run (requires 2 slots):
 * mpirun -np 2 ./deadlock_sol_reorder.bin
 * ============================================================
 */