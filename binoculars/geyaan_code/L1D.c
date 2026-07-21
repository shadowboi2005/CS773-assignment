#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <x86intrin.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 256
#define NUM_ITERATIONS 1000

// Global variables for thread coordination
volatile int running = 1;
volatile uint8_t *conflict_addr;
int conflict_page;

// Thread function for background write operation
void *background_writer(void *arg) {
  // Bind to core 1 (different from main thread)
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(1, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  conflict_addr = (int *)arg + 204;
  printf("%d\n" , ((int)conflict_addr%PAGE_SIZE + PAGE_SIZE)%PAGE_SIZE);
  // Continuously write to the conflict address
  while (running) {
    *conflict_addr = rand();                 // Write operation
    // _mm_clflush((void *)conflict_addr); // Ensure memory access each time
    _mm_mfence();

  }

  return NULL;
}

// Bind to a specific core for consistency
void bind_to_core(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}

uint64_t measure_access_time(volatile uint8_t *addr) {
  uint32_t unused;
  uint64_t start, end;

  _mm_mfence();
  _mm_lfence();

  start = __rdtscp(&unused);
  *(volatile uint8_t *)addr; // Read access
  _mm_lfence();
  end = __rdtscp(&unused);

  return end - start;
}

void handle_signal(int sig) {
  running = 0;
  printf("\nReceived signal %d, cleaning up and exiting...\n", sig);
}

int main() {
  // Set up signal handler for clean exit
  signal(SIGINT, handle_signal);

  // Bind main thread to core 0
  bind_to_core(0);

  // Seed random number generator
  srand(time(NULL));

  // Allocate a large buffer aligned to page size
  int *buffer = malloc(PAGE_SIZE*2);
  
  if (!buffer) {
    perror("aligned_alloc");
    return 1;
  }
  memset(buffer, 0, PAGE_SIZE*2);
  size_t buffer_size = PAGE_SIZE * 2;

  // Choose a random 4K offset for our conflict address
  conflict_addr = buffer + PAGE_SIZE/sizeof(int);

  printf("Using conflict address at 4K offset: %d (%p)\n", conflict_page,
         (void *)conflict_addr);

  // Create CSV output file
  FILE *output = fopen("cache_aliasing_data.csv", "w");
  if (!output) {
    perror("Failed to open output file");
    free(buffer);
    return 1;
  }

  fprintf(output, "offset_kb,access_time_cycles\n");

  // Create background writer thread
  pthread_t writer_thread;
  if (pthread_create(&writer_thread, NULL, background_writer, conflict_addr) != 0) {
    perror("Failed to create background writer thread");
    free(buffer);
    fclose(output);
    return 1;
  }

  // Give the background thread time to start
  usleep(100000); // 100ms

  printf("Starting access time measurements across all offsets...\n");

  // Now test all offsets to see which ones conflict with our fixed address
  for (int i = 0; i < PAGE_SIZE/sizeof(int) && running; i++) {

    // Test address at current offset
    volatile int *test_addr = buffer+i;

    uint64_t total_cycles = 0;
    // printf("reader addr accessed: %p\n", (void *)test_addr);
    // Multiple iterations for statistical significance
    for (int iter = 0; iter < NUM_ITERATIONS && running; iter++) {
      // Access test address - if there's aliasing with the background writer's
      // target, this access will be slower
      *test_addr; // Load into cache
      _mm_mfence();

      // Measure access time to the test address
      uint64_t cycles = measure_access_time(test_addr);
      total_cycles += cycles;
    }

    double avg_cycles = (double)total_cycles / NUM_ITERATIONS;
    fprintf(output, "%d,%.2f\n", i, avg_cycles);

    // Print progress
    if (i % 4 == 0) {
      printf("Offset %d : %.2f cycles\n", ((int)test_addr%PAGE_SIZE + PAGE_SIZE)%PAGE_SIZE , avg_cycles);
    }
  }

  // Stop background thread
  running = 0;
  pthread_join(writer_thread, NULL);

  fclose(output);
  free(buffer);
  return 0;
}
