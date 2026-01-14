/*
 * File:    deadlock_sol_sendrecv.cpp
 *
 * Purpose: Solution 3 for MPI Deadlock - The Combined Call
 * Demonstrates the use of MPI_Sendrecv to perform a "swap" operation.
 * This function atomicly handles sending and receiving, ensuring that
 * internal buffering is managed correctly to prevent deadlock.
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

  // 2. The Solution Logic (Symmetric & Atomic)
  
  // Calculate the neighbor rank.
  // Rank 0 <-> Rank 1
  int neighbor = (rank == 0) ? 1 : 0;

  printf("[Rank %d] Exchanging data with Rank %d using Sendrecv...\n", rank, neighbor);

  // MPI_Sendrecv: The "All-in-One" solution.
  // It effectively says: "Send to 'dest' AND Receive from 'source' simultaneously."
  // We no longer need if/else blocks to order the calls manually.
  
  MPI_Sendrecv(
      send_buf.data(), N, MPI_INT, neighbor, 0,  // SEND params: buffer, count, type, dest, tag
      recv_buf.data(), N, MPI_INT, neighbor, 0,  // RECV params: buffer, count, type, src, tag
      MPI_COMM_WORLD, MPI_STATUS_IGNORE          // Communicator & Status
  );

  printf("[Rank %d] Exchange Complete. Received value: %d\n", rank, recv_buf[0]);

  MPI_Finalize();
  return 0;
}

/*
 * ============================================================
 * Compile & Run Instructions:
 * ============================================================
 * 1. Compile:
 * mpic++ deadlock_sol_sendrecv.cpp -o deadlock_sol_sendrecv.bin
 *
 * 2. Run (requires 2 slots):
 * mpirun -np 2 ./deadlock_sol_sendrecv.bin
 * ============================================================
 */