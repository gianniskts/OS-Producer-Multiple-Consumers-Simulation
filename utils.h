#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <stdio.h>
using namespace std;
#define MAX_LENGTH 100

struct shared_memory {
    sem_t* semPerSegMutex;
    sem_t  FIFOmutex;
    sem_t  request;
    sem_t  wait_response;
    sem_t  write;
    int*   countOfReaders;
    int    currentReaderCounter;
    int    segmentRequested;
    int    lineRequested;
    int    currentSegment;
};
typedef struct shared_memory* SharedMemory;

char*** start(char* argv, int linesPerSegment, int* numberOfLines, int* lengthOfLine);

int random_segment(int min, int max);

int random_line(int min, int max);


