#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

typedef struct magic_number_s {
  char a;
  char b;
  char type;
  char dimentions;
} magic_number_t;

typedef struct labels_s{
  magic_number_t magic_number;
  unsigned int number_of_items;
  unsigned char label[1];
} labels_t;

typedef struct images_s{
  magic_number_t magic_number;
  unsigned int number_of_images;
  unsigned int number_of_rows;
  unsigned int number_of_columns;
  unsigned char pixel[1];
} images_t;

void *loadfile(char *file, long *size) {
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

int main(int argc, char ** argv) {
  long train_images_size;
  long train_labels_size;
  long t10k_images_size;
  long t10k_labels_size;
  images_t* train_images = ( images_t * ) loadfile( "train-images-idx3-ubyte.idx", &train_images_size);
  labels_t* train_labels = ( labels_t * ) loadfile( "train-labels-idx1-ubyte.idx", &train_labels_size);
  images_t* t10k_images = ( images_t * ) loadfile( "t10k-images-idx3-ubyte.idx", &t10k_images_size);
  labels_t* t10k_labels = ( labels_t * ) loadfile( "t10k-labels-idx1-ubyte.idx", &t10k_labels_size);

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

  return 0;
}
