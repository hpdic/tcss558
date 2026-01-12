/*
 * File: mpi_cluster_test.cpp
 * Path: tcss558/week2/mpi_cluster_test.cpp
 * Description: A simple MPI program to test cluster setup by printing
 *              the processor name each rank is running on.
 * Author: Dongfang Zhao
 * Email: dzhao@uw.edu
 */
#include <cstdio>
#include <mpi.h>

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  MPI_Barrier(MPI_COMM_WORLD);

  printf("[Rank %d] Hello! I am running on physical node: %s\n", world_rank,
         processor_name);

  MPI_Finalize();
  return 0;
}
/**
 * To setup the system
 *      On Ubuntu: sudo apt install openmpi-bin libopenmpi-dev build-essential
 *      On MacOS: brew install open-mpi
 *      IMPORTANT: Make sure passwordless SSH is setup among all cluster nodes.
 *
 * To compile:
 *      mpic++ mpi_cluster_test.cpp -o ~/mpi_cluster_test.bin
 *      scp ~/mpi_cluster_test.bin cc@192.5.86.160:~/
 *
 * To run:
mpirun --hostfile hosts \
       --mca btl_tcp_if_include 10.140.0.0/16 \
       --mca oob_tcp_if_include 10.140.0.0/16 \
       ~/mpi_cluster_test.bin

 * Example output:
[Rank 0] Hello! I am running on physical node: rtx6000
[Rank 1] Hello! I am running on physical node: rtx6000
[Rank 2] Hello! I am running on physical node: uc-nvme
[Rank 3] Hello! I am running on physical node: uc-nvme
 */