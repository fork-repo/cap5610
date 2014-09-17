#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include "mnist.h"

unsigned int digits[10];
float mle[10], map[5][10];

/* Maximum likelihood estimation
 * Maximum A Posterior estimation
 */
static void estimation(void){
  unsigned int i,d;

  for(i = 0; i < 10; i++){
    digits[i] = 0;
  }

  // # of digit d in training set
  for(i = 0; i < mnist_train; i++){
    digits[*(mnist_labels+i)]++;
  }

  printf("digit\tcount\tmle\t");
  for(d = 0; d < 5; d++) {
    printf("map(%.0f%%)\t", pow(2,d)); //map
  }
  printf("\n");

  for(i = 0; i < 10; i++){
    printf("%d\t%d\t", i,digits[i]);
    mle[i] = (float)digits[i]/mnist_train; //mle
    printf("%.2f%%\t", 100*mle[i]);
    for(d = 0; d < 5; d++) {
      map[d][i] = ((float)digits[i] + (pow(2,d)/100)*mnist_train)/(mnist_train + (pow(2,d)/100)*mnist_train*10); //map
      printf("%.2f%%\t", 100*map[d][i]);
    }
    printf("\n");
  }
}

int main(int argc, char ** argv) {
  load_idx();
  estimation();
  return 0;
}
