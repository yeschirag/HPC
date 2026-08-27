#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TC 1.0

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <image_size>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);

    if (n < 3) {
        printf("Image size must be at least 3.\n");
        return 1;
    }

    double *image = malloc((long long)n * n * sizeof(double));
    double *output = malloc((long long)n * n * sizeof(double));

    if (image == NULL || output == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Generate input image
    for (long long i = 0; i < (long long)n * n; i++) {
        image[i] = (double)(i % 256);
        output[i] = 0.0;
    }

    // 3x3 edge detection kernel
    double kernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    clock_t start = clock();

    // Process image
    for (int i = 1; i < n - 1; i++) {

        for (int j = 1; j < n - 1; j++) {

            double sum = 0.0;

            for (int ki = -1; ki <= 1; ki++) {

                for (int kj = -1; kj <= 1; kj++) {

                    sum +=
                        image[(i + ki) * n + (j + kj)]
                        * kernel[ki + 1][kj + 1];
                }
            }

            output[i * n + j] = sum;
        }
    }

    clock_t end = clock();

    double time_taken =
        (double)(end - start) / CLOCKS_PER_SEC;

    printf("%d,1,%.6f\n", n, time_taken);

    free(image);
    free(output);

    return 0;
}