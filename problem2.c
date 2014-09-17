#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "mnist.h"

unsigned char *images;
unsigned char *labels;

int main(int argc, char ** argv) {
  load_idx();
  return 0;
}
