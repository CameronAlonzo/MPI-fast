#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int main(int argc, char *argv[]) {
    int rank, size, n, rootP, sub_n;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            printf("Usage: %s <matrix size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    n = atoi(argv[1]);
    rootP = (int)sqrt(size);
    if (n % rootP != 0 || rootP * rootP != size) {
        if (rank == 0) {
            printf("Error: Number of processes must be a perfect square, and matrix size n must be divisible by the square root of the number of processes.\n");
        }
        MPI_Finalize();
        return 1;
    }

    sub_n = n / rootP;
    int *D0 = malloc(sub_n * sub_n * sizeof(int));
    int *D = malloc(sub_n * sub_n * sizeof(int));

    for (int i = 0; i < sub_n; i++) {
        for (int j = 0; j < sub_n; j++) {
            D0[i * sub_n + j] = (i != j) ? (i + j + 2) : 0; // Plus 2 to ensure positive values
        }
    }

    memcpy(D, D0, sub_n * sub_n * sizeof(int));

    for (int k = 0; k < n; k++) {
        int k_owner = k / sub_n;
        int k_row_global = (k % sub_n) * sub_n;
        int *kth_row = malloc(sub_n * sizeof(int));

        if (rank == k_owner) {
            memcpy(kth_row, &D0[k_row_global], sub_n * sizeof(int));
        }
        MPI_Bcast(kth_row, sub_n, MPI_INT, k_owner, MPI_COMM_WORLD);

        for (int i = 0; i < sub_n; i++) {
            for (int j = 0; j < sub_n; j++) {
                D[i * sub_n + j] = MIN(D[i * sub_n + j], D0[i * sub_n + (k % sub_n)] + kth_row[j]);
            }
        }
        memcpy(D0, D, sub_n * sub_n * sizeof(int));
        free(kth_row);
    }

    // Allocate memory for the full matrix at the root process
    int *full_D = NULL;
    if (rank == 0) {
        full_D = malloc(n * n * sizeof(int));
    }

    // Gather all sub-matrices into the full matrix at the root process
    MPI_Gather(D0, sub_n * sub_n, MPI_INT, full_D, sub_n * sub_n, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Final matrix D:\n");
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                printf("%d ", full_D[i * n + j]);
            }
            printf("\n");
        }
        free(full_D);
    }

    free(D0);
    free(D);
    MPI_Finalize();
    return 0;
}