#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_points>\n", argv[0]);
        return 1;
    }

    long long n = atoll(argv[1]);
    long long inside = 0;

    srand(42);

    clock_t start = clock();

    for (long long i = 0; i < n; i++) {
        double x = 2.0 * rand() / (double)RAND_MAX - 1.0;
        double y = 2.0 * rand() / (double)RAND_MAX - 1.0;

        if (x * x + y * y <= 1.0)
            inside++;
    }

    clock_t end = clock();

    double pi = 4.0 * inside / n;
    double error = pi - 3.14159265358979323846;
    double abs_error = error < 0 ? -error : error;
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("%lld,1,%.12f,%.12f,%.9f\n",
       n, pi, abs_error, time_taken);

    return 0;
}