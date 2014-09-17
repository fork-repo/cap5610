#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "mnist.h"

unsigned char *images;
unsigned char *labels;

// the k-nearest neighbors is here.
typedef struct knn_s{
  int index;
  double distance;
} knn_t;

typedef struct cfg_s{
  knn_t * knn;
  unsigned int number_of_neighbor;
  unsigned int k;
  unsigned int train_start, train_stop, test_start, test_stop;
  unsigned int thread;
  unsigned int total;
  unsigned int * hit;
} cfg_t;

// tell the majority results fom knn.
int get_prediction(cfg_t * cfg, unsigned int k){
  unsigned char key, i;
  unsigned int counter[10];

  for(i = 0; i < 10; i++) {
    counter[i] = 0;
  }

  for(i = cfg->k - k; i < cfg->k; i++){
    unsigned char label = labels[(cfg->knn+i)->index];
    counter[label]++;
  }

  for(key = 0, i = 1; i < 10; i++) {
    if(counter[key] < counter[i]) {
      key = i;
    }
  }
  return key;
}

// caculate the euclidean distance
double distance(unsigned char img1[], unsigned char img2[]) {
  int i,j;
  double sum = 0, value;
  for(i = 0, j= 28*28; i < j; i++) {
    value = img1[i] - img2[i];
    sum = sum + value*value;
  }
  return sum;
}

// clear the knn list
void clear_neighbor(cfg_t *cfg) {
  unsigned int i;
  for(i = 0; i < cfg->k; i++) {
    (cfg->knn+i)->index = 0;
    (cfg->knn+i)->distance = 0;
  }
  cfg->number_of_neighbor = 0;
}

// maintain the knn list
void insert_neighbor(cfg_t * cfg, unsigned int index, double distance) {
  unsigned int i;
  unsigned int index1, index2;
  double distance1, distance2;
  if(cfg->number_of_neighbor < cfg->k) {
    for(i = 0; i < cfg->number_of_neighbor; i++) {
      if((cfg->knn+i)->distance < distance) {
        break;
      }
    }

    index1 = index;
    distance1 = distance;
    cfg->number_of_neighbor++;
    for(; i < cfg->number_of_neighbor; i++) {
      index2 = (cfg->knn+i)->index;
      distance2 = (cfg->knn+i)->distance;
      (cfg->knn+i)->index = index1;
      (cfg->knn+i)->distance = distance1;
      index1 = index2;
      distance1 = distance2;
    }
  } else {
    for(i = 0; i < cfg->number_of_neighbor; i++) {
      if((cfg->knn+i)->distance < distance) {
        break;
      }
    }
    index1 = index;
    distance1 = distance;
    while(i != 0){
      i--;
      index2 = (cfg->knn+i)->index;
      distance2 = (cfg->knn+i)->distance;
      (cfg->knn+i)->index = index1;
      (cfg->knn+i)->distance = distance1;
      index1 = index2;
      distance1 = distance2;
    }
  }
}

void * predict(void * arg){
  cfg_t * cfg = arg;
  unsigned int i,j;
  unsigned char prediction, label;
  for(i = cfg->test_start; i < cfg->test_stop; i+=cfg->thread) { //for all test set
    clear_neighbor(cfg);
    for(j = cfg->train_start; j < cfg->train_stop; j++) { //find knn
      insert_neighbor(cfg, j, distance( get_img(images, j), get_img(images, i)));
    }

    label       = *(labels + i);
    for(j = 1; j <= cfg->k; j++){
      //printf("%u\t%u\t%u\n",j , prediction, label);
      prediction  = get_prediction(cfg, j);
      if(prediction == label) {
        *(cfg->hit + j)+=1;
      }
    }
    cfg->total++;
  }

  return NULL;
}

void start_predict(unsigned int k, \
    unsigned int thread, \
    unsigned int train_start, \
    unsigned int train_stop, \
    unsigned int test_start, \
    int test_stop) {
  pthread_t tid[thread];
  cfg_t cfg[thread];

  unsigned int i, j, hit[k], total = 0;

  for(i = 0; i < k; i++){
    hit[i] = 0;
  }

  for(i = 0; i < thread; i++) {
    cfg[i].k=k;
    cfg[i].hit = (unsigned int *) calloc( sizeof(unsigned int), k );
    cfg[i].total = 0;
    cfg[i].knn = ( knn_t * ) calloc( sizeof(knn_t), k );
    cfg[i].train_start  = train_start;
    cfg[i].train_stop   = train_stop;
    cfg[i].test_start   = test_start + i;
    cfg[i].test_stop    = test_stop;
    cfg[i].thread       = thread;

    for(j = 0; j < k; j++){
      *(cfg[i].hit + j) = 0;
    }

    if(pthread_create(tid+i, NULL, predict, cfg+i)) {
      fprintf(stderr, "Error creating thread\n");
      exit(1);
    }
  }

  for(i = 0; i < thread; i++) {
    if(pthread_join(tid[i], NULL)) {
      fprintf(stderr, "Error joining thread\n");
      exit(1);
    }

    for(j = 0; j < k; j++){
      hit[j] += *(cfg[i].hit + j);
    }
    free(cfg[i].hit);

    total += cfg[i].total;
  }

  for(j = 0; j < k; j++){
    printf("hit %u, ", hit[j]);
  }
  printf(", total %u\n", total);

}

int main(int argc, char ** argv) {
  int c;
  int thread = 1;
  int k = 1;

  while ((c = getopt (argc, argv, "t:k:")) != -1) {
    switch (c) {
      case 't':
        thread = atoi(optarg);
        break;
      case 'k':
        k = atoi(optarg);
        break;
      default:
        fprintf(stderr, "Usage: %s <-k K> <-t> thread\n", argv[0]);
        return 1;
    }
  }

  load_idx();
  images = mnist_images;
  labels = mnist_labels;
  printf("k = %d, thread = %d\n", k, thread);
  start_predict(k, thread, 0, 60000, 60000, 70000);
  return 0;
}
