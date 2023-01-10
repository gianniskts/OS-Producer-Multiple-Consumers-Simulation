#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#include "utils.h"
#include "child.h"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

using namespace std;

int main(int argc, char* argv[]) {
    srand(time(NULL));
    
    // example run args
    // ./parent text.txt 2 2 5
    int N                = atoi(argv[2]); 
    int linesPerSegment  = atoi(argv[3]); 
    int numberOfRequests = atoi(argv[4]);
    int numberOfSegments = 0; 
    int numberOfLines    = 0;
    int lengthOfLine     = 0;

    // parent process 
    // arrayOfSegments contains [segment][rowsPerSegment][line]
    char*** arrayOfSegments;
    // args argv[1] is the input file
    arrayOfSegments = start(argv[1], linesPerSegment, &numberOfLines, &lengthOfLine);

    numberOfSegments = numberOfLines / linesPerSegment;
    cout << "Total number of Segments: " << numberOfSegments << endl;
    cout << "Lines per Segment: "        << linesPerSegment  << endl;
    cout << "Total number of Lines: "    << numberOfLines    << endl;
    
    int shmID = shmget(IPC_PRIVATE, sizeof(SharedMemory), 0777 | IPC_CREAT);
    if (shmID == -1) { 
        cout << "ERROR: Create SharedMemory Memory" << endl;
        return -1;
    }

    SharedMemory sharedMem = (SharedMemory) shmat(shmID, NULL, 0);
    if (sharedMem == (void*)-1) {
        cout << "ERROR: Attach ShareMemory" << endl;
        return -2;
    }
    
    sem_init(&(sharedMem->FIFOmutex), 1 , 1);
    sem_init(&(sharedMem->request), 1, 0);
    sem_init(&(sharedMem->wait_response), 1 , 0);
    sem_init(&(sharedMem->write), 1, 1);

    // initialize sem_t* semPerSegArray 
    int semSegArrID;
    semSegArrID = shmget(IPC_PRIVATE, sizeof(sem_t) * numberOfSegments, O_CREAT | SEM_PERMS);
    sharedMem->semPerSegMutex = (sem_t *) shmat(semSegArrID, NULL, 0);
    if (sharedMem->semPerSegMutex == (void*)-1) {
        cout << "ERROR: Shared semaphorePerSegment Array" << endl;
        return -3;
    }
    for(int i = 0; i < numberOfSegments; i++) {
        sem_init(&(sharedMem->semPerSegMutex[i]), 1, 1);
    }

    sharedMem->currentSegment       = -1;
    sharedMem->currentReaderCounter =  0;

    // initialize the readCount array
    int readID = shmget(IPC_PRIVATE, sizeof(int) * numberOfSegments, O_CREAT | SEM_PERMS);
    sharedMem->countOfReaders = (int*) shmat(readID, NULL, 0);
    for (int i = 0; i < numberOfSegments; i++) {
        sharedMem->countOfReaders[i] = 0;
    } 

    // Buffer which carries the lines between child - parent
    int bufferID = shmget(IPC_PRIVATE, sizeof(char) * (linesPerSegment+1), O_CREAT | SEM_PERMS);
    char* buffer = (char*) shmat(bufferID, NULL, 0);
    if (buffer == (void*)-1) {
        cout << "ERROR: Create Shared Button" << endl;
        return -4;
    }

    // create child processes
    for (int i = 0; i < N; i++) {
        if (fork() == 0) {
            child(i, numberOfRequests, linesPerSegment, numberOfSegments, lengthOfLine, sharedMem, buffer); 
            exit(0);
        } 
    }

    int lastRequested = -1;
    int lineRequested = -1;
    FILE* f = fopen("parent_file.txt", "w");
    if (f == NULL) {
        cout << "ERROR: Create parent file" << endl;
        exit(-7);
    }

    long start   = -1;
    long finish  = -1;
    bool changed = false;
    bool first   = false;

    for(int i = 0; i < N * numberOfRequests; i++) {  
        sem_wait(&(sharedMem->request));
        cout << endl << "--------- Parent Process ---------" << endl;
        
        lineRequested = sharedMem->lineRequested;
        if (lastRequested != sharedMem->segmentRequested) changed = true;
        else    changed = false;
        if (lastRequested == -1)                          first = true;
        else    first = false;
        // only if the next segment is different from the previous or 
        // it is just the first try
        if (changed || first) {

            if (first) {
                start = time(NULL);
            } else {
                finish = time(NULL);
                fprintf(f, "Segment %d Line %d start: <%ld> - \t end: <%ld>\n", lastRequested, lineRequested+1, start, finish);
                start = time(NULL);
            }

            printf("Last Requested = %d, Current: %d\n", lastRequested, sharedMem->segmentRequested);
            lastRequested = sharedMem->segmentRequested;
            // copy the requested line from the main array to the Shared Memory
            strcpy(buffer, arrayOfSegments[sharedMem->segmentRequested][sharedMem->lineRequested]);
        } else {
            printf("Segment %d requested is already in SharedMemory memory\n", sharedMem->segmentRequested);
        }
        cout << "--------- Child Process ---------" << endl;
        sem_post(&(sharedMem->wait_response));
    }
    if (start != -1)
        fprintf(f, "Segment %d Line %d start <%ld> - end: <%ld>\n", lastRequested, lineRequested,start, finish);
    else
        fprintf(f, "Segment %d Line %d start <%ld> - end: ERROR", lastRequested, lineRequested,start);

    fclose(f);
    
    // finish all processes
    for (int i = 0; i < N; i++) {
        wait(0);
    }

    // closing the program
    sem_destroy(&(sharedMem->FIFOmutex));
    sem_destroy(&(sharedMem->request));
    sem_destroy(&(sharedMem->wait_response));
    sem_destroy(&(sharedMem->write));

    shmdt(sharedMem->countOfReaders);
    shmctl(readID, IPC_RMID, 0);

    // destroy the dynamic allocated char*** array
    for (int i=0; i<numberOfSegments; i++) {
        for (int j=0; j<numberOfLines; j++) {
            free(arrayOfSegments[i][j]);
        }
    }
    // destroy the array in the Shared Memory
    for (int i = 0; i < numberOfSegments; i++) {
        sem_destroy(&(sharedMem->semPerSegMutex[i]));
    }
    shmdt(sharedMem->semPerSegMutex);
    shmctl(semSegArrID, IPC_RMID, 0);

    shmdt(sharedMem);
    shmctl(shmID, IPC_RMID, 0);
    
    shmdt(buffer);
    shmctl(bufferID, IPC_RMID, 0);

    cout << endl << "--------- END OF PROGRAM ---------" << endl;

    return 0;
}