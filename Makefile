CFLAGS	= -Wall -W -O2 -g 
LIBS		= 
ALL 		=  mnist
PWD			=  `pwd`

all: $(ALL)

%.o: %.c	
	$(CC) $(CFLAGS) -c -o $*.o $<

mnist: mnist.o
	cc $(CFLAGS) -o $@ $^ $(LIBS)

test: mnist prepare
	$(PWD)/mnist

clean : 
	rm -f *.o
	rm -f $(ALL)

%.gz:
	wget http://yann.lecun.com/exdb/mnist/$*.gz

%.idx: %.gz
	gzip -d $*.gz
	mv $* $*.idx

prepare: t10k-images-idx3-ubyte.idx t10k-labels-idx1-ubyte.idx train-images-idx3-ubyte.idx train-labels-idx1-ubyte.idx
