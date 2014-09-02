CFLAGS	= -Wall -W -O2 -g 
LIBS		= -lpthread
ALL 		=  mnist validation cross-validation
PWD			=  `pwd`

all: $(ALL)

%.o: %.c	
	$(CC) $(CFLAGS) -c -o $*.o $<

mnist: mnist.o
	cc $(CFLAGS) -o $@ $^ $(LIBS)

validation: validation.o
	cc $(CFLAGS) -o $@ $^ $(LIBS)

cross-validation: cross-validation.o
	cc $(CFLAGS) -o $@ $^ $(LIBS)

test: mnist validation cross-validation prepare
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
