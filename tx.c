#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "shared_fns.h"
#include "cpu.h"

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
    fprintf(stderr, "invalid args\n");
    return 2;
  }

  char* message = argv[1];
  int msgLen = strlen(message);

  if (msgLen <= 0) {
    fprintf(stderr, "invalid message\n");
    return 2;
  }

  fprintf(stdout, "message of size %i reads: \"%s\"\n", msgLen, message);

  int state = 0;

  char currentChar = message[0];
  int charIndex = 0; //0 to msgLen
  int bitIndex = 0; // 0 to 7

  struct timespec tcurr;
  clock_gettime(CLOCK_MONOTONIC, &tcurr);

  long secTimePrev = tcurr.tv_sec/2;
  double prev_curr_time = 1.0e-9 * tcurr.tv_nsec + 0.75;
  double prev_curr_time_base = (int) prev_curr_time;
  double prev_curr_fraction = prev_curr_time - prev_curr_time_base;

  int readTimePrev = (int) (prev_curr_fraction * 4.0); 

  pthread_t tid;
  pthread_create(&tid, NULL, signalingThread, NULL);

  while (1) {
    clock_gettime(CLOCK_MONOTONIC, &tcurr);
    long secTime = tcurr.tv_sec/2;

    double curr_time = 1.0e-9 * tcurr.tv_nsec + 0.75;
    double curr_time_base = (int) curr_time;
    double fraction = curr_time - curr_time_base;
    int readTime = (int) (fraction * 4.0);    

    static int bitNumber = 0;
    if (secTime != secTimePrev) { // character processor
      switch(state){
        case 0:
        sigBit = 0;
        bitIndex = 0;
        state = 1;
        printf("start sending...\n");
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
        printf("transmitting size...\n");
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
  fprintf(stdout, "Sent!\n");

  return 0;
} 