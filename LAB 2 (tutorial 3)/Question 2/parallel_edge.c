#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int start_row;
    int end_row;
    int n;
    double *image;
    double *output;
} ThreadData;


double kernel[3][3] = {
    {-1, -1, -1},
    {-1,  8, -1},
    {-1, -1, -1}
};


void *process_rows(void *arg) {

    ThreadData *data = (ThreadData *)arg;

    int n = data->n;

    for (int i = data->start_row; i < data->end_row; i++) {

        // Skip boundary rows
        if (i == 0 || i == n - 1)
            continue;

        for (int j = 1; j < n - 1; j++) {

            double sum = 0.0;

            for (int ki = -1; ki <= 1; ki++) {

                for (int kj = -1; kj <= 1; kj++) {

                    sum +=
                        data->image[
                            (i + ki) * n + (j + kj)
                        ]
                        * kernel[ki + 1][kj + 1];
                }
            }

            data->output[i * n + j] = sum;
        }
    }

    return NULL;
}


double get_time() {

    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec + ts.tv_nsec / 1e9;
}


int main(int argc, char *argv[]) {

    if (argc != 3) {

        printf(
            "Usage: %s <image_size> <threads>\n",
            argv[0]
        );

        return 1;
    }

    int n = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    if (n < 3) {

        printf("Image size must be at least 3.\n");

        return 1;
    }

    if (num_threads <= 0) {

        printf("Number of threads must be positive.\n");

        return 1;
    }

    if (num_threads > n - 2)
        num_threads = n - 2;


    long long total_pixels =
        (long long)n * n;


    double *image =
        malloc(total_pixels * sizeof(double));

    double *output =
        malloc(total_pixels * sizeof(double));


    if (image == NULL || output == NULL) {

        printf("Memory allocation failed.\n");

        return 1;
    }


    // Generate input image
    for (long long i = 0; i < total_pixels; i++) {

        image[i] = (double)(i % 256);
        output[i] = 0.0;
    }


    pthread_t threads[num_threads];

    ThreadData data[num_threads];


    // Number of rows that actually need processing
    int rows = n - 2;

    int rows_per_thread =
        rows / num_threads;

    int remainder =
        rows % num_threads;


    int current_row = 1;


    double start = get_time();


    // Create threads
    for (int i = 0; i < num_threads; i++) {

        int extra =
            (i < remainder) ? 1 : 0;

        data[i].start_row =
            current_row;

        data[i].end_row =
            current_row
            + rows_per_thread
            + extra;

        data[i].n = n;

        data[i].image = image;

        data[i].output = output;


        pthread_create(
            &threads[i],
            NULL,
            process_rows,
            &data[i]
        );


        current_row =
            data[i].end_row;
    }


    // Wait for all threads
    for (int i = 0; i < num_threads; i++) {

        pthread_join(
            threads[i],
            NULL
        );
    }


    double end = get_time();


    double time_taken =
        end - start;


    printf(
        "%d,%d,%.6f\n",
        n,
        num_threads,
        time_taken
    );


    free(image);
    free(output);

    return 0;
}