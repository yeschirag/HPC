#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

typedef struct {
    long long points;
    long long inside;
    unsigned int seed;
} ThreadData;

void *monte_carlo(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    data->inside = 0;

    for (long long i = 0; i < data->points; i++) {
        double x = 2.0 * rand_r(&data->seed) / (double)RAND_MAX - 1.0;
        double y = 2.0 * rand_r(&data->seed) / (double)RAND_MAX - 1.0;

        if (x * x + y * y <= 1.0)
            data->inside++;
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
        printf("Usage: %s <number_of_points> <number_of_threads>\n", argv[0]);
        return 1;
    }

    long long n = atoll(argv[1]);
    int num_threads = atoi(argv[2]);

    if (num_threads <= 0) {
        printf("Number of threads must be greater than 0.\n");
        return 1;
    }

    if (num_threads > n)
        num_threads = (int)n;

    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    // Divide points among threads
    long long points_per_thread = n / num_threads;
    long long remainder = n % num_threads;

    double start = get_time();

    // Create threads
    for (int i = 0; i < num_threads; i++) {

        data[i].points = points_per_thread;

        // Give one extra point to the first 'remainder' threads
        if (i < remainder)
            data[i].points++;

        data[i].inside = 0;

        // Different seed for every thread
        data[i].seed = 42 + i * 100;

        pthread_create(
            &threads[i],
            NULL,
            monte_carlo,
            &data[i]
        );
    }

    // Wait for all threads
    long long inside = 0;

    for (int i = 0; i < num_threads; i++) {

        pthread_join(threads[i], NULL);

        inside += data[i].inside;
    }

    double end = get_time();

    // Calculate Pi
    double pi = 4.0 * inside / n;

    double error = pi - 3.14159265358979323846;

    double abs_error = fabs(error);

    double time_taken = end - start;

    printf("%lld,%d,%.12f,%.12f,%.9f\n",
       n, num_threads, pi, abs_error, time_taken);

    return 0;
}