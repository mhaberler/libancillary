CC=gcc
CFLAGS=-Wall -g -O2
LDFLAGS=
LIBS=
AR=ar cr
RANLIB=ranlib

OBJECTS=fd_pass.o

.c.o:
	$(CC) -c $(CFLAGS) $<

all: libancillary.a

libancillary.a: $(OBJECTS)
	$(AR) $@ $(OBJECTS)
	$(RANLIB) $@

fd_pass.o:

test: test.c libancillary.a
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) -L. $< -lancillary $(LIBS)

clean:
	-$(RM) *.o *.a test 
