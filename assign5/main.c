#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "dLinkedList.h"

//Global Variables
linked_list *Ready_Q, *IO_Q, *done_Q;
double CLOCK = 0;
int file_read_done = 0;
int cpu_sch_done = 0;
int io_sys_done = 0;
int cpu_busy = 0;
int io_busy = 0;
sem_t sem_cpu, sem_io;
struct timespec atimespec;
FILE *fp;

typedef struct quantum{
    char *algo;
    int quan;
}quantum;

//Functions
void *fileRead(void *args);
void *CPU_scheduler_FIFO(void *args);
void *IO_system_thread(void *args);
/*
 * Function - Main(int, char*): Main takes in user input as command line arguments. Argc tells us the amount of arguments and Argv gives us access to those arguments in the form of a string
 * Validates the number of command line arguments, stores the corresponding data into local variables to be passed later.
 *           Three threads are created and waited on, at the end times for the process and CPUbursts are calculated and printed.
 */
int main(int argc, char *argv[]) {
    //Initialize the 2 lists we need and create the 3 pthread variables
   Ready_Q = newLinkedList();
   IO_Q = newLinkedList();
   done_Q = newLinkedList();
   pthread_t th1, th2, th3;

   //initialize sem
    sem_init(&sem_cpu, 0, 0);
    sem_init(&sem_io, 0, 0);
   //Variables that hold input line information (quantum, file name, etc)
   char *fileName;
   char *algo;
   int quantum;
   atimespec.tv_sec = 1; //Set the tv_sec to 1 for our two scheduler functions

   if(argc < 5 || argc > 7){ //Checks for two few or too little command line arguments
       printf("Invalid Number of Arguments!");
       exit(1);
   }
    printf("Student Name: Brandon Stevenson\n");
    for(int x = 0; x < argc; x++) { //Will look for filename, quantum number, and Algorithm
        if (strcmp(argv[x], "-input") == 0) {
            fileName = argv[x + 1];
            printf("\n");
            printf("Input File Name :     %s\n", fileName);
        }
        if(strcmp(argv[x], "-alg") == 0) {
            algo = argv[x + 1];
            printf("CPU Scheduling Alg:   %s", algo);
        }
        if(strcmp(argv[x], "-quantum") == 0) {
            quantum = atoi(argv[x + 1]);
            printf("  quantum: %d", quantum);
        }
    }

    struct quantum q = {algo, quantum}, *RR;
    RR = calloc(1,sizeof(quantum));
    RR = &q;
    //Creates all 3 process threads
    pthread_create(&th1, NULL, fileRead, fileName); //Starts thread fileRead
    pthread_create(&th2, NULL, CPU_scheduler_FIFO, RR); //Starts thread CPU_scheduler_FIFO
    pthread_create(&th3, NULL, IO_system_thread, NULL); //Starts thread IO_scheduler

    //Wait for threads to finish
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    pthread_join(th3, NULL);
    PCB *forPrint = done_Q->head;
    while(forPrint != NULL){
        CLOCK += forPrint->elapsed;
        forPrint = forPrint->next;
    }

    printf("CPU Utilization:      TODO\n");
    printf("Throughput:           %.3f jobs per ms\n", LinkedListLength(done_Q)/((done_Q->tail->elapsed) * 1000));
    printf("Avg. Turnaround time: %.2f ms\n", (CLOCK*1000)/LinkedListLength(done_Q));
    CLOCK = CLOCK - done_Q->tail->elapsed;
    printf("Avg. Waiting time in R queue: %.2f ms\n", (CLOCK * 1000)/ 5);
    //printForward(done_Q);
    //printPCB(done_Q);
    //Free up all memory in both linked list and close the input file
    FreeLinkedList(Ready_Q);
    FreeLinkedList(IO_Q);
    FreeLinkedList(done_Q);
    return 0;
}
/*
 * Function - fileRead*/
void *fileRead(void *args){
    int currPID = 0; //To assign PID to new processes
    char *fileName = (char*) args; //Initializes args
    int *cpuBurstA, *ioBurstA;
    fp = fopen(fileName, "r"); //Open file to be read
    if(fp == NULL){ //Checks if file is empty/unreadable
        printf("File is Empty!");
        fclose(fp);
        exit(1);
    }
    //Find size of file bytes to create a memory accurate calloc
    fseek(fp, 0L, SEEK_END);
    long bytes;
    bytes = ftell(fp);

    fseek(fp, 0L, SEEK_SET);
    char *fileData = (char*) calloc(bytes, sizeof(char)); //Calloc should be called correctly with the correct number of elements and size of bytes
    fread(fileData, sizeof(char), bytes, fp);


    char *line = NULL, *save1 = NULL, *token = NULL, *save2 = NULL; //Create char pointers to hold string values
    line = strtok_r(fileData, "\n", &save1); //Grabs first line in input file
    while(line != NULL) {
        //this part is kind of dumb, but I'll leave it in. Just gets size of the line we just read and copies it into temp so that it can be token'd in a few lines
        int size = (int) strlen(line);
        char temp[size];
        strcpy(temp, line);

        token = strtok_r(temp, " ", &save2); //Tokens the first String in line
        while(token != NULL) {
            if(strcmp(token, "proc") == 0) {
                currPID++; //New process is made so increment PID
            //    printf("new process: %d\n", currPID);

                int procPrio = (int)strtol((token = strtok_r(NULL, " ", &save2)), NULL, 10); //This line will get Process Priority from a line we are tokening as well as save the new token's string in token
            //    printf("Process Prio: %s\n", token);
                token = strtok_r(NULL, " ", &save2);
           //     printf("Amount of CPU/IO bursts: %s\n", token);

                int burst = (int)strtol(token, NULL, 10); //The amount of cpu/io bursts on the line
                cpuBurstA = (int*)calloc((burst/2)+1, sizeof(int)); //CPU will always have one more burst than io burst
                ioBurstA = (int*)calloc((burst/2), sizeof(int));
                int sBurst, x = 0, y = 0, count = 0;

                while(token != NULL){
                    sBurst = (int)strtol((token = strtok_r(NULL, " ", &save2)), NULL, 10); //Gets next burst value and stores the string in char token and the int value in sBurst

                    if(strtol(token, NULL, 10) != 0) { //Extra white space on 2nd line in input file after the 40 gives us an extra burst. This is needed to circumvent that
                        if (count % 2 == 0) { //
                            cpuBurstA[x] = sBurst;
                         //   printf("CPU burst: %d\n", cpuBurstA[x]);
                            x++;
                        } else {
                            ioBurstA[y] = sBurst;
                          //  printf("IO burst: %d\n", ioBurstA[y]);
                            y++;
                        }
                        count++;
                        //printf("Count is: %d\n", count);
                    }

                }
                struct PCB *n = newPCB(currPID, procPrio, cpuBurstA, ioBurstA); //Creates new PCB to be stored into the Ready_Q
                n->numCPUBurst = (burst/2) + 1;
                n->numIOBurst = (burst/2);
                clock_gettime(CLOCK_MONOTONIC, &n->ts_begin); //Start time

                addToBack(Ready_Q, n); //Add to front of doubly linked list
               // printPCB(Ready_Q);
               // printForward(Ready_Q);
               // printf("made it to sem_post\n");
                sem_post(&sem_cpu); //For CPU scheduler thread
            }
            else if(strcmp(token, "sleep") == 0) {
                token = strtok_r(NULL, " ", &save2);
                int ms = (int)strtol(token, NULL, 10);
                //printf("sleeping for %dms\n", ms);
                usleep(ms*1000);
                token = strtok_r(NULL, "\n", &save2);
            }
            else if(strcmp(token, "stop") == 0) {
               // printf("we are done!\n");
                break;
            }
        }
        line = strtok_r(NULL, "\n", &save1);
    }
    file_read_done = 1;
   // printf("END OF FIRST THREAD\n");
    free(fileData);
    fclose(fp);
    //free(cpuBurstA);
    //free(ioBurstA);
    return NULL;
}

void *CPU_scheduler_FIFO(void *args){
   // printf("START OF SECOND THREAD\n");
    struct PCB *PCB_cpu;
    quantum *RR = (quantum *)args;
  //  printf("Algorithm for rn: %s\n", algo);
    while(true){
        if(LinkedListIsEmpty(Ready_Q) && !cpu_busy && LinkedListIsEmpty(IO_Q) && !io_busy && file_read_done == 1)
            break;
       // printf("BEFORE ALGO IF\n");
        if(strcmp(RR->algo, "FIFO") == 0){
           //printf("IN ALGO IF\n");
            int res = sem_timedwait(&sem_cpu, (const struct timespec *) &atimespec.tv_sec);
            if(res == -1 && errno == ETIMEDOUT) continue;
            cpu_busy = 1;

            PCB_cpu = removeFromFront(Ready_Q);
            usleep(PCB_cpu->CPUBurst[PCB_cpu->cpuIndex]*1000);
            PCB_cpu->cpuIndex++;
           // printf("NUM OF CPU BURSTS FOR PROGRAM %d: %d\n", PCB_cpu->PID, PCB_cpu->numCPUBurst);
           // printf("CPU INDEX: %d\n", PCB_cpu->cpuIndex);
            if(PCB_cpu->cpuIndex >= PCB_cpu->numCPUBurst){
                clock_gettime(CLOCK_MONOTONIC, &PCB_cpu->ts_end);
                //Calculate turnaround for PCB
                PCB_cpu->elapsed = (double) PCB_cpu->ts_end.tv_sec - (double) PCB_cpu->ts_begin.tv_sec;
                PCB_cpu->elapsed += (PCB_cpu->ts_end.tv_nsec - PCB_cpu->ts_begin.tv_nsec) / 1000000000.0;

                //free(PCB_cpu);
                addToBack(done_Q, PCB_cpu);

                cpu_busy = 0;
            }else{
            //    printf("ADDED PCB TO IO QUEUE\n");
                addToBack(IO_Q, PCB_cpu);
                cpu_busy = 0;
                sem_post(&sem_io);
            }
        } else if(strcmp(RR->algo, "SJF") == 0){
            int res = sem_timedwait(&sem_cpu, (const struct timespec *) &atimespec.tv_sec);
            if(res == -1 && errno == ETIMEDOUT) continue;
            cpu_busy = 1;

            PCB_cpu = GetLinkedListElementMin(Ready_Q);

            usleep(PCB_cpu->CPUBurst[PCB_cpu->cpuIndex]*1000);
            PCB_cpu->cpuIndex++;

            if(PCB_cpu->cpuIndex >= PCB_cpu->numCPUBurst){
                clock_gettime(CLOCK_MONOTONIC, &PCB_cpu->ts_end);
                //Calculate turnaround for PCB
                PCB_cpu->elapsed = (double) PCB_cpu->ts_end.tv_sec - (double) PCB_cpu->ts_begin.tv_sec;
                PCB_cpu->elapsed += (PCB_cpu->ts_end.tv_nsec - PCB_cpu->ts_begin.tv_nsec) / 1000000000.0;

                //free(PCB_cpu);
                addToBack(done_Q, PCB_cpu);

                cpu_busy = 0;
            }else{
                //printf("ADDED PCB TO IO QUEUE\n");
                addToBack(IO_Q, PCB_cpu);
                cpu_busy = 0;
                sem_post(&sem_io);
            }
        } else if(strcmp(RR->algo, "PR") == 0){
            int res = sem_timedwait(&sem_cpu, (const struct timespec *) &atimespec.tv_sec);
            if(res == -1 && errno == ETIMEDOUT) continue;
            cpu_busy = 1;

            PCB_cpu = GetLinkedListElementMax(Ready_Q);

            usleep(PCB_cpu->CPUBurst[PCB_cpu->cpuIndex]*1000);
            PCB_cpu->cpuIndex++;

            if(PCB_cpu->cpuIndex >= PCB_cpu->numCPUBurst){
                clock_gettime(CLOCK_MONOTONIC, &PCB_cpu->ts_end);
                //Calculate turnaround for PCB
                PCB_cpu->elapsed = (double) PCB_cpu->ts_end.tv_sec - (double) PCB_cpu->ts_begin.tv_sec;
                PCB_cpu->elapsed += (PCB_cpu->ts_end.tv_nsec - PCB_cpu->ts_begin.tv_nsec) / 1000000000.0;

                //free(PCB_cpu);
                addToBack(done_Q, PCB_cpu);

                cpu_busy = 0;
            }else{
                //printf("ADDED PCB TO IO QUEUE\n");
                addToBack(IO_Q, PCB_cpu);
                cpu_busy = 0;
                sem_post(&sem_io);
            }
        } else if(strcmp(RR->algo, "RR") == 0){
            int res = sem_timedwait(&sem_cpu, (const struct timespec *) &atimespec.tv_sec);
            if(res == -1 && errno == ETIMEDOUT) continue;
            cpu_busy = 1;

            PCB_cpu = removeFromFront(Ready_Q);

            int qTime = PCB_cpu->CPUBurst[PCB_cpu->cpuIndex] - RR->quan;
            if(qTime > 0) {
                usleep(RR->quan * 1000);
                PCB_cpu->CPUBurst[PCB_cpu->cpuIndex] = qTime;
            } else if(qTime == 0){
                usleep(RR->quan * 1000);
                PCB_cpu->CPUBurst[PCB_cpu->cpuIndex] = 0;
                PCB_cpu->cpuIndex++;
            } else if(qTime < 0){
                usleep(PCB_cpu->CPUBurst[PCB_cpu->cpuIndex] * 1000);
                PCB_cpu->CPUBurst[PCB_cpu->cpuIndex] = 0;
                PCB_cpu->cpuIndex++;
            }

            if(PCB_cpu->cpuIndex >= PCB_cpu->numCPUBurst){
                clock_gettime(CLOCK_MONOTONIC, &PCB_cpu->ts_end);
                //Calculate turnaround for PCB
                PCB_cpu->elapsed = (double) PCB_cpu->ts_end.tv_sec - (double) PCB_cpu->ts_begin.tv_sec;
                PCB_cpu->elapsed += (PCB_cpu->ts_end.tv_nsec - PCB_cpu->ts_begin.tv_nsec) / 1000000000.0;

                //free(PCB_cpu);
                addToBack(done_Q, PCB_cpu);

                cpu_busy = 0;
            }else{
                //printf("ADDED PCB TO IO QUEUE\n");
                addToBack(IO_Q, PCB_cpu);
                cpu_busy = 0;
                sem_post(&sem_io);
            }
        }
    }
    cpu_sch_done = 1;
    //free(PCB_cpu);
  //  printf("END OF SECOND THREAD\n");
    return NULL;
}

void *IO_system_thread(void *args){
    struct PCB *PCB_io;
    while(true){
        if(LinkedListIsEmpty(Ready_Q) && !cpu_busy && LinkedListIsEmpty(IO_Q) && file_read_done == 1)
            break;
        while(LinkedListIsEmpty(IO_Q)) {
            if(LinkedListIsEmpty(Ready_Q) && cpu_busy == 0 && io_busy == 0 && file_read_done == 1) {
                io_sys_done = 1;
               // printf("END OF THIRD THREAD\n");

                pthread_exit(0);
            }
            usleep(1000);
        }
        int res = sem_timedwait(&sem_io, (const struct timespec *) &atimespec.tv_sec);
     //   printf("PRINT RES: %d\n", res);

        if(res == -1 && errno == ETIMEDOUT) continue;
        io_busy = 1;

        PCB_io = removeFromFront(IO_Q);
        usleep(PCB_io->IOBurst[PCB_io->ioIndex]*1000);
        PCB_io->ioIndex++;
        addToBack(Ready_Q, PCB_io);
        io_busy = 0;
        sem_post(&sem_cpu);
    }
    io_sys_done = 1;
    //free(PCB_io);
    return NULL;
}