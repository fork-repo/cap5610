#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

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
unsigned int train_start, train_stop, test_start, test_stop;

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

  train_start = 0;
  train_stop  = train_images->number_of_images;
  test_start  = train_images->number_of_images;
  test_stop   = train_images->number_of_images + t10k_images->number_of_images;

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
knn_t * knn;
unsigned int k;
double max_distance_in_knn = 0;

// tell the majority results fom knn.
int get_prediction(){
  unsigned char key, i;
  unsigned int counter[10];

  for(i = 0; i < 10; i++) {
    counter[i] = 0;
  }

  for(i = 0; i < k; i++){
    unsigned char label = labels[(knn+i)->index];
    counter[label]++;
  }

  for(key = 0, i = 1; i < 10; i++) {
    if(counter[key] < counter[i]) {
      key = i;
    }
  }
  return key;
}

// caculate the distance
double euclidean_distance(unsigned char img1[], unsigned char img2[]) {
  int i,j;
  double sum = 0, value;
  for(i = 0, j= 28*28; i < j; i++) {
    value = img1[i] - img2[i];
    sum = sum + value*value;
  }
  return sum;
}

void predict(void){
  unsigned int i,j,l;
  unsigned char prediction, label;
  unsigned int hit = 0;
  for(i = test_start; i < test_stop; i++) { //for all test set
    for(j = train_start; j < train_start + k; j++) { //the first k from train set
      (knn+j)->index = j;
      (knn+j)->distance = euclidean_distance( get_img(images, j), get_img(images, i) );
    }

    for(; j < train_stop; j++) { //the k nearest
      double dist = euclidean_distance( get_img(images, j), get_img(images, i) );
      for(l = 0; l < k; l++) {
        if(dist < (knn+l)->distance) {
          (knn+l)->index = j;
          (knn+l)->distance = dist;
          break;
        }
      }
    }

    prediction  = get_prediction();
    label       = *(labels + i);
    printf("%u\t%u\t%u\n",i , prediction, label);
    if(prediction == label) hit++;
  }
  printf("hit %u, total %u", hit, train_stop - train_start);
}

int main(int argc, char ** argv) {
  load_idx();
  k=10;
  knn = ( knn_t * ) calloc( sizeof(knn_t), k );
  predict();
  return 0;
}
