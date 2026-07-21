#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"
#include "cacheutils.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>



#define SHM_SIZE 4*1024
#define WAIT_TIME 1


int main(){

    // ********** DO NOT MODIFY THIS SECTION **********
    FILE *fp = fopen(MSG_FILE, "r");
    if(fp == NULL){
        printf("Error opening file\n");
        return 1;
    }

    char msg[MAX_MSG_SIZE];
    int msg_size = 0;
    char c;
    while((c = fgetc(fp)) != EOF){
        msg[msg_size++] = c;
    }
    fclose(fp);

    clock_t start = clock();
    // **********************************************
    // ********** YOUR CODE STARTS HERE **********

    int shm_fd = shm_open("secrettunnel",O_CREAT|O_RDWR,0666);
    ftruncate(shm_fd,SHM_SIZE);


    char *shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE , MAP_SHARED, shm_fd, 0); // Shared memory mapped read only
    printf("%p\n",shm_ptr);

    //start the message with a full 1111111
    for(int i =0 ;i<4000;i++){
        //access that memory address
        char x = shm_ptr[0];
        //waste some cycles here
        usleep(WAIT_TIME);
    }

    for( int i =0;i < msg_size;i++){
        for(int j =0;j<8*sizeof(char);j++){
            for(int k = 0 ;k < 100 ; k++){if(msg[i] & 1){
                //access that memory address
                char x = shm_ptr[0];
                usleep(WAIT_TIME);
            }
            msg[i] = msg[i] >>1;
            //waste some cycles here
            }
            
        }
    }

    while(! shm_ptr[0]){
        printf("waiting ... \n");
        usleep(300000);
    }
    munmap(shm_ptr,SHM_SIZE);
    shm_unlink("secrettunnel");


    // ********** YOUR CODE ENDS HERE **********
    // ********** DO NOT MODIFY THIS SECTION **********
    clock_t end = clock();
    double time_taken = ((double)end - start) / CLOCKS_PER_SEC;
    printf("Message sent successfully\n");
    printf("Time taken to send the message: %f\n", time_taken);
    printf("Message size: %d\n", msg_size);
    printf("Bits per second: %f\n", msg_size * 8 / time_taken);
    // **********************************************
}
