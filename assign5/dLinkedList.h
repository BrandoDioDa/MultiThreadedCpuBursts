//
// Created by Brandon on 8/1/2022.
//
#include <sys/time.h>
#ifndef ASSIGN5_DLINKEDLIST_H
#define ASSIGN5_DLINKEDLIST_H
typedef struct PCB {
    //Essential variables laid out in helper document
    int PID, PR;
    int numCPUBurst, numIOBurst, cpuIndex, ioIndex, cpuBurst, processPrio;
    int *CPUBurst, *IOBurst;

    struct timespec ts_begin, ts_end;
    struct PCB *prev, *next;
    //Other variables go below
    int waitTime, queueClock;
    double elapsed;

} PCB;

typedef struct linked_list{
    struct PCB *head;
    struct PCB *tail;
}linked_list;

PCB *newPCB(int processId, int processPrio, int *CPUburst, int *IOBurst); //Initializes new PCB

linked_list *newLinkedList(void); //allocates and returns an empty linked list

void FreeLinkedList(linked_list *list); //This function frees the storage associated with list

void addToBack(linked_list *list, PCB *element); //This function adds a node pointed by an element to the end of the list

void addToFront(linked_list *list, PCB *element);

void insertAtIndex(linked_list *list, PCB *element, int key);

PCB *removeFromFront(linked_list *list); //removes element at front of list

PCB *removeFromEnd(linked_list *list); //removes element at end of list

PCB *removeElement(linked_list *list, int index);

void deleteElement(linked_list *list, struct PCB *toFree);

int LinkedListIsEmpty(linked_list *list); //tests whether the list is empty or full

int LinkedListLength(linked_list *list); //returns number of elements in the list

PCB *GetLinkedListElementMin(linked_list *list); //I modified this function for SJF, This function now searches for the PCB with the lowest CPUburst

PCB *GetLinkedListElementMax(linked_list *list);

void printForward(linked_list *list);

void printBackward(linked_list *list);

void printPCB(linked_list *list);
#endif //ASSIGN5_DLINKEDLIST_H
