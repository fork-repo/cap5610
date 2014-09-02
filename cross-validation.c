#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

typedef struct magic_number_s {
  char a;
  char b;
  char type;
  char dimentions;
} magic_number_t;

typedef struct labels_s{
  magic_number_t magic_number;
  unsigned int number_of_items;
  unsigned char labels[1]; //[number of MNIST images]
} labels_t;

typedef struct images_s{
  magic_number_t magic_number;
  unsigned int number_of_images;
  unsigned int number_of_rows;
  unsigned int number_of_columns;
  unsigned char images[1]; //28x28x[number of MNIST images]
} images_t;

#define get_img(head, index) (head + 28*28*index)

// dataset
unsigned char *images;
unsigned char *labels;

// create a buffer and load the whole file
void *load_file(char *file, long *size) {
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen ( file , "rb" );
  if( !fp ) perror(file),exit(1);

  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );

  /* allocate memory for entire content */
  buffer = calloc( 1, lSize+1 );
  if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

  /* copy the file into the buffer */
  if( 1!=fread( buffer , lSize, 1 , fp) )
    fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

  /* do your work here, buffer is a string contains the whole text */
  *size = lSize;
  fclose(fp);
  return buffer;
}

// The sizes in each dimension are 4-byte integers (MSB first, high endian, like in most non-Intel processors).
int is_bigendian() {
  int i = 1;
  char *p = (char *)&i;

  if (p[0] == 1)
    return 0;
  else
    return 1;
}

unsigned int bit32conversion(unsigned int num) {
  return ((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) | ((num<<24)&0xff000000);
}

unsigned short bit16conversion(unsigned short num) {
  return (num>>8) | (num<<8);
}

// load the idx file and print the basic informations
void load_idx(void){
  images_t* train_images;
  labels_t* train_labels;
  images_t* t10k_images;
  labels_t* t10k_labels;

  long train_images_size;
  long train_labels_size;
  long t10k_images_size;
  long t10k_labels_size;
  train_images  = ( images_t * ) load_file( "train-images-idx3-ubyte.idx", &train_images_size);
  train_labels  = ( labels_t * ) load_file( "train-labels-idx1-ubyte.idx", &train_labels_size);
  t10k_images   = ( images_t * ) load_file( "t10k-images-idx3-ubyte.idx", &t10k_images_size);
  t10k_labels   = ( labels_t * ) load_file( "t10k-labels-idx1-ubyte.idx", &t10k_labels_size);

  if(!is_bigendian()) {
    train_images->number_of_images  = bit32conversion(train_images->number_of_images);
    train_images->number_of_rows    = bit32conversion(train_images->number_of_rows);
    train_images->number_of_columns = bit32conversion(train_images->number_of_columns);
    train_labels->number_of_items   = bit32conversion(train_labels->number_of_items);
    t10k_images->number_of_images   = bit32conversion(t10k_images->number_of_images);
    t10k_images->number_of_rows     = bit32conversion(t10k_images->number_of_rows);
    t10k_images->number_of_columns  = bit32conversion(t10k_images->number_of_columns);
    t10k_labels->number_of_items    = bit32conversion(t10k_labels->number_of_items);
  }

  printf("train images: %ld\ntype: 0x%.2x\ndimensions: %u\nnumber of images: %u\nnumber of rows: %u\nnumber of columns: %u\n\n", train_images_size, train_images->magic_number.type, train_images->magic_number.dimentions, train_images->number_of_images, train_images->number_of_rows, train_images->number_of_columns);
  printf("train labels: %ld\ntype: 0x%.2x\ndimentions: %u\nnumber of items: %u\n\n", train_labels_size, train_labels->magic_number.type, train_labels->magic_number.dimentions, train_labels->number_of_items);
  printf("t10k images: %ld\ntype: 0x%.2x\ndimentions: %u\nnumber of images: %u\nnumber of rows: %u\nnumber of columns: %u\n\n", t10k_images_size, t10k_images->magic_number.type, t10k_images->magic_number.dimentions, t10k_images->number_of_images, t10k_images->number_of_rows, t10k_images->number_of_columns);
  printf("t10k labels: %ld\ntype: 0x%.2x\ndimentions: %u\nnumber of items: %u\n\n", t10k_labels_size, t10k_labels->magic_number.type, t10k_labels->magic_number.dimentions, t10k_labels->number_of_items);

  images = calloc(1, 28*28*(train_images->number_of_images + t10k_images->number_of_images));
  labels = calloc(1, (train_images->number_of_images + t10k_images->number_of_images));
  memcpy(images, train_images->images, 28*28*train_images->number_of_images);
  memcpy(images + 28*28*train_images->number_of_images, t10k_images->images, 28*28*t10k_images->number_of_images);
  memcpy(labels, train_labels->labels, train_images->number_of_images);
  memcpy(labels + train_images->number_of_images, t10k_labels->labels, t10k_images->number_of_images);

  free(train_images);
  free(train_labels);
  free(t10k_images);
  free(t10k_labels);
}

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
  unsigned int hit;
} cfg_t;

// tell the majority results fom knn.
int get_prediction(cfg_t * cfg){
  unsigned char key, i;
  unsigned int counter[10];

  for(i = 0; i < 10; i++) {
    counter[i] = 0;
  }

  for(i = 0; i < cfg->k; i++){
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
    for(j = cfg->train_start; j < cfg->test_start; j++) { //find knn
      insert_neighbor(cfg, j, distance( get_img(images, j), get_img(images, i)));
    }
    for(j = cfg->test_stop; j < cfg->train_stop; j++) { //find knn
      insert_neighbor(cfg, j, distance( get_img(images, j), get_img(images, i)));
    }

    prediction  = get_prediction(cfg);
    label       = *(labels + i);
    //printf("%u\t%u\t%u\n",i , prediction, label);
    if(prediction == label) cfg->hit++;
    cfg->total++;
  }

  return NULL;
}

void start_predict(unsigned int thread, \
    unsigned int fold, \
    unsigned int dataset) {
  pthread_t tid[thread];
  cfg_t cfg[thread];

  unsigned int test, hit, total, i, k = 5;
  for(test = 0; test < fold; test++) {
    hit = total = 0;
    for(i = 0; i < thread; i++) {
      cfg[i].k=k;
      cfg[i].hit = 0;
      cfg[i].total = 0;
      cfg[i].knn = ( knn_t * ) calloc( sizeof(knn_t), k );
      cfg[i].train_start  = 0;
      cfg[i].train_stop   = fold*dataset;
      cfg[i].test_start   = test*dataset;
      cfg[i].test_stop    = cfg[i].test_start + dataset;
      cfg[i].thread       = thread;
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
      hit += cfg[i].hit;
      total += cfg[i].total;
    }
    printf("fold %u, hit %u, total %u", fold, hit, total);
  }
}

int main(int argc, char ** argv) {
  int c;
  int thread = 1;
  int fold = 5;
  int dataset = 10000;

  while ((c = getopt (argc, argv, "t:f:d:")) != -1) {
    switch (c) {
      case 't':
        thread = atoi(optarg);
        break;
      case 'f':
        fold = atoi(optarg);
        break;
      case 'd':
        dataset = atoi(optarg);
        break;
      default:
        fprintf(stderr, "Usage: %s <-t> thread <-f> fold <-d> dataset per fold\n", argv[0]);
        return 1;
    }
  }

  load_idx();
  printf("thread = %d\n", thread);
  start_predict(thread, fold, dataset);
  return 0;
}
