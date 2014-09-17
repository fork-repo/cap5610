CFLAGS	= -Wall -W -O2 -g 
LIBS		= -lpthread -lm
ALL 		=  knn validation cross-validation problem2
PWD			=  `pwd`

all: $(ALL)

%.o: %.c	
	$(CC) $(CFLAGS) -c -o $*.o $<

problem2: problem2.o mnist.o
	cc $(CFLAGS) -o $@ $^ $(LIBS)

knn: knn.o
	cc $(CFLAGS) -o $@ $^ $(LIBS)

validation: validation.o
	cc $(CFLAGS) -o $@ $^ $(LIBS)

cross-validation: cross-validation.o
	cc $(CFLAGS) -o $@ $^ $(LIBS)

test: knn validation cross-validation prepare
	$(PWD)/test.sh

clean : 
	rm -f *.o
	rm -f $(ALL)

%.gz:
	wget http://yann.lecun.com/exdb/mnist/$*.gz

%.idx: %.gz
	gzip -d $*.gz
	mv $* $*.idx

prepare: t10k-images-idx3-ubyte.idx t10k-labels-idx1-ubyte.idx train-images-idx3-ubyte.idx train-labels-idx1-ubyte.idx
