# Operating Systems
## OS-Producer-Multiple-Consumers-Simulation
Multiple forked children ask for data from the parent process through a shared memory segment using semaphores. User inputs a big text file and each child asks for a random segment and line of the given file.

## The project consists of a total of 7 modules to better organize the code, which are presented below.
    1. parent.cpp
    2. child.h
    3. child.cpp
    4. utils.h
    5. utils.cpp
    6. makefile
    7. text.txt

The task is implemented in C++. The STL library has not been used, as it is mainly using C libraries.

## Execution
    1. In terminal open the current directory of the task with the appropriate cd's and execute the following commands
    2. make
    3. ./parent <file> <N> <Partition_Location> <Number_of_questions_per_field>
Indicative execution: ./parent text.txt 10 10 5

## Output
    1. For each child process a log file named child_N is generated, where N is the child ID number.
    2. The parent process produces a log file named parent_file.txt.
    3. Each child log file contains the request time, response time, requested segment number, requested line number and requested line from the file.
    4. The parent job log contains the segment number, line number, request time and response time.
## Implementation
utils
The following are implemented in this file:
    1. struct shared memory. This contains:
        a. The semPerSegMutex size (sem) * numberOfSegments semaphores table which contains one semaphores for each segment
        b. The FIFOmutex semaphore whose use is for children processes to be served in FIFO order
        c. The request semaphore
        d. The wait_response semaphore
        e. The write semaphore
        f. The integer table countOfReaders of size (int)*numberOfSegments containing the number of readers for each segment
        g. The integer currentReaderCounter, which contains the total number of readers
        h. The integer segmentRequested, which indicates the requested number of segments
        i. The integer lineRequested, which indicates the requested number of the line
        j. The integer currentSegment, indicating the current number of the segment in the shared memory
    2. char*** start(char* argv, int linesPerSegment, int* numberOfLines, int* lengthOfLine)
        a. Returns to the parent the 3x3 array char*** arrayOfSegments[segment][linesOfSegment][line] which splits the input file into segments for a cleaner and clearer implementation
        b. After the array is created and the input file is copied to it then it is deleted to avoid overloading the memory with unnecessary data
        c. Returns to the parent the variable numberOfLines which defines the maximum number of lines in the input file
        d. Returns to the parent the variable lengthOfLine which defines the maximum number of characters per line in the input file

    3. int random_segment(int min, int max)
        a. Returns an integer in the range [min, max].
    4. int random_line(int min, int max)
        a. Returns an integer in the range [min,max)


child
void child(int segmentID, int numberOfRequests, int linesPerSegment, int numberOfSegments, int lengthOfLine, SharedMemory sharedMem, char* buffer)

    1. Implements the child process
    2. Takes as arguments
        a. Identification number of the given segment
        b. Request number for each child
        c. Number of lines of each segment
        d. Total number of segments
        e. Total number of characters per line
        f. Index in shared memory
        g. Index in the shared memory table that transfers the requested line from the parent process to the child
    3. Creates a new log file named child_N with N in the range [0, N) which will be exploited at the end of the process by the child
    4. Randomly selects requested line number of segment
    5. Randomly selects requested segment number with probability 70% same as the previous one and 30% different
    6. Implements the Readers - Writers synchronization model with semPerSegMutex[targetSegment], write, request, wait_response semPerSegMutex[targetSegment], write, request, wait_response
    7. Copies to its own target_buffer the contents of the requested line from the shared memory buffer
    8. Calculates the request submission time and response time
    9. It prints the above times, the target segment and line numbers and finally the requested line to the log file

parent

    1. Creates the shared memory
    2. Initializes the contents of the
        a. Semaphores
        b. Tables
        c. Integers
    3. Creates the shared memory buffer that carries requests from the parent process to children
    4. Starts N child processes
    5. Creates log file parent_file.txt 
    6. Prints in it the last segment and line numbers requested and the service times of child processes
    7. Copies the request to the shared buffer
    8. Gives order to the child process
    9. Waits for all children processes to finish
    10. Destroys shared memory and its contents
    11. Destroys the shared memory buffer
    12. Destroys the table of segments of the input file
    13. End of work.
