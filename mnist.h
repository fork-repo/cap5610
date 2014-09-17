#ifndef _MNIST_H_
#define _MNIST_H_

#define get_img(head, index) (head + 28*28*index)

extern unsigned char *mnist_images;
extern unsigned char *mnist_labels;
extern unsigned int  *mnist_count;

void load_idx(void);

#endif
