#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h> // For rdtsc intrinsic
#define N 10000

int main() {
    volatile int data = 10; // Volatile to prevent compiler optimizations
    volatile int x =0;
    uint64_t start_cycles, end_cycles;
    uint64_t cycletime[N];
    for(int j = 0; j < N; j++) {
        start_cycles = __rdtsc();
        for(int i =0;i<100;i++){
            x = data;
            _mm_mfence();
            _mm_clflush(&data);
            _mm_mfence();
        }
        end_cycles = __rdtsc();
        cycletime[j] = (end_cycles - start_cycles )/ 100;
    }

    FILE *file = fopen("cycles_output.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    for (int j = 0; j < N; j++) {
        fprintf(file, "%lu\n", cycletime[j]);
    }

    fclose(file);
    return 0;
}