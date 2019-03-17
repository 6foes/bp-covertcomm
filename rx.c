#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "shared_fns.h"
#include "constants.h"
#include "cpu.h"

#define SS 600

struct timespec tstart, tend;

void logOutputstr(char* outputStr){
  if (outputStr != NULL) { // clean up
    fprintf(stdout, "message is: %s\n", outputStr);
  }
}

int stateAction0(){
  fprintf(stdout, "verifying...\n");
  return 1;
}

int stateAction1Success(){
  fprintf(stdout, "receiving...\n");
  return 2;
}

int stateAction1Failure(){
  fprintf(stdout, "failed! listening...\n");
  return 0;
}

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
  double prev_end_time = 1.0e-9 * tend.tv_nsec + 0.10;
  double prev_end_time_base = (int) prev_end_time;
  double prev_fraction = prev_end_time - prev_end_time_base;
  int readTimePrev = (int) (prev_fraction * 4.0); 

  int thresh = (int) (tend.tv_nsec - tstart.tv_nsec);
  int thrSamp[4] = {0};
  int samp[SS] = {0};
  int sampPtr = 0;

  char length = -1;
  char current = 0;
  char *outputStr = NULL;
  char currentChar = 0;

  fprintf(stdout, "waiting...\n");
  while (1) {
    int i = 0, td = 0, readTime = 0;
    long sum = 0, secTime = 0;
    unsigned int avg = 0;

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
        sampPtr = 0;
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
          state = stateAction0();          
        } else {
          sum = 0;
          int jl = 0;
          while(jl < 4)
          {
            sum += thrSamp[jl];
            jl++;
          }
          // for (i = 0; i < 4; i++) {
          //  sum += thrSamp[i];
          //}
          thresh = sum / 4;
        }
        break;

        case 1:
        if (currentChar == (char)0b10101010) {
        } else if (currentChar == (char)0b00000000) {
          state = stateAction1Success();          
        } else {
          state = stateAction1Failure();          
        }
        break;

      case 2:
        if (length < 0) { // length is the first bit to be sent
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
      sum = 0;
      int kl = 0;
      while (kl < 4)
      {
        sum += thrSamp[kl];
        kl++;
      }
        //for (i = 0; i < 4; i++) {
      //    sum += thrSamp[i];
      //  }
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

    usleep(RX_SLEEP);

  }

exit:
  logOutputstr(outputStr);  
  free(outputStr);
  return 0;
} 
