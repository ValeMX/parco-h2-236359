#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define EPSILON 1e-6
#define FILE_NAME_MPI "results_mpi.csv"

void printMatrix(const double* M, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%10g ", M[i * n + j]);
        }
        printf("\n");
    }
    printf("\n");
}

double elapsedTime(struct timespec s, struct timespec e) {
    long seconds = e.tv_sec - s.tv_sec;
    long nanoseconds = e.tv_nsec - s.tv_nsec;
    double time = seconds + nanoseconds * 1e-9;
    return time;
}

int saveResultsMPI(const char* code, int n, int processes, double t1m, double t1e, double t2m, double t2e) {
    FILE* f = fopen(FILE_NAME_MPI, "a");

    if (f == NULL) return -1;

    fprintf(f, "%s,%d,%d,%.9f,%.9f,%.9f,%.9f\n", code, n, processes, t1m, t1e, t2m, t2e);
    fclose(f);

    return 0;
}

int initMatrices(double** M, double** T, int n) {
    *M = (double*)malloc(n * n * sizeof(double));
    *T = (double*)malloc(n * n * sizeof(double));

    if (*M == NULL || *T == NULL) {
        free(*M);
        free(*T);
        return -1;
    }

    srand(time(0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            (*M)[i * n + j] = (double)rand() / RAND_MAX * 100;
        }
    }

    return 0;
}

void testResults(double* M, double* T, int n) {
    bool check = true;
    bool transposed = true;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (fabs(M[i * n + j] - M[j * n + i]) > EPSILON) {
                check = false;
            }

            if (T[j * n + i] != M[i * n + j]) {
                transposed = false;
            }
        }
    }

    printf("Tested results: symmetry %s and transposed %s.\n\n", check ? "true" : "false", transposed ? "correct" : "incorrect");
}

void testResultsBlock(double* M, double* T, int n) {
    bool transposed = true;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (T[j * n + i] != M[i * n + j]) {
                transposed = false;
            }
        }
    }

    printf("Tested results: transposed %s.\n\n", transposed ? "correct" : "incorrect");
}