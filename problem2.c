#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "mnist.h"

/* Maximum likelihood estimation
 * Maximum A Posterior estimation
 */

unsigned int digits[10];

static void estimation(void){
  unsigned int i;
  float d;

  for(i = 0; i < 10; i++){
    digits[i] = 0;
  }

  // # of digit d in training set
  for(i = 0; i < mnist_train; i++){
    digits[*(mnist_labels+i)]++;
  }

  printf("digit\tcount\tmle\t");
  for(d = 0.01; d <= 0.16; d=d*2) {
    printf("map(%.0f%%)\t", 100*d); //map
  }
  printf("\n");

  for(i = 0; i < 10; i++){
    printf("%d\t%d\t", i,digits[i]); //digit count
    printf("%.2f%%\t", ((float)digits[i] * 100)/mnist_train); //mle
    for(d = 0.01; d <= 0.16; d=d*2) {
      printf("%.2f%%\t", 100*((float)digits[i] + d*mnist_train)/(mnist_train + d*mnist_train*10)); //map
    }
    printf("\n");
  }
}

int main(int argc, char ** argv) {
  load_idx();
  estimation();
  return 0;
}
