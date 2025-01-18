#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "functions.h"

#define CODE "MC"

MPI_Datatype type_row_n, type_column, type_column_n, type_column_chunk;

bool checkSymMPI(double* M, double* temp_rows, double* temp_columns, int n, int rank, int size, double* t) {
    int chunk = n / size;
    bool local_check = true;
    bool check = true;

    MPI_Scatter(M, chunk, type_row_n, temp_rows, chunk, type_row_n, 0, MPI_COMM_WORLD);
    MPI_Scatter(M, chunk, type_column_n, temp_columns, chunk, type_row_n, 0, MPI_COMM_WORLD);

    double t1 = MPI_Wtime();

    for (int i = 0; i < chunk; i++) {
        for (int j = 0; j < i; j++) {
            if (fabs(temp_rows[i * n + j] - temp_columns[i * n + j]) > EPSILON) {
                local_check = false;
            }
        }
    }

    double t2 = MPI_Wtime();
    *t += t2 - t1;

    MPI_Reduce(&local_check, &check, 1, MPI_C_BOOL, MPI_LAND, 0, MPI_COMM_WORLD);

    return check;
}

void matTransposeMPI(double* M, double* T, double* temp_rows, double* temp_columns, int n, int rank, int size, double* t) {
    int chunk = n / size;

    MPI_Scatter(M, chunk, type_row_n, temp_rows, chunk, type_row_n, 0, MPI_COMM_WORLD);

    double t1 = MPI_Wtime();

    for (int i = 0; i < chunk; i++) {
        for (int j = 0; j < n; j++) {
            temp_columns[j * chunk + i] = temp_rows[i * n + j];
        }
    }

    double t2 = MPI_Wtime();
    *t += t2 - t1;

    MPI_Gather(temp_columns, chunk, type_column_chunk, T, chunk, type_column_n, 0, MPI_COMM_WORLD);
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
    double *temp_rows, *temp_columns;   // temporary matrix

    // Matrices allocation
    if (rank == 0) {
        if (initMatrices(&M, &T, n) == -1) {
            printf("Error in allocating matrices!\n\n");
            MPI_Finalize();
            return -1;
        }
    }

    // Derived data types definition
    MPI_Type_contiguous(n, MPI_DOUBLE, &type_row_n);
    MPI_Type_commit(&type_row_n);

    MPI_Type_vector(n, 1, n, MPI_DOUBLE, &type_column);
    MPI_Type_commit(&type_column);

    MPI_Type_create_resized(type_column, 0, 1 * sizeof(double), &type_column_n);
    MPI_Type_commit(&type_column_n);

    MPI_Type_vector(n, 1, n / size, MPI_DOUBLE, &type_column);
    MPI_Type_commit(&type_column);

    MPI_Type_create_resized(type_column, 0, 1 * sizeof(double), &type_column_chunk);
    MPI_Type_commit(&type_column_chunk);

    // Parallel execution
    temp_rows = (double*)malloc(n * n / size * sizeof(double));
    temp_columns = (double*)malloc(n * n / size * sizeof(double));

    MPI_Barrier(MPI_COMM_WORLD);

    ts1 = MPI_Wtime();
    for (int i = 0; i < rep; i++) symmetric = checkSymMPI(M, temp_rows, temp_columns, n, rank, size, &t1);
    te1 = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);

    ts2 = MPI_Wtime();
    for (int i = 0; i < rep; i++) matTransposeMPI(M, T, temp_rows, temp_columns, n, rank, size, &t2);
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
    if (M != NULL && rank == 0) free(M);
    if (T != NULL && rank == 0) free(T);
    if (temp_rows != NULL) free(temp_rows);
    if (temp_columns != NULL) free(temp_columns);

    // Derived data types deallocation
    MPI_Type_free(&type_row_n);
    MPI_Type_free(&type_column);
    MPI_Type_free(&type_column_n);
    MPI_Type_free(&type_column_chunk);

    MPI_Finalize();

    return 0;
}