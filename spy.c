// Nolan Shah
// Covert Channels Communication Assignment
// COSC 3330, Computer Architecture, University of Houston

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

#define SS 600

struct timespec tstart, tend;

void* signalingThread(void *vargp) {
  setProcessor(SIGNALING_PROCESSOR);

  while (1) {
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    alwaysNotTaken();
    clock_gettime(CLOCK_MONOTONIC, &tend);
    int probe_thresh = (int) (tend.tv_nsec - tstart.tv_nsec);
    fprintf(stdout, "Threshold probing time difference in seconds %i.\n", probe_thresh);

    usleep(20);
  }
}

int genBit(double num) {
  double a = (int) (1.0e-9 * num + 0.10);
  double newB = num - a
  int newbit = (int) (newB * 4.0)
  return newbit; 
}

int main( int argc, char** argv ) {
  setProcessor(CONTROL_PROCESSOR);

  clock_gettime(CLOCK_MONOTONIC, &tstart); 
  alwaysNotTaken();  
  clock_gettime(CLOCK_MONOTONIC, &tend);

  int bench_thresh = (int) (tend.tv_nsec - tstart.tv_nsec);
  fprintf(stdout, "Benchmark Threshold:  %i.\n", bench_thresh);

  pthread_t tid; // second thread
  pthread_create(&tid, NULL, signalingThread, NULL);

  int state = 0; // program states      

  long secTimePrev = tend.tv_sec/2; // new character
  int readTimePrev = (int) (getFraction(1.0e-9 * tend.tv_nsec + 0.10) * 4.0); // new bit

  int thresh = (int) (tend.tv_nsec - tstart.tv_nsec);
  int thrSamp[4] = {0};
  int samp[SS] = {0};
  int sampPtr = 0;

  char length = -1;
  char current = 0;
  char *outputStr = NULL;
  char currentChar = 0;

  fprintf(stdout, "listening for transmission...\n");
  while (1) {
    int i = 0, td = 0, readTime = 0;
    long sum = 0, secTime = 0;
    unsigned int avg = 0;

    secTime = tend.tv_sec/2;
    readTime = (int) (getFraction(1.0e-9 * tend.tv_nsec + 0.10) * 4.0); // new bit

    td = (int)(tend.tv_nsec - tstart.tv_nsec);
    
    if (td < 1000 && td > 0)  { // store latest measure to samp
      samp[sampPtr] = td;
      sampPtr++;
      if (sampPtr >= SS) {
        sampPtr = 0;
      }
    }
    for (i = 0; i < SS; i++) { // get current sample
      sum += samp[i];
    }
    avg = sum / SS;

    if (secTime != secTimePrev) { // character processor
      
      switch(state) {
        case 0:
        if (currentChar == (char)0b10101010) {
          state = 1;
          fprintf(stdout, "potential transmission. verifying...\n");
        } else {
          sum = 0;
          for (i = 0; i < 4; i++) {
            sum += thrSamp[i];
          }
          thresh = sum / 4;
        }
        break;

        case 1:
        if (currentChar == (char)0b10101010) {
        } else if (currentChar == (char)0b00000000) {
          state = 2;
          fprintf(stdout, "verification complete! receiving...\n");
        } else {
          state = 0;
          fprintf(stdout, "verification failed! resetting...\n");
        }
        break;

      case 2:
        if (length < 0) { // length is the first bit to be sent
          length = currentChar;
          outputStr = malloc(sizeof(char) * (length + 1));
          fprintf(stdout, "message length is %i.\n", length);
        } else {
          if (currentChar == (char)0) {
            outputStr[current] = '\0';
            goto exit;
          } else if (current == length - 1) {
            outputStr[current] = currentChar;
            outputStr[length] = '\0';
            goto exit;
          } else {
            outputStr[current] = currentChar;
            current++;
          }
        }
      break;

      case -1:
      sum = 0;
        for (i = 0; i < 4; i++) {
          sum += thrSamp[i];
        }
      thresh = sum / 4;
      break;

      default:
      break;
      }          
      currentChar = 0;
      secTimePrev = secTime;
    }

    if (readTime != readTimePrev) { // bit processor
      if (state == 0 || state == -1) {
        thrSamp[readTime] = avg;
      }
      char alpha = 0;
      if (avg < thresh) {
        alpha = 1;
      }
      currentChar = (currentChar << 1) | alpha;
      readTimePrev = readTime;
    }

    usleep(60);

  }

exit:

  if (outputStr != NULL) { // clean up
    fprintf(stdout, "message reads: %s\n", outputStr);
  }
  free(outputStr);
  return 0;
} 
