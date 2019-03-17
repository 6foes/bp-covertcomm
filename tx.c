#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "shared_fns.h"
#include "constants.h"
#include "cpu.h"

char sigBit = zero; 

void* signalingThread(void *vargp) {
  setProcessor(SIGNALING_PROCESSOR);

  while (one) {
    if (sigBit == zero) {
      alwaysTaken();
    } else if (sigBit == one) {
      alwaysNotTaken();
    } else {
      break;
    }
    usleep(one);
  }
}

int main( int argc, char** argv ) {
  setProcessor(CONTROL_PROCESSOR);

  if (argc < two) {
    fprintf(stderr, "invalid args\n");
    return two;
  }

  char* message = argv[1];
  int msgLen = strlen(message);

  if (msgLen <= zero) {
    fprintf(stderr, "invalid message\n");
    return two;
  }

  fprintf(stdout, "message of size %i reads: \"%s\"\n", msgLen, message);

  int state = zero;

  char currentChar = message[0];
  int charIndex = zero; //0 to msgLen
  int bitIndex = zero; // 0 to 7

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

    static int bitNumber = zero;
    if (secTime != secTimePrev) { // character processor
      switch(state){
        case 0:
        sigBit = zero;
        bitIndex = zero;
        state = one;
        printf("start sending...\n");
        break;

        case 1:
        sigBit = zero;
        bitIndex = zero;
        if (bitNumber == two) {
          state = two;
          sigBit = zero;
        }
        break;

        case 2:
        state = three;
        printf("transmitting size...\n");
        break;

        case 3:
        bitIndex = zero;
        charIndex = zero;
        state = four;
        printf("sending char %i: %c\n", charIndex, message[charIndex]);
        break;

        case 4:
        charIndex++;
        bitIndex = zero;
        if (charIndex == msgLen) {
          sigBit = nVe;
          goto exit;
        }        
        printf("sending char %i: %c\n", charIndex, message[charIndex]);
        break;

        case -1:
        sigBit = one;
        break;

        default:
        break;
        
      }
                
      bitNumber += one;
      secTimePrev = secTime;
    }

    if (readTime != readTimePrev) { // bit processor
      if (state == one | state == nVe) {
        if (sigBit == 0) {
          sigBit = one;
        } else {
          sigBit = zero;
        }
      } else if (state == 3) {
        if (((msgLen >> (7 - bitIndex)) & 0b1) == 1) {
          sigBit = one;
        } else {
          sigBit = zero;
        }
        bitIndex++;
      } else if (state == 4) {
        if (((message[charIndex] >> (7 - bitIndex)) & 0b1) == 1) {
          sigBit = one;
        } else {
          sigBit = zero;
        }
        bitIndex++;
      }
      //printf("switching to %d at %d\n", sigBit, readTime);
      readTimePrev = readTime;
    }

    usleep(sleeptimetwo);
  }

exit:
  fprintf(stdout, "Sent!\n");

  return 0;
} 