#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#define PHYS_ADDR 0xFEC00000  // 🔥 Replace with safe UC memory
#define PAGE_SIZE 4096

int main() {
    int fd;
    volatile uint32_t *mem;

    // Open /dev/mem (requires root)
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Map physical memory (must be UC to lock bus)
    mem = (volatile uint32_t *) mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                                     MAP_SHARED, fd, PHYS_ADDR);
    if (mem == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("Memory mapped at %p\n", mem);

    while(1){
    // Perform LOCK XCHG — try to lock bus
        asm volatile (
            "movl $0x1, %%eax\n\t"
            "lock xchgl %%eax, %0\n\t"
            : "+m" (*mem)
            :
            : "eax", "memory"
        );
        // struct timespec ts = {0, 100};
        // nanosleep(&ts, NULL);
    }

    printf("LOCK XCHG executed\n");

    munmap((void *)mem, PAGE_SIZE);
    close(fd);

    return 0;
}
