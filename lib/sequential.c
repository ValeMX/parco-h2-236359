#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "functions.h"

#define CODE "S"

bool checkSym(const double* M, int n) {
    bool check = true;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (fabs(M[i * n + j] - M[j * n + i]) > EPSILON) {
                check = false;
            }
        }
    }

    return check;
}

void matTranspose(const double* M, double* T, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            T[j * n + i] = M[i * n + j];
        }
    }
}

// The attribute is necessary to avoid the compiler optimization on the repetitions loops
int __attribute__((optimize("O0"))) main(int argc, char** argv) {
    int dim = 0;
    int rep = 0;
    if (argc < 2) {
        printf("Correct usage: program-name [M dimension as exponent of 2] [number of repetitions (default 5)]");
        return 1;
    } else if (argc == 2) {
        dim = atoi(argv[1]);
        rep = 500;
    } else {
        dim = atoi(argv[1]);
        rep = atoi(argv[2]) > 0 ? atoi(argv[2]) : 500;
    }

    unsigned int n = pow(2, dim);
    printf("Matrix dimension: %d\n", n);
    printf("Repetitions: %d\n\n", rep);

    // Variables declaration
    struct timespec s1, s2, e1, e2;   // start, end times
    double t1, t2, flops, bandwidth;  // execution times and performance metrics
    bool symmetric = false;           // symmetry check
    double* M;                        // input matrix
    double* T;                        // transposed matrix

    // Matrices allocation
    if (initMatrices(&M, &T, n) == -1) {
        printf("Error in allocating matrices!\n\n");
        return -1;
    }

    // Sequential execution
    clock_gettime(CLOCK_MONOTONIC, &s1);
    for (int i = 0; i < rep; i++) symmetric = checkSym(M, n);
    clock_gettime(CLOCK_MONOTONIC, &e1);

    clock_gettime(CLOCK_MONOTONIC, &s2);
    for (int i = 0; i < rep; i++) matTranspose(M, T, n);
    clock_gettime(CLOCK_MONOTONIC, &e2);
    // --------------------------------

    // Results printing and saving
    t1 = elapsedTime(s1, e1) / rep;
    t2 = elapsedTime(s2, e2) / rep;

    flops = (double)((n * n) / 2 - n) / t1;
    bandwidth = (double)(2 * n * n * sizeof(double)) / t2;

    printf("Sequential execution: symmetry: %s\n", symmetric ? "true" : "false");
    printf("checkSym:\t%.9f seconds\t%10.4g GFLOPS\n", t1, flops * 1e-9);
    printf("matTranspose:\t%.9f seconds\t%10.4g GB/s\n\n", t2, bandwidth * 1e-9);

    testResults(M, T, n);

    if (saveResultsMPI(CODE, n, 1, t1, t1, t2, t2) == -1) {
        printf("Error in saving results!\n\n");
    }

    // Matrices deallocation
    free(M);
    free(T);

    return 0;
}