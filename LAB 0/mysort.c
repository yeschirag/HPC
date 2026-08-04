/*
 * mysort.c
 * ------------------------------------------------------------
 * Performance Profiling Lab (GPROF / PERF)
 *
 * Simulates N daily temperature readings (default 1,000,000,
 * i.e. roughly 50 years of "hourly-ish" weather-station records)
 * and sorts them with two algorithms:
 *
 *      1. Bubble Sort   -> O(n^2)  baseline
 *      2. Quick Sort    -> O(n log n) average, optimized approach
 *
 * Usage:
 *   ./mysort [N] [mode]
 *      N     -> number of temperature records to generate (default 1000000)
 *      mode  -> "quick" | "bubble" | "both"   (default "both")
 *
 * Because Bubble Sort is O(n^2), running it on 1,000,000 elements
 * is not feasible in a lab setting (it would need on the order of
 * 10^12 comparisons). The program therefore automatically caps the
 * Bubble Sort input size and warns the user, while Quick Sort always
 * runs on the full requested size. This mirrors real engineering
 * practice: you do not run an O(n^2) baseline at production scale,
 * you extrapolate its behaviour from smaller measured runs.
 * ------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_SIZE   1000000
#define BUBBLE_CAP     50000     /* practical ceiling for O(n^2) sort */

/* ---------- data generation -------------------------------- */

/* Simulate a daily temperature in degrees C * 10 (tenths of a degree),
 * range roughly -30.0C to 50.0C, stored as int for cheap, cache-friendly
 * comparisons (this is how real weather-station pipelines often store
 * fixed-point sensor data). */
static int *generate_temperatures(long n, unsigned int seed) {
    int *arr = (int *)malloc(n * sizeof(int));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed for %ld elements\n", n);
        exit(1);
    }
    srand(seed);
    for (long i = 0; i < n; i++) {
        /* -300 .. 500 tenths => -30.0C .. 50.0C */
        arr[i] = (rand() % 801) - 300;
    }
    return arr;
}

/* ---------- Bubble Sort -------------------------------------- */

static void swap_int(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

static void bubbleSort(int arr[], long n) {
    for (long i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (long j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap_int(&arr[j], &arr[j + 1]);
                swapped = 1;
            }
        }
        if (!swapped) break; /* already sorted, small optimization */
    }
}

/* ---------- Quick Sort ----------------------------------------- */

static int partition(int arr[], long low, long high) {
    int pivot = arr[high];
    long i = (low - 1);

    for (long j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap_int(&arr[i], &arr[j]);
        }
    }
    swap_int(&arr[i + 1], &arr[high]);
    return (int)(i + 1);
}

static void quickSort(int arr[], long low, long high) {
    if (low < high) {
        long pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

/* ---------- helpers -------------------------------------------- */

static int is_sorted(int arr[], long n) {
    for (long i = 1; i < n; i++)
        if (arr[i - 1] > arr[i]) return 0;
    return 1;
}

static double elapsed_seconds(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

/* ---------- main -------------------------------------------------- */

int main(int argc, char *argv[]) {
    long n = DEFAULT_SIZE;
    char mode[16] = "both";

    if (argc >= 2) n = atol(argv[1]);
    if (argc >= 3) strncpy(mode, argv[2], sizeof(mode) - 1);

    unsigned int seed = 42; /* fixed seed => reproducible datasets across runs */

    printf("=== Weather Sort Benchmark ===\n");
    printf("Requested dataset size: %ld temperature records\n", n);

    struct timespec t0, t1;

    if (strcmp(mode, "quick") == 0 || strcmp(mode, "both") == 0) {
        int *data = generate_temperatures(n, seed);
        clock_gettime(CLOCK_MONOTONIC, &t0);
        quickSort(data, 0, n - 1);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double t = elapsed_seconds(t0, t1);
        printf("[QuickSort]  n=%ld  time=%.4f s  sorted_ok=%d\n",
               n, t, is_sorted(data, n));
        free(data);
    }

    if (strcmp(mode, "bubble") == 0 || strcmp(mode, "both") == 0) {
        long bn = n;
        if (bn > BUBBLE_CAP) {
            printf("[BubbleSort] n=%ld exceeds practical O(n^2) limit; "
                   "capping Bubble Sort input to %d for this run.\n",
                   n, BUBBLE_CAP);
            bn = BUBBLE_CAP;
        }
        int *data2 = generate_temperatures(bn, seed);
        clock_gettime(CLOCK_MONOTONIC, &t0);
        bubbleSort(data2, bn);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double t = elapsed_seconds(t0, t1);
        printf("[BubbleSort] n=%ld  time=%.4f s  sorted_ok=%d\n",
               bn, t, is_sorted(data2, bn));
        free(data2);
    }

    return 0;
}
