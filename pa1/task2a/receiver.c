#include <stdio.h>
#include "utils.h"
#include "cacheutils.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



#define SHM_SIZE 4*1024
#define WAIT_TIME 1
#define MIN_CACHE_MISS_CYCLES 220



int main(){
    
    // Update these values accordingly
    char* received_msg = NULL;
    received_msg = malloc(MAX_MSG_SIZE);
    int received_msg_size = 0;
    int shm_fd = shm_open("secrettunnel",O_CREAT|O_RDWR,0666);
    ftruncate(shm_fd,SHM_SIZE);

    char *shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE , MAP_SHARED, shm_fd, 0); // Shared memory mapped read only
    shm_ptr[0] = 'a';
    printf("%08x \n",shm_ptr[0]);
    char buff = NULL;
    char end = 0xff;
    printf("%d \n",sizeof(buff));
    while(buff != end){
        flush(shm_ptr);
        usleep(WAIT_TIME);
        size_t time = rdtsc();
            maccess(shm_ptr); // find hit/miss time for T0
        size_t delta = rdtsc() - time;
        buff = buff <<1 ;
        buff = buff |(delta < MIN_CACHE_MISS_CYCLES);
        if(delta < MIN_CACHE_MISS_CYCLES){
            printf("%08x \n",buff);
        }
    }
    
    end = 0;
    int counter = 0;
    while(buff != end){
        counter++;
        flush(shm_ptr);
        usleep(WAIT_TIME);
        size_t time = rdtsc();
            maccess(shm_ptr); // find hit/miss time for T0
        size_t delta = rdtsc() - time;
        buff = buff <<1 ;
        buff = buff |(delta < MIN_CACHE_MISS_CYCLES);
        if(delta < MIN_CACHE_MISS_CYCLES){
            printf("%08x \n",buff);
        }
        if(counter ==8){
            received_msg[received_msg_size] = buff;
            received_msg_size++;
            buff = 0;
        }
    }

    munmap(shm_ptr,SHM_SIZE);
    shm_unlink("secrettunnel");

    printf("%s \n" ,received_msg );     
    // DO NOT MODIFY THIS LINE
    printf("Accuracy (%%): %f\n", check_accuracy(received_msg, received_msg_size)*100);
}