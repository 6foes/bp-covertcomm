#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "shared_fns.h"
#include "cpu.h"
#include "constants.h"

#define SS 600

struct timespec tstart, tend;

void* signalingThread(void *vargp) {
  setProcessor(SIGNALING_PROCESSOR);

  while (one) {
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    alwaysNotTaken();
    clock_gettime(CLOCK_MONOTONIC, &tend);
    int probe_thresh = (int) (tend.tv_nsec - tstart.tv_nsec);
    fprintf(stdout, "Threshold probing time difference in seconds %i.\n", probe_thresh);
    usleep(20);
  }
}

int stateActionOnZero (){  
  fprintf(stdout, "verifying...\n");
  return one;
}

int stateActionOnOneSuccess (){    
  fprintf(stdout, "receiving...\n");
  return two;
}

int stateActionOnOneFailure (){    
  fprintf(stdout, "failed! listening...\n");
  return zero;
}

void logOutputString(outputStr){
  if (outputStr != NULL) { //clear 
    fprintf(stdout, "message is: %s\n", outputStr);
  }
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

  int state = zero; // program states      

  long secTimePrev = tend.tv_sec/2; // new character
  double prev_end_time = 1.0e-9 * tend.tv_nsec + 0.10;
  double prev_end_time_base = (int) prev_end_time;
  double prev_fraction = prev_end_time - prev_end_time_base;
  int readTimePrev = (int) (prev_fraction * 4.0); 

  int thresh = (int) (tend.tv_nsec - tstart.tv_nsec);
  int thrSamp[four] = {zero};
  int samp[SS] = {zero};
  int sampPtr = zero;

  char length = nVe;
  char current = zero;
  char *outputStr = NULL;
  char currentChar = zero;

  fprintf(stdout, "waiting...\n");
  while (1) {
    int i = 0, td = 0, readTime = 0;
    long sum = zero, secTime = zero;
    unsigned int avg = zero;

    secTime = tend.tv_sec/2;
    double end_time = 1.0e-9 * tend.tv_nsec + 0.10;
    double end_time_base = (int) end_time;
    double fraction = end_time - end_time_base;
    readTime = (int) (fraction * 4.0); // new bit

    td = (int)(tend.tv_nsec - tstart.tv_nsec);
    
    if (td < 1000 && td > 0)  { // store latest measure to samp
      samp[sampPtr] = td;
      sampPtr++;
      if (sampPtr >= SS) {
        sampPtr = zero;
      }
    }
    int il = 0;
    while(il < SS)
    {
       sum += samp[il];
       il++;
    }
    //for (i = 0; i < SS; i++) { // get current sample
    //  sum += samp[i];
    //}
    avg = sum / SS;

    if (secTime != secTimePrev) { // character processor
      
      switch(state) {
        case 0:
        if (currentChar == (char)0b10101010) {
          state = stateActionOnOne();
        } else {
          sum = zero;
          int jl = zero;
          while(jl < four)
          {
            sum += thrSamp[jl];
            jl++;
          }          
          thresh = sum / 4;
        }
        break;

        case 1:
        if (currentChar == (char)0b10101010) {
        } else if (currentChar == (char)0b00000000) {
          state = stateActionOnOneSuccess();
        } else {
          state = stateActionOnOneFailure();
        }
        break;

      case 2:
        if (length < zero) { // length is the first bit to be sent
          length = currentChar;
          outputStr = malloc(sizeof(char) * (length + 1));
          fprintf(stdout, "message size is %i.\n", length);
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
      sum = zero;
      int kl = 0;
      while (kl < four)
      {
        sum += thrSamp[kl];
        kl++;
      }       
      thresh = sum / four;
      break;

      default:
      break;
      }          
      currentChar = zero;
      secTimePrev = secTime;
    }

    if (readTime != readTimePrev) { // bit processor
      if (state == zero || state == -1) {
        thrSamp[readTime] = avg;
      }
      char alpha = zero;
      if (avg < thresh) {
        alpha = 1;
      }
      currentChar = (currentChar << 1) | alpha;
      readTimePrev = readTime;
    }

    usleep(sleeptime);

  }

exit:
  logOutputString(outputStr);
  free(outputStr);
  return zero;
} 
