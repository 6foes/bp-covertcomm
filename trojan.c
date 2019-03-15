// Nolan Shah
// Covert Channels Communication Assignment
// COSC 3330, Computer Architecture, University of Houston

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

char sigBit = 0; // current bit sent, not thread safe, but doesn't need to be for this program

void* signalingThread(void *vargp) {
  setProcessor(SIGNALING_PROCESSOR);

  while (1) {
    if (sigBit == 0) {
      alwaysTaken();
    } else if (sigBit == 1) {
      alwaysNotTaken();
    } else {
      break;
    }
    usleep(1);
  }
}

int main( int argc, char** argv ) {
  setProcessor(CONTROL_PROCESSOR);

  if (argc < 2) {
    fprintf(stderr, "bad args\n");
    return 2;
  }

  char* message = argv[1];
  int msgLen = strlen(message);

  if (msgLen <= 0) {
    fprintf(stderr, "bad message\n");
    return 2;
  }

  fprintf(stdout, "message of length %i reads: \"%s\"\n", msgLen, message);

  int state = 0;

  char currentChar = message[0];
  int charIndex = 0; //0 to msgLen
  int bitIndex = 0; // 0 to 7

  struct timespec tcurr;
  clock_gettime(CLOCK_MONOTONIC, &tcurr);

  long secTimePrev = tcurr.tv_sec/2;
  int readTimePrev = (int) (getFraction(1.0e-9 * tcurr.tv_nsec + 0.75) * 4.0 );

  pthread_t tid;
  pthread_create(&tid, NULL, signalingThread, NULL);

  while (1) {
    clock_gettime(CLOCK_MONOTONIC, &tcurr);
    long secTime = tcurr.tv_sec/2;
    int readTime = (int) (getFraction(1.0e-9 * tcurr.tv_nsec + 0.75) * 4.0 ) ; // get current times

    static int bitNumber = 0;
    if (secTime != secTimePrev) { // character processor
      switch(state){
        case 0:
        sigBit = 0;
        bitIndex = 0;
        state = 1;
        printf("beginning transmission...\n");
        break;

        case 1:
        sigBit = 0;
        bitIndex = 0;
        if (bitNumber == 2) {
          state = 2;
          sigBit = 0;
        }
        break;

        case 2:
        state = 3;
        printf("sending length...\n");
        break;

        case 3:
        bitIndex = 0;
        charIndex = 0;
        state = 4;
        printf("sending char %i: %c\n", charIndex, message[charIndex]);
        break;

        case 4:
        charIndex++;
        bitIndex = 0;
        if (charIndex == msgLen) {
          sigBit = -1;
          goto exit;
        }
        //printf("sending char: %i, %c\t%f0.4\n", charIndex, message[charIndex], getFraction(1.0e-9 * tcurr.tv_nsec + 0.75));
        printf("sending char %i: %c\n", charIndex, message[charIndex]);
        break;

        case -1:
        sigBit = 1;
        break;

        default:
        break;
        
      }
                
      bitNumber += 1;
      secTimePrev = secTime;
    }

    if (readTime != readTimePrev) { // bit processor
      if (state == 1 | state == -1) {
        if (sigBit == 0) {
          sigBit = 1;
        } else {
          sigBit = 0;
        }
      } else if (state == 3) {
        if (((msgLen >> (7 - bitIndex)) & 0b1) == 1) {
          sigBit = 1;
        } else {
          sigBit = 0;
        }
        bitIndex++;
      } else if (state == 4) {
        if (((message[charIndex] >> (7 - bitIndex)) & 0b1) == 1) {
          sigBit = 1;
        } else {
          sigBit = 0;
        }
        bitIndex++;
      }
      //printf("switching to %d at %d\n", sigBit, readTime);
      readTimePrev = readTime;
    }

    usleep(100);
  }

exit:
  fprintf(stdout, "message sent!\n");

  return 0;
} 