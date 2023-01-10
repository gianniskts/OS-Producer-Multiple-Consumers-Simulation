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
#include <sys/time.h>
using namespace std;
#include "child.h"

void child(int segmentID, int numberOfRequests, int linesPerSegment, int numberOfSegments, int lengthOfLine, SharedMemory sharedMem, char* buffer) {
    // each child requests a segment and a line of the segment
    int targetSegment;
    int targetLine;

    // create child output file
    FILE* f1;
    char output_file[60];
    char idToString[20];
    strcpy(output_file, "child_");
    sprintf(idToString, "%d", segmentID);
    strcat(output_file, idToString);
    char type[5] = ".txt";
    strcat(output_file, type);
    if ((f1 = fopen(output_file, "w")) == NULL) {
        cout << "ERROR: Creating Child File" << endl;
        exit(-8);
    }

    // the buffer where the targetLine will be stored
    // targetLine will be copied from the Shared Memory to target_buffer
    char* target_buffer = (char*) malloc(sizeof(char) * (lengthOfLine + 1));
    unsigned int curtime = time(NULL);
    // re-initialize the rand, to produce different results on every process
    srand((unsigned int) curtime - getpid());

    int i = 0;
    do {
        // Select a random line in [0, linesPerSegment)
        targetLine = random_line(0, linesPerSegment-1);
        
        // if first time
        if (i == 0) {
            // Select segment in [1, numberOfSegments]
            targetSegment = random_segment(1, numberOfSegments);
        } else { // P(new) == 0.3 && P(old) == 0.7
            double probability = 0.3;
            double result = rand() / RAND_MAX;
            if (result <= probability) {
                targetSegment = random_segment(1, numberOfSegments);
                targetLine    = random_line(0, linesPerSegment-1);
            }
        }

        struct timespec start, finish;
        
        sem_wait(&(sharedMem->semPerSegMutex[targetSegment-1]));
        sharedMem->countOfReaders[targetSegment-1]++;
        if (sharedMem->countOfReaders[targetSegment-1] == 1) {
            sem_wait(&sharedMem->FIFOmutex);
        }
        
        // start time
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
            cout << "ERROR: Time" << endl;
            return;
        }    
        // request segment and line
        sem_wait(&(sharedMem->write));

        sharedMem->segmentRequested = targetSegment;
        sharedMem->lineRequested = targetLine;
        
        sem_post(&(sharedMem->request));
        sem_wait(&(sharedMem->wait_response));
        sem_post(&sharedMem->write);

        sem_post(&(sharedMem->semPerSegMutex[targetSegment-1]));
        // finish time
        if (clock_gettime(CLOCK_MONOTONIC, &finish) == -1) {
            cout << "ERROR: Time" << endl;
            return;
        }    
        // Read from SharedMemory memory
        strcpy(target_buffer, buffer);
        sleep(0.2);

        sem_wait(&(sharedMem->semPerSegMutex[targetSegment-1]));
        // print startTime - finishTime <targetSegment, targetLine> - line
        fprintf(f1, "%f - %f \t < %d, %d > - \t - %s\n", start.tv_sec + (start.tv_nsec*1.0e-9), 
                finish.tv_sec + (finish.tv_nsec*1.0e-9) , targetSegment, (targetLine+1), buffer);
        sharedMem->countOfReaders[targetSegment-1]--;
        if (sharedMem->countOfReaders[targetSegment-1] == 0) {
            // change the segment
            cout << "Segment " << targetSegment << " from Child " << segmentID <<  " OK" << endl;
            sem_post(&sharedMem->FIFOmutex);
        }
        sem_post(&(sharedMem->semPerSegMutex[targetSegment-1]));
        i++;

    } while (i < numberOfRequests);

    free(target_buffer);

}