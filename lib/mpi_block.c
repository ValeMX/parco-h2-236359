#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "functions.h"

#define CODE "MB"

#define BLOCK_SIZE 2

MPI_Datatype block_type, block_send_type, block_partition_type, block_recv_type;

void matTransposeBlockMPI(double* M, double* T, double* blocks, int n, int rank, int size, double* t) {
    int chunk = n / size;
    int bs = chunk > BLOCK_SIZE ? BLOCK_SIZE : chunk;
    int total_blocks = n * n / (bs * bs);
    int blocks_per_process = total_blocks / size;
    int blocks_per_row = n / bs;
    int blocks_per_row_per_process = blocks_per_row / size;

    // Blocks partitioning
    for (int i = 0; i < blocks_per_row; i++) {
        MPI_Scatter(M + i * n * bs, blocks_per_row_per_process, block_send_type,
                    blocks + i * bs * blocks_per_row_per_process, blocks_per_row_per_process, block_partition_type,
                    0, MPI_COMM_WORLD);
    }

    double t1 = MPI_Wtime();

    // Blocks transposition
    for (int bn = 0; bn < blocks_per_process; bn++) {
        double* block = blocks + bn * bs;
        for (int i = 0; i < bs; i++) {
            for (int j = 0; j < i; j++) {
                double temp = block[i * bs * blocks_per_process + j];
                block[i * bs * blocks_per_process + j] = block[j * bs + i];
                block[j * bs + i] = temp;
            }
        }
    }

    double t2 = MPI_Wtime();
    *t += t2 - t1;

    // Blocks gathering
    for (int i = 0; i < blocks_per_row; i++) {
        MPI_Gather(blocks + i * bs * blocks_per_row_per_process, blocks_per_row_per_process, block_partition_type,
                   T + i * bs, blocks_per_row_per_process, block_recv_type,
                   0, MPI_COMM_WORLD);
    }
}

// The attribute is necessary to avoid the compiler optimization on the repetitions loops
int __attribute__((optimize("O0"))) main(int argc, char** argv) {
#ifndef MPI_VERSION
    printf("Error: compile with MPI support!\n");
    return -1;
#endif

    int dim = 0;
    int rep = 0;
    if (argc < 2) {
        printf("Correct usage: program-name [M dimension as exponent of 2] [number of repetitions (default 5)]\n");
        return 1;
    } else if (argc == 2) {
        dim = atoi(argv[1]);
        rep = 500;
    } else {
        dim = atoi(argv[1]);
        rep = atoi(argv[2]) > 0 ? atoi(argv[2]) : 500;
    }

    unsigned int n = pow(2, dim);

    // Setup MPI
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        printf("Compiled with MPI %d.%d\n", MPI_VERSION, MPI_SUBVERSION);
        printf("Matrix dimension: %d\n", n);
        printf("Repetitions: %d\n\n", rep);
    }

    if (n % size != 0) {
        if (rank == 0) {
            printf("Error: the matrix dimension must be divisible by the number of processes!\n");
        }

        MPI_Finalize();
        return -1;
    }

    // Variables declaration
    double ts1, te1, t1;  // temp time variables
    double t1m, t1e;      // execution times
    double* M;            // input matrix
    double* T;            // transposed matrix
    double* blocks;       // temporary blocks

    int bs = n / size > BLOCK_SIZE ? BLOCK_SIZE : n / size;
    int blocks_per_process = n * n / (bs * bs) / size;

    blocks = (double*)malloc(bs * bs * sizeof(double) * blocks_per_process);

    // Matrices allocation
    if (rank == 0) {
        if (initMatrices(&M, &T, n) == -1) {
            printf("Error in allocating matrices!\n\n");
            MPI_Finalize();
            return -1;
        }
    }

    // Derived data types definition
    MPI_Type_vector(bs, bs, n, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);

    MPI_Type_create_resized(block_type, 0, sizeof(double) * bs, &block_send_type);
    MPI_Type_commit(&block_send_type);

    MPI_Type_vector(bs, bs, bs * blocks_per_process, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);

    MPI_Type_create_resized(block_type, 0, sizeof(double) * bs, &block_partition_type);
    MPI_Type_commit(&block_partition_type);

    MPI_Type_vector(bs, bs, n, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);

    MPI_Type_create_resized(block_type, 0, sizeof(double) * bs * n, &block_recv_type);
    MPI_Type_commit(&block_recv_type);

    // Parallel execution
    MPI_Barrier(MPI_COMM_WORLD);

    ts1 = MPI_Wtime();
    for (int i = 0; i < rep; i++) matTransposeBlockMPI(M, T, blocks, n, rank, size, &t1);
    te1 = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);

    // Results printing and saving
    MPI_Reduce(&t1, &t1e, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        t1m = (te1 - ts1) / rep;
        t1e = t1 / rep;

        printf("Parallel execution (message passing included and excluded):\n");
        printf("matTransposeBlockMPI: %.9f\t%.9f seconds\n\n", t1m, t1e);

        testResultsBlock(M, T, n);

        if (saveResultsMPI(CODE, n, size, 0, 0, t1m, t1e) == -1) {
            printf("Error in saving results!\n\n");
        }
    }

    // Matrices deallocation
    if (M != NULL && rank == 0) free(M);
    if (T != NULL && rank == 0) free(T);
    if (blocks != NULL) free(blocks);

    // Derived data types deallocation
    MPI_Type_free(&block_type);
    MPI_Type_free(&block_send_type);
    MPI_Type_free(&block_partition_type);
    MPI_Type_free(&block_recv_type);

    MPI_Finalize();

    return 0;
}