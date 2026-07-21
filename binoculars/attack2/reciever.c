#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <x86intrin.h>
#include <string.h>
#include <sys/ioctl.h>

#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#include "/home/imnothackr/cs_core/cs773/binoculars/geyaan_code/1/binoculars/PTEditor/ptedit_header.h"
// #include "utils.h"


#define VICTIM_LOAD_ADDR (0x5d21ca821000ull)
#define PAGE_SIZE 4096
#define Threshold 2000
#define Repeat 100

typedef uint64_t u64;
typedef uint8_t u8;
typedef uint32_t u32;

u8 *mmap_private(void *addr, size_t size) {
    u8 *ptr = mmap(addr, size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static inline void _maccess(volatile void *p) {
    asm volatile("" : : "r"(*(volatile u8 *)p) : "memory");
}

// Read time stamp counter at start
static inline u64 _timer_start() {
    unsigned cycles_high, cycles_low;
    asm volatile ("CPUID\n\t"
                  "RDTSC\n\t"
                  "mov %%edx, %0\n\t"
                  "mov %%eax, %1\n\t"
                  : "=r" (cycles_high), "=r" (cycles_low)
                  :: "%rax", "%rbx", "%rcx", "%rdx");
    return ((u64)cycles_high << 32) | cycles_low;
}

// Read time stamp counter at end
static inline u64 _timer_end() {
    unsigned cycles_high, cycles_low;
    asm volatile ("RDTSCP\n\t"
                  "mov %%edx, %0\n\t"
                  "mov %%eax, %1\n\t"
                  "CPUID\n\t"
                  : "=r" (cycles_high), "=r" (cycles_low)
                  :: "%rax", "%rbx", "%rcx", "%rdx");
    return ((u64)cycles_high << 32) | cycles_low;
}



void measure_page_access_time(){
    const unsigned MEASURES = 10000;
    int results[MEASURES];
    // int results2[MEASURES];
    u8 *page = mmap_private((void *)VICTIM_LOAD_ADDR, PAGE_SIZE);
    for (unsigned cnt = 0; cnt < MEASURES; cnt++) {
        _maccess(page);
        // int sum = 0;
        // int max = 0;
        u64 t_start = _timer_start();
        for (unsigned i = 0; i < Repeat; i++) {
            ptedit_invalidate_tlb(page);
            // _mm_mfence();
            _maccess(page);
            _mm_mfence();
        }
        u64 t_end = _timer_end();
        results[cnt] = (t_end - t_start )/ Repeat;
        // results2[cnt] = max;

    }

    for (u32 i = 0; i < MEASURES; i++) {
        printf("%u\n", results[i]);
    }
    // printf("<-------------------------------------------------------->\n");
    // for (u32 i = 0; i < MEASURES; i++) {
    //     printf("%u\n", results2[i]);
    // }
}

int measure_page_access_time_once(){
    const unsigned MEASURES = 1;
    u8 *page = mmap_private((void *)VICTIM_LOAD_ADDR, PAGE_SIZE);
    _maccess(page);
    u64 t_start = _timer_start();
    for (unsigned i = 0; i < Repeat; i++) {
        ptedit_invalidate_tlb(page);
        _mm_mfence();
        _maccess(page);
        _mm_mfence();
    }
    u64 t_end = _timer_end();

    return (t_end - t_start )/ Repeat;
}



int main(){
    if (ptedit_init()) {
        return 1;
    }
    int shm_fd = shm_open("/shared_mem", O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        return 1;
    }

    // Set the size of the shared memory
    if (ftruncate(shm_fd, PAGE_SIZE) == -1) {
        perror("ftruncate");
        shm_unlink("/shared_mem");
        return 1;
    }

    // Map the shared memory
    int *shared_mem = (int *)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap (shared memory)");
        shm_unlink("/shared_mem");
        return 1;
    }

    printf("Shared memory mapped at %p\n", shared_mem);

    // Wait for the sender to signal
    while (shared_mem[1] == 0);
    shared_mem[0] = 1;
    shared_mem[1] = 0;
    int size = shared_mem[4];
    int arr[size];
    int i=0;
    while(shared_mem[2]){
        while(shared_mem[1] == 0);
        shared_mem[1] = 0;
        int time = measure_page_access_time_once();
        if(time < Threshold){
            arr[i++] = 0;
        }else{
            arr[i++] = 1;
        }
        shared_mem[1] = 0;
        shared_mem[0] = 1;
    }

    // for(int i =0 ;i<size;i++){
    //     printf("%d , ",arr[i]);
    // }
    // printf("\n");

    FILE *output_file = fopen("output_covert", "w");
    if (output_file == NULL) {
        perror("fopen");
        return 1;
    }

    for (int j = 0; j < size; j++) {
        fprintf(output_file, "%d, ", arr[j]);
    }
    fprintf(output_file, "\n");

    fclose(output_file);

    // measure_page_access_time_once();

    ptedit_cleanup();
    return 0;
}