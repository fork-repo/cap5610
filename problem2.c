#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "mnist.h"

/* Maximum likelihood estimation
 */

static void mle(void){
  unsigned int digits[10], i;

  for(i = 0; i < 10; i++){
    digits[i] = 0;
  }

  for(i = 0; i < mnist_train; i++){
    digits[*(mnist_labels+i)]++;
  }

  printf("Maximum likelihood estimation:\n");

  for(i = 0; i < 10; i++){
    printf("%d\t", i);
  }
  printf("\n");

  for(i = 0; i < 10; i++){
    printf("%d\t", digits[i]);
  }
  printf("\n");

  for(i = 0; i < 10; i++){
    printf("%.2f\%\t", ((float)digits[i] * 100)/mnist_train);
  }
  printf("\n");
}

int main(int argc, char ** argv) {
  load_idx();
  mle();
  return 0;
}
