#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h> // For rdtsc intrinsic
#define N 10000

int main() {
    volatile int data = 10; // Volatile to prevent compiler optimizations
    volatile int x =0;
    uint64_t start_cycles, end_cycles;
    uint64_t cycletime[N];
    while(1){
        start_cycles = __rdtsc();
        for(int i =0;i<1;i++){
            x = data;
            _mm_mfence();
            _mm_clflush(&data);
            _mm_mfence();
        }
        end_cycles = __rdtsc();
    }

    return 0;
}