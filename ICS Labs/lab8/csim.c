/*
 * name: DengShiyi stu_no: 518021910184
 * 
 * csim.c - A simulator of cache following LRU strategy of eviction
 * 
 * The simulator counts number of hit & miss & evivtion with cache 
 * who has 2^s sets, E lines each set and 2^b bytes each block.
 * 
 * The adress is 64bit with (64 - s - b) tag bits, s set bits and
 * b offset bits.
 */

#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFLEN 1000
#define FILELEN 1000
#define ADDRLEN 64
#define BASE 16

int sBitNum, ENum, bBitNum, helpMode = 0, verboseMode = 0, clockTime = 0;
int hitCount = 0, missCount = 0, evictionCount = 0;

/*
 * Argumen cacheLine - Data structure of cacheLine.
 * Argumen timeStamp records time when this line is loaded,
 * read or modified.
 * Argument isValid is the valid bit.
 */
typedef struct{
    int timeStamp;
    int isValid;
    unsigned long tag;
} cacheLine;

cacheLine **cache = NULL;

/* 
 * LorS - Parse the address, update cacheLine and records number of
 * hit & miss & evivtion.
 * Argument address is the memory address.
 */
void LorS(unsigned long address){
    unsigned long setAddress;
    setAddress = (address >> bBitNum) & ((-1U) >> (ADDRLEN - sBitNum));
    unsigned long tag = (address >> (sBitNum + bBitNum));
    int emptyLine = -1;     // -1 means there is no empty line
    for (int i = 0; i < ENum; ++i){
        if(!cache[setAddress][i].isValid){
            emptyLine = i;
        }
        else if(cache[setAddress][i].tag == tag){   // hit
            hitCount++;
            cache[setAddress][i].timeStamp = clockTime;
            if (verboseMode){
                printf("hit ");
            }
            return;
        }
    }
    missCount++;
    if (verboseMode){
        printf("miss ");
    }
    if(emptyLine != -1){    // have an empty line
        cache[setAddress][emptyLine].isValid = 1;
        cache[setAddress][emptyLine].tag = tag;
        cache[setAddress][emptyLine].timeStamp = clockTime;
        return;
    }
    else{   // no empty line
        if (verboseMode){
            printf("eviction ");
        }
        evictionCount++;
        int minTime = ~(1 << 31);
        int minLine = -1;
        for (int i = 0; i < ENum; ++i){
            if(cache[setAddress][i].timeStamp < minTime){
                minTime = cache[setAddress][i].timeStamp;
                minLine = i;
            }
        }
        cache[setAddress][minLine].tag = tag;
        cache[setAddress][minLine].timeStamp = clockTime;
    }
}

/* 
 * parseBuffer - Parse the command from a disk file and load 
 * the argument to memory.
 * Argument buffer is a pointer pointing to command string.
 * Argument type points to command type like 'L', 'S' or 'M'.
 * Argument address points to the target address.
 * Argument tmp points to bit number which should be ignored. 
 */
void parseBuffer(const char * buffer, char * type, 
                            unsigned long * address, int *tmp){
    if(buffer[0]==' '){
        *type = buffer[1];
        *address = (unsigned long) strtoul(buffer + 3, NULL, BASE);
        *tmp = 0;
    }
    else{
        *type = 'I';
    }
}

/*
 * main - Main function of the simulator.
 * Argument argc is the length of argv.
 * Argument argv is an array holding command line arguments. 
 */
int main(int argc, char *argv[]){
    extern char *optarg;
    extern int optind, opterr, optopt;
    int opt, S, tmp;
    unsigned long address;
    char type;  //operation type
    char filename[FILELEN];
    char buffer[BUFLEN];
    // parse command line in argv and store arguments
    while((opt = getopt(argc, argv, ":hvs:E:b:t:")) != -1){
        switch(opt){
            case 'h':
                helpMode = 1;
                break;
            case 'v':
                verboseMode = 1;
                break;
            case 's':
                sBitNum = atoi(optarg);
                break;
            case 'E':
                ENum = atoi(optarg);
                break;
            case 'b':
                bBitNum = atoi(optarg);
                break;
            case 't':
                strcpy(filename, optarg);
                break;
            case '?':
                printf("unrecognized option\n");
                exit(0);
                break;
            case ':':
                printf("./csim-ref: Missing required command line argument\n");
                exit(0);
                break;
            }
    }
    // output help information
    if(helpMode){
        printf("Options:\n");
        printf("\t-h\t\tPrint this help message.\n");
        printf("\t-v\t\tOptional verbose flag.\n");
        printf("\t-s <num>   Number of set index bits.\n");
        printf("\t-E <num>   Number of lines per set.\n");
        printf("\t-b <num>   Number of block offset bits.\n");
        printf("\t-t <file>  Trace file.\n\n");
        printf("Examples:\n");
        printf("\tlinux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
        printf("\tlinux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
    }

    S = 1 << sBitNum;
    // initiate cache
    cache = (cacheLine **) malloc(sizeof(cacheLine *) * S);
    if(cache == NULL){
        printf("malloc failure\n");
        exit(0);
    }
    for (int i = 0; i < S;++i){
        cache[i] = (cacheLine *) malloc(sizeof(cacheLine) * ENum);
        if(cache[i] == NULL){
            printf("malloc failure\n");
            exit(0);
        }
    }
    for (int i = 0; i < S; ++i){
        for (int j = 0; j < ENum;++j){
            cache[i][j].isValid = 0;
            cache[i][j].timeStamp = 0;
        }
    }

    FILE *fp = fopen(filename, "r");
    if(fp == NULL){
        printf("Can't open file.\n");
        exit(0);
    }
    // read command from disk file and update cache
    while(fgets(buffer, BUFLEN, fp)){
        parseBuffer(buffer, &type, &address, &tmp);
        if(verboseMode){
            printf("%c %lx,%d ", type, address, tmp);
        }
        switch(type){
            case 'L':
                LorS(address);
                break;
            case 'M':
                LorS(address);
            case 'S':
                LorS(address);
                break;
        }
        if(verboseMode){
            printf("\n");
        }
        clockTime++;
    }

    for (int i = 0; i < S; ++i){
        free(cache[i]);
    }
    free(cache);
    fclose(fp);
    printSummary(hitCount, missCount, evictionCount);
    return 0;
}
