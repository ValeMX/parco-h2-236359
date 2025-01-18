#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "functions.h"

#define CODE "M"

bool checkSymMPI(double* M, int n, int rank, int size, double* t) {
    int chunk = n / size;
    bool local_check = true;
    bool check = true;

    MPI_Bcast(M, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double t1 = MPI_Wtime();

    for (int i = rank * chunk; i < (rank + 1) * chunk; i++) {
        for (int j = 0; j < i; j++) {
            if (fabs(M[i * n + j] - M[j * n + i]) > EPSILON) {
                local_check = false;
            }
        }
    }

    double t2 = MPI_Wtime();
    *t += t2 - t1;

    MPI_Reduce(&local_check, &check, 1, MPI_C_BOOL, MPI_LAND, 0, MPI_COMM_WORLD);

    return check;
}

void matTransposeMPI(double* M, double* T, double* temp, int n, int rank, int size, double* t) {
    int chunk = n / size;

    MPI_Bcast(M, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double t1 = MPI_Wtime();

    for (int i = 0; i < chunk; i++) {
        for (int j = 0; j < n; j++) {
            temp[j * chunk + i] = M[n * (rank * chunk + i) + j];
        }
    }

    double t2 = MPI_Wtime();
    *t += t2 - t1;

    for (int i = 0; i < n; i++) {
        MPI_Gather(temp + i * chunk, chunk, MPI_DOUBLE, T + i * n, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
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
        printf("Repetitions: %d\n", rep);
        printf("Processes: %d\n\n", size);
    }

    if (n % size != 0) {
        if (rank == 0) {
            printf("Error: the matrix dimension must be divisible by the number of processes!\n");
        }

        MPI_Finalize();
        return -1;
    }

    // Variables declaration
    double ts1, ts2, te1, te2, t1, t2;  // temp time variables
    double t1m, t1e, t2m, t2e;          // execution times
    bool symmetric = false;             // symmetry check
    double* M;                          // input matrix
    double* T;                          // transposed matrix
    double* temp;                       // temporary matrix

    // Matrices allocation
    if (rank == 0) {
        if (initMatrices(&M, &T, n) == -1) {
            printf("Error in allocating matrices!\n\n");
            MPI_Finalize();
            return -1;
        }
    } else {
        M = (double*)malloc(n * n * sizeof(double));
    }

    // Parallel execution
    temp = (double*)malloc(n * n / size * sizeof(double));

    MPI_Barrier(MPI_COMM_WORLD);

    ts1 = MPI_Wtime();
    for (int j = 0; j < rep; j++) symmetric = checkSymMPI(M, n, rank, size, &t1);
    te1 = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);

    ts2 = MPI_Wtime();
    for (int j = 0; j < rep; j++) matTransposeMPI(M, T, temp, n, rank, size, &t2);
    te2 = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);

    // Results printing and saving
    MPI_Reduce(&t1, &t1e, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t2, &t2e, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        t1m = (te1 - ts1) / rep;
        t2m = (te2 - ts2) / rep;
        t1e = t1 / rep;
        t2e = t2 / rep;

        printf("Parallel execution (message passing included and excluded): symmetry: %s\n", symmetric ? "true" : "false");
        printf("checkSymMPI:\t %.9f\t%.9f seconds\n", t1m, t1e);
        printf("matTransposeMPI: %.9f\t%.9f seconds\n\n", t2m, t2e);

        testResults(M, T, n);

        if (saveResultsMPI(CODE, n, size, t1m, t1e, t2m, t2e) == -1) {
            printf("Error in saving results!\n\n");
        }
    }

    // Matrices deallocation
    if (M != NULL) free(M);
    if (T != NULL && rank == 0) free(T);
    if (temp != NULL) free(temp);

    MPI_Finalize();

    return 0;
}