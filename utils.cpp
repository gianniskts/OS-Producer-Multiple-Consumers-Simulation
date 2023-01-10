#include "utils.h"
#define MAX_LENGTH 100

char*** start(char* argv, int linesPerSegment, int* numberOfLines, int* lengthOfLine) {
    // open file
    ifstream file;
    file.open((argv));
    FILE* fp = fopen(argv, "r");

    if (!file || !fp) {
        cout << "ERROR: Could not open file\n";
        exit (-1);
    } else {
        cout << "File " << argv << " has opened successfully\n";
    }

    char line[MAX_LENGTH]; // temporary array
    // find the count of rows, max string length
    int rows=0, cols=0, maxLength;
    while(fgets(line, 30, fp) != NULL) {
        rows++;
        maxLength = strlen(line);
        if (maxLength > cols)    cols = maxLength;
    }
    *numberOfLines = rows;
    *lengthOfLine = maxLength;
    // close file for counting rows, maxLength
    fclose(fp);

    cout << rows << endl;
    cout << maxLength << endl;

    // copy file to array
    char array[rows][maxLength];
    // initialize array
    for (int i=0; i<rows; i++) {
        for (int j=0; j<maxLength; j++) {
            array[i][j] = '\0';
        }
    }
    // copy 
    string eachRow;
    int row=0;
    while (getline(file, eachRow)) {
        int column=0;
        for (char& ch : eachRow) {
            if (ch != '\n') {
                array[row][column] = ch;
                column++;
            }
        }
        row++;
    }
    // close file
    file.close();

    // array into segments
    // char arrayOfSegments[rows/linesPerSegment][rows][columns];
    char*** arrayOfSegments = new char**[rows/linesPerSegment];

    for (int i=0; i<rows/linesPerSegment; i++) {
        arrayOfSegments[i] = new char*[rows];
        for (int j=0; j<rows; j++) {
            arrayOfSegments[i][j] = new char[maxLength];
        }
    }
    // initialize array
    for (int i=0; i<rows/linesPerSegment; i++) {
        for (int j=0; j<rows; j++) {
            for (int k=0; k<maxLength; k++) {
                arrayOfSegments[i][j][k] = '\0';
            }
        }
    }
    // copy file from array to the useful arrayOfSegments
    int x=0;
    for (int i=0; i<rows/linesPerSegment; i++) {
        for (int j = 0; j<linesPerSegment; j++) {
                // arrayOfSegments[i][j] = array[x];
                strcpy(arrayOfSegments[i][j], array[x]);
                x++;        
        }
    }    
    // print segments
    for (int i = 0; i < rows/linesPerSegment; i++) {
        cout << "Segment: " << i << endl;
        for (int j=0; j < linesPerSegment; j++) {
            cout << arrayOfSegments[i][j] << endl;
        }
        cout << endl;
    }

    return arrayOfSegments;
}

int random_segment(int min, int max) {
    return (rand() % (min - max + 1)) + 1;
}

int random_line(int min, int max) {
    return (rand() % (max - min + 1));
}