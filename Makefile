CC=gcc
CFLAGS=-Wall -g -O2
LDFLAGS=
LIBS=
AR=ar crs

.c.o:
	$(CC) -c $(CFLAGS) $<

libancillary.a: fd_pass.o
	$(AR) $@ $^

fd_pass.o:

test: test.c libancillary.a
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) -L. $< -lancillary
