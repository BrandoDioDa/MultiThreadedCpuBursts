//
// Created by Brandon on 8/1/2022.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dLinkedList.h"

PCB *newPCB(int processId, int processPrio, int *CPUburst, int *IOBurst){
    PCB *element;

    element = (PCB*) calloc(1, sizeof(PCB));
    if(!element){
        fprintf(stderr, "Node cannot allocate memory\n");
        return NULL;
    }
    element->PID = processId;
    element->processPrio = processPrio;
    element->CPUBurst = CPUburst;
    element->IOBurst = IOBurst;

    return element;
}

linked_list *newLinkedList(void){
    linked_list *list;

    list = (linked_list*) malloc(sizeof(linked_list));
    if(!list){
        fprintf(stderr, "NewLinkedList cannot allocate memory\n");
        return NULL;
    }
    list->head = NULL;
    list->tail = NULL;

    return list;
}

void FreeLinkedList(linked_list *list){
    while(!LinkedListIsEmpty(list)) {
        free(removeFromFront(list));
    }
    if(LinkedListIsEmpty(list)) {
        free(list);
    }
}
void printForward(linked_list *list) {
    PCB *node = list->head;
    while(node != NULL) {
        if(node->next != NULL) {
            printf("%d -> ", node->PID);
        }
        else {
            printf("%d\n", node->PID);
        }
        node = node->next;
    }
}

void printBackward(linked_list *list) {
    PCB *node = list->tail;
    while(node != NULL) {
        if(node->prev != NULL) {
            printf("%d <- ", node->PID);
        }
        else {
            printf("%d\n", node->PID);
        }
        node = node->prev;
    }
}

void printPCB(linked_list *list){
    PCB *node = list->head;
    if(LinkedListIsEmpty(list)){
        printf("THIS LINKED LIST IS EMPTY\n");
    }
    while(node != NULL){
        printf("PROCESS %d IN THE LIST VALUES: \n", node->PID);
        printf("Process Prio: %d\n", node->processPrio);
        printf("CPU Bursts: ");
        for(int i = 0; i < node->numCPUBurst; i++)
            printf("%d ", node->CPUBurst[i]);
        printf("\nIO Bursts: ");
        for(int j = 0; j < node->numIOBurst; j++)
            printf("%d ", node->IOBurst[j]);
        printf("\nTurn around time: %.2f", (node->elapsed) * 1000);
        printf("\n\n");
        node = node->next;
    }
}

void addToBack(linked_list *list, PCB *element){
    if(list->head == NULL && list->tail == NULL) {
        list->head = list->tail = element;
        list->head->next = NULL;
        list->head->prev = NULL;
    }
    else{
        list->tail->next = element;
        element->prev = list->tail;
        list->tail = element;
        list->tail->next = NULL;
    }
}

void addToFront(linked_list *list, PCB *element){
    if(list->head == NULL && list->tail == NULL) {
        list->head = list->tail = element;
        element->next = NULL;
        element->prev = NULL;
    }
    else{
        element->next = list->head;
        list->head->prev = element;
        list->head = element;
        list->head->prev = NULL;
    }
}

void insertAtIndex(linked_list *list, PCB *element, int key){
    struct PCB *nextNode;
    struct PCB *previous = list->head;
    if(list->head == NULL)
        list->head = element;
    else{
        for(int i = 0; i < key; i++)
            previous = previous->next;
        nextNode = previous->next;
        nextNode->prev = element;
        previous->next = element;
        element->next = nextNode;
    }
}

PCB *removeFromFront(linked_list *list){
    PCB *node = list->head;
    if(node == list->head && node == list->tail) {
        list->head = NULL;
        list->tail = NULL;
        node->next = NULL;
        node->prev = NULL;
        return node;
    }
    if(list->head != NULL) {
        list->head = (list->head)->next;
        (list->head)->prev = NULL;
        return node;
    }
    return NULL;
}

PCB *removeFromEnd(linked_list *list){
    PCB *node = list->tail;
    if(node == list->tail && node == list->head) {
        list->head = NULL;
        list->tail = NULL;
        node->next = NULL;
        node->prev = NULL;
        return node;
    }
    if(list->tail != NULL) {
        list->tail = (list->tail)->prev;
        (list->tail)->next = NULL;
        return node;
    }
    return NULL;
}
 void deleteElement(linked_list *list, struct PCB *toFree){
    if(toFree == list->head && toFree == list->tail){
        list->head = NULL;
        list->tail = NULL;
    } else if(toFree == list->head){
        list->head = toFree->next;
        list->head->prev = NULL;
    } else if(toFree == list->tail){
        list->tail = toFree->prev;
        list->tail->next = NULL;
    } else if(toFree->next != NULL && toFree->prev != NULL){
        toFree->next->prev = toFree->prev;
        toFree->prev->next = toFree->next;
    } else
        printf("ERROR. FAILED TO DELETE!\n");
    toFree->prev = NULL;
    toFree->next = NULL;
}

PCB *removeElement(linked_list *list, int index){
    PCB *temp = list->head;
    if(index == 0) {
        return removeFromFront(list);
    }
    if(index == LinkedListLength(list)) {
        return removeFromEnd(list);
    }
    for(int x = 0; x < index && temp != NULL; x++) {
        temp = temp->next;
    }
    //printf("temp: %s\n", temp->name);
    temp->next->prev = temp->prev;
    temp->prev->next = temp->next;
    temp->next = NULL;
    temp->prev = NULL;
    return temp;
}

int LinkedListIsEmpty(linked_list *list){
    if(list->head == NULL && list->tail == NULL)
        return 1;
    return 0;
}

int LinkedListLength(linked_list *list){
    int count = 0;
    struct PCB *newNode = list->head;
    while (newNode != NULL){
        count++;
        newNode = newNode->next;
    }
    return count;
}

//I modified this function for SJF, This function now searches for the PCB with the lowest CPUburst
PCB *GetLinkedListElementMin(linked_list *list){
    int lowest;
    if(!LinkedListIsEmpty(list))
        lowest = list->head->numCPUBurst;
    else
        printf("List is empty, cannot get min value.\n");
    PCB *node = list->head;
    PCB *node2 = node;
  //  printf("Process %d has the lowest numBurst with: %d\n", node->PID, node2->numCPUBurst);

    if(node == list->head && node == list->tail) {
        list->head = NULL;
        list->tail = NULL;
        node->next = NULL;
        node->prev = NULL;
        return node;
    }

    while(node->next != NULL){
        if(node->numCPUBurst < lowest) {
            lowest = node->numCPUBurst;
            node2 = node;
           // printf("Process %d has the lowest numBurst with: %d\n", node2->PID, node2->numCPUBurst);
        }
        node = node->next;
    }
    if(node2 == list->head){
        list->head = (list->head)->next;
        (list->head)->prev = NULL;
    } else if(node2 == list->tail){
        list->tail = (list->tail)->next;
        (list->tail)->prev = NULL;
    } else{
        (node2->next)->prev = node2->prev;
        (node2->prev)->next = node2->next;
        node2->next = NULL;
        node2->prev = NULL;
    }
    return node2;
}
PCB *GetLinkedListElementMax(linked_list *list){
    int highest;
    if(!LinkedListIsEmpty(list))
        highest = list->head->processPrio;
    else
        printf("List is empty, cannot get min value.\n");
    PCB *node = list->head;
    PCB *node2 = node;
    //  printf("Process %d has the lowest numBurst with: %d\n", node->PID, node2->numCPUBurst);

    if(node == list->head && node == list->tail) {
        list->head = NULL;
        list->tail = NULL;
        node->next = NULL;
        node->prev = NULL;
        return node;
    }

    while(node->next != NULL){
        if(node->processPrio > highest) {
            highest = node->processPrio;
            node2 = node;
           //  printf("Process %d has the lowest numBurst with: %d\n", node2->PID, node2->processPrio);
        }
        node = node->next;
    }
    if(node2 == list->head){
        list->head = (list->head)->next;
        (list->head)->prev = NULL;
    } else if(node2 == list->tail){
        list->tail = (list->tail)->next;
        (list->tail)->prev = NULL;
    } else{
        (node2->next)->prev = node2->prev;
        (node2->prev)->next = node2->next;
        node2->next = NULL;
        node2->prev = NULL;
    }
    return node2;
}